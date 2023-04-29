#include "App.h"

#include <Babylon/Graphics/Device.h>
#include <Babylon/ScriptLoader.h>
#include <Babylon/Plugins/NativeEngine.h>
#include <Babylon/Plugins/NativeOptimizations.h>
#include <Babylon/Polyfills/Console.h>
#include <Babylon/Polyfills/Window.h>
#include <Babylon/Polyfills/XMLHttpRequest.h>
#include <Babylon/Polyfills/Canvas.h>

#include <winrt/windows.ui.core.h>
#include <winrt/windows.gaming.input.h>

namespace
{
    template <typename T>
    T from_cx(Platform::Object^ from)
    {
        T to{ nullptr };

        if (from != nullptr)
        {
            winrt::check_hresult(reinterpret_cast<::IUnknown*>(from)
                ->QueryInterface(winrt::guid_of<T>(), winrt::put_abi(to)));
        }

        return to;
    }
}

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::Gaming::Input;

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto direct3DApplicationSource = ref new Direct3DApplicationSource();
    CoreApplication::Run(direct3DApplicationSource);
    return 0;
}

// Helper function to handle mouse button routing
void ProcessMouseButtons(Babylon::Plugins::NativeInput* m_nativeInput, PointerUpdateKind updateKind, int x, int y)
{
    switch (updateKind)
    {
        case PointerUpdateKind::LeftButtonPressed:
            m_nativeInput->MouseDown(Babylon::Plugins::NativeInput::LEFT_MOUSE_BUTTON_ID, x, y);
            break;
        case PointerUpdateKind::MiddleButtonPressed:
            m_nativeInput->MouseDown(Babylon::Plugins::NativeInput::MIDDLE_MOUSE_BUTTON_ID, x, y);
            break;
        case PointerUpdateKind::RightButtonPressed:
            m_nativeInput->MouseDown(Babylon::Plugins::NativeInput::RIGHT_MOUSE_BUTTON_ID, x, y);
            break;
        case PointerUpdateKind::LeftButtonReleased:
            m_nativeInput->MouseUp(Babylon::Plugins::NativeInput::LEFT_MOUSE_BUTTON_ID, x, y);
            break;
        case PointerUpdateKind::MiddleButtonReleased:
            m_nativeInput->MouseUp(Babylon::Plugins::NativeInput::MIDDLE_MOUSE_BUTTON_ID, x, y);
            break;
        case PointerUpdateKind::RightButtonReleased:
            m_nativeInput->MouseUp(Babylon::Plugins::NativeInput::RIGHT_MOUSE_BUTTON_ID, x, y);
            break;
    }
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new App();
}

App::App() :
    m_windowClosed{ false },
    m_windowVisible{ true }
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
    // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

    CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

    // Register these before querying initial list to avoid 'race condition' with enumeration.
    Gamepad::GamepadAdded += ref new EventHandler<Gamepad^ >( [ = ]( Platform::Object^, Gamepad^ )
    {
        m_currentGamepadNeedsRefresh = true;
    } );

    Gamepad::GamepadRemoved += ref new EventHandler<Gamepad^ >( [ = ]( Platform::Object^, Gamepad^ )
    {
        m_currentGamepadNeedsRefresh = true;
    } );

    m_currentGamepadNeedsRefresh = false;
    m_localCollection = ref new Platform::Collections::Vector<Windows::Gaming::Input::Gamepad^> ();

    RefreshCachedGamepads();
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
    window->SizeChanged +=
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

    window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

    window->Closed +=
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

    window->PointerMoved +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerMoved);

    window->PointerPressed +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerPressed);

    window->PointerReleased +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerReleased);

    window->PointerWheelChanged +=
        ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &App::OnPointerWheelChanged);

    window->KeyDown +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyPressed);  
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
}

inline float FilterAnalogInput( int val, int deadZone )
{
    if(val < 0)
    {
        if(val > -deadZone)
            return 0.0f;
        else
            return ( val + deadZone ) / ( 32768.0f - deadZone );
    }
    else
    {
        if(val < deadZone)
            return 0.0f;
        else
            return ( val - deadZone ) / ( 32767.0f - deadZone );
    }
}

// This method is called after the window becomes active.
void App::Run()
{
    while (!m_windowClosed)
    {
        if(m_currentGamepadNeedsRefresh)
        {
            m_currentGamepadNeedsRefresh = false;
            RefreshCachedGamepads();
            auto mostRecentGamepad = GetLastGamepad();
            if(m_currentGamepad != mostRecentGamepad)
            {
                m_currentGamepad = mostRecentGamepad;
            }
        }

        if(m_currentGamepad != nullptr)
        {
            m_reading = m_currentGamepad->GetCurrentReading();

            uint32_t buttons = static_cast< uint32_t >( m_reading.Buttons );
            double rightTrigger = m_reading.RightTrigger;
            double leftTrigger = m_reading.LeftTrigger;
            double LeftThumbstickX = m_reading.LeftThumbstickX;
            double LeftThumbstickY = m_reading.LeftThumbstickY;
            double RightThumbstickX = m_reading.RightThumbstickX;
            double RightThumbstickY = m_reading.RightThumbstickY;

            // choose a deadzone -- readings inside this radius are ignored.
            const double deadzoneRadius = 0.1;
            const double deadzoneSquared = deadzoneRadius * deadzoneRadius;

            // Pythagorean theorem -- for a right triangle, hypotenuse^2 = (opposite side)^2 + (adjacent side)^2
            auto oppositeSquared = LeftThumbstickX * LeftThumbstickY;
            auto adjacentSquaredX = LeftThumbstickX * LeftThumbstickX;
            auto adjacentSquaredY = LeftThumbstickY * LeftThumbstickY;

            if(( oppositeSquared + adjacentSquaredX + adjacentSquaredY ) <= deadzoneSquared)
            {
                LeftThumbstickX = 0.0;
                LeftThumbstickY = 0.0;
            }

            m_runtime->Dispatch( [ buttons, rightTrigger, leftTrigger, LeftThumbstickX, LeftThumbstickY, RightThumbstickX, RightThumbstickY ]( Napi::Env env )
            {
                auto global = env.Global();
                auto setGamePadFunction = global.Get( "SetGamepad" ).As<Napi::Function>();
                setGamePadFunction.Call( {
                        Napi::Value::From( env , buttons ),
                        Napi::Value::From( env , rightTrigger ),
                        Napi::Value::From( env , leftTrigger ),
                        Napi::Value::From( env , LeftThumbstickX ),
                        Napi::Value::From( env , LeftThumbstickY ),
                        Napi::Value::From( env , RightThumbstickX ),
                        Napi::Value::From( env , RightThumbstickY ),
                    } );
            } );
        }


        if (m_device)
        {
            m_update->Finish();
            m_device->FinishRenderingCurrentFrame();
            m_device->StartRenderingCurrentFrame();
            m_update->Start();
        }

        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
    }
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
    if (m_device)
    {
        m_update->Finish();
        m_device->FinishRenderingCurrentFrame();
    }

    m_nativeInput = {};
    m_runtime.reset();
    m_device.reset();
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();

    if (args->Kind == Activation::ActivationKind::File)
    {
        m_files = static_cast<FileActivatedEventArgs^>(args)->Files;
    }
    else
    {
        m_files = nullptr;
    }

    RestartRuntime(applicationView->CoreWindow->Bounds);
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    // Save app state after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations. Be
    // aware that a deferral may not be held indefinitely. After about five seconds,
    // the app will be forced to exit.
    auto deferral = args->SuspendingOperation->GetDeferral();

    if (m_device)
    {
        m_update->Finish();
        m_device->FinishRenderingCurrentFrame();
    }

    m_runtime->Suspend();

    deferral->Complete();
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
    // Restore any data or state that was unloaded on suspend. By default, data
    // and state are persisted when resuming from suspend. Note that this event
    // does not occur if the app was previously terminated.
    m_runtime->Resume();

    if (m_device)
    {
        m_device->StartRenderingCurrentFrame();
        m_update->Start();
    }
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ /*sender*/, WindowSizeChangedEventArgs^ args)
{
    size_t width = static_cast<size_t>(args->Size.Width * m_displayScale);
    size_t height = static_cast<size_t>(args->Size.Height * m_displayScale);
    m_device->UpdateSize(width, height);
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    m_windowClosed = true;
    Uninitialize();
}

void App::OnPointerMoved(CoreWindow^, PointerEventArgs^ args)
{
    if (m_nativeInput != nullptr)
    {
        const auto& position = args->CurrentPoint->RawPosition;
        const auto deviceType = args->CurrentPoint->PointerDevice->PointerDeviceType;
        const auto deviceSlot = args->CurrentPoint->PointerId;
        const auto updateKind = args->CurrentPoint->Properties->PointerUpdateKind;
        const auto x = static_cast<int>(position.X);
        const auto y = static_cast<int>(position.Y);

        if (deviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
        {
            m_nativeInput->MouseMove(x, y);

            if (args->CurrentPoint->IsInContact)
            {
                ProcessMouseButtons(m_nativeInput, updateKind, x, y);
            }
        }
        else
        {
            m_nativeInput->TouchMove(deviceSlot, x, y);
        }
    }
}

void App::OnPointerPressed(CoreWindow^, PointerEventArgs^ args)
{
    if (m_nativeInput != nullptr)
    {
        const auto& position = args->CurrentPoint->RawPosition;
        const auto deviceType = args->CurrentPoint->PointerDevice->PointerDeviceType;
        const auto deviceSlot = args->CurrentPoint->PointerId;
        const auto updateKind = args->CurrentPoint->Properties->PointerUpdateKind;
        const auto x = static_cast<int>(position.X);
        const auto y = static_cast<int>(position.Y);

        if (deviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
        {
            ProcessMouseButtons(m_nativeInput, updateKind, x, y);
        }
        else
        {
            m_nativeInput->TouchDown(deviceSlot, x, y);
        }
    }
}

void App::OnPointerReleased(CoreWindow^, PointerEventArgs^ args)
{
    if (m_nativeInput != nullptr)
    {
        const auto& position = args->CurrentPoint->RawPosition;
        const auto deviceType = args->CurrentPoint->PointerDevice->PointerDeviceType;
        const auto deviceSlot = args->CurrentPoint->PointerId;
        const auto updateKind = args->CurrentPoint->Properties->PointerUpdateKind;
        const auto x = static_cast<int>(position.X);
        const auto y = static_cast<int>(position.Y);

        if (deviceType == Windows::Devices::Input::PointerDeviceType::Mouse)
        {
            ProcessMouseButtons(m_nativeInput, updateKind, x, y);
        }
        else
        {
            m_nativeInput->TouchUp(deviceSlot, x, y);
        }
    }
}
void App::OnPointerWheelChanged(CoreWindow^, PointerEventArgs^ args)
{
    const auto delta = args->CurrentPoint->Properties->MouseWheelDelta;
    m_nativeInput->MouseWheel(Babylon::Plugins::NativeInput::MOUSEWHEEL_Y_ID, -delta);
}

void App::OnKeyPressed(CoreWindow^ window, KeyEventArgs^ args)
{
    if (args->VirtualKey == VirtualKey::R)
    {
        RestartRuntime(window->Bounds);
    }
}

void App::RefreshCachedGamepads()
{
    m_localCollection->Clear();
    auto gamepads = Gamepad::Gamepads;
    for(auto gamepad : gamepads)
    {
        m_localCollection->Append( gamepad );
    }
}

Gamepad^ App::GetLastGamepad()
{
    Gamepad^ gamepad = nullptr;

    if(m_localCollection->Size > 0)
    {
        gamepad = m_localCollection->GetAt( m_localCollection->Size - 1 );
    }

    return gamepad;
}

void App::RestartRuntime(Windows::Foundation::Rect bounds)
{
    Uninitialize();

    DisplayInformation^ displayInformation = DisplayInformation::GetForCurrentView();
    m_displayScale = static_cast<float>(displayInformation->RawPixelsPerViewPixel);
    size_t width = static_cast<size_t>(bounds.Width * m_displayScale);
    size_t height = static_cast<size_t>(bounds.Height * m_displayScale);
    auto window = from_cx<winrt::Windows::Foundation::IInspectable>(CoreWindow::GetForCurrentThread());

    Babylon::Graphics::WindowConfiguration graphicsConfig{};
    graphicsConfig.Window = window;
    graphicsConfig.Width = width;
    graphicsConfig.Height = height;
    graphicsConfig.MSAASamples = 4;
    m_device = Babylon::Graphics::Device::Create(graphicsConfig);
    m_update = std::make_unique<Babylon::Graphics::DeviceUpdate>(m_device->GetUpdate("update"));
    m_device->StartRenderingCurrentFrame();
    m_update->Start();

    m_runtime = std::make_unique<Babylon::AppRuntime>();

    m_runtime->Dispatch([this](Napi::Env env) {
        m_device->AddToJavaScript(env);

        Babylon::Polyfills::Console::Initialize(env, [](const char* message, auto) {
            OutputDebugStringA(message);
        });

        Babylon::Polyfills::Window::Initialize(env);
        Babylon::Polyfills::XMLHttpRequest::Initialize(env);
        Babylon::Plugins::NativeEngine::Initialize(env);
        Babylon::Plugins::NativeOptimizations::Initialize(env);
        m_nativeCanvas = std::make_unique <Babylon::Polyfills::Canvas>(Babylon::Polyfills::Canvas::Initialize(env));
        m_nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(env);
    });

    Babylon::ScriptLoader loader{*m_runtime};
    loader.Eval("document = {}", "");
    // Commenting out recast.js for now as v8jsi is incompatible with asm.js.
    // loader.LoadScript("app:///Scripts/recast.js");
    loader.LoadScript("app:///Scripts/babylon.max.js");
    loader.LoadScript("app:///Scripts/babylonjs.loaders.js");
    loader.LoadScript("app:///Scripts/babylonjs.materials.js");
    loader.LoadScript("app:///Scripts/babylon.gui.js");
    loader.LoadScript("app:///Scripts/experience.js" );
}
