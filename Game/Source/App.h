#pragma once

#include <Babylon/Plugins/NativeInput.h>
#include <Babylon/AppRuntime.h>
#include <Babylon/Graphics/Device.h>
#include <Babylon/Polyfills/Canvas.h>
#include <collection.h>

// Main entry point for our app. Connects the app with the Windows shell and handles application lifecycle events.
ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
    App();

    // IFrameworkView Methods.
    virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

private:
    // Application lifecycle event handlers.
    void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
    void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
    void OnResuming(Platform::Object^ sender, Platform::Object^ args);

    // Window event handlers.
    void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
    void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
    void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
    void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnPointerWheelChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs ^ args);
    void OnKeyPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);

    void RefreshCachedGamepads();
    Windows::Gaming::Input::Gamepad^ App::GetLastGamepad();

    void RestartRuntime(Windows::Foundation::Rect bounds);
    std::unique_ptr<Babylon::Graphics::Device> m_device{};
    std::unique_ptr<Babylon::Graphics::DeviceUpdate> m_update{};
    std::unique_ptr<Babylon::AppRuntime> m_runtime{};
    Babylon::Plugins::NativeInput* m_nativeInput{};
    std::unique_ptr<Babylon::Polyfills::Canvas> m_nativeCanvas{};
    Windows::Foundation::Collections::IVectorView<Windows::Storage::IStorageItem^>^ m_files;
    bool m_windowClosed;
    bool m_windowVisible;
    float m_displayScale{ 1.f };

    //Gamepad states
    Platform::Collections::Vector<Windows::Gaming::Input::Gamepad^>^ m_localCollection;
    Windows::Gaming::Input::GamepadReading      m_reading;
    Windows::Gaming::Input::Gamepad^ m_currentGamepad;

    std::atomic<bool>       m_currentGamepadNeedsRefresh;
    std::wstring            m_buttonString;
    double                  m_leftTrigger;
    double                  m_rightTrigger;
    double                  m_leftStickX;
    double                  m_leftStickY;
    double                  m_rightStickX;
    double                  m_rightStickY;
   
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
