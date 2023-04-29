set(BABYLON_SCRIPTS
    "Scripts/node_modules/babylonjs-loaders/babylonjs.loaders.js"
    "Scripts/node_modules/babylonjs-loaders/babylonjs.loaders.js.map"
    "Scripts/node_modules/babylonjs/babylon.max.js"
    "Scripts/node_modules/babylonjs/babylon.max.js.map"
    "Scripts/node_modules/babylonjs-materials/babylonjs.materials.js"
    "Scripts/node_modules/babylonjs-materials/babylonjs.materials.js.map"
    "Scripts/node_modules/babylonjs-gui/babylon.gui.js"
    "Scripts/node_modules/babylonjs-gui/babylon.gui.js.map")

set(SCRIPTS "Scripts/experience.js")

set(APPX_FILES "Package.appxmanifest" "TemporaryKey.pfx")
set_property(SOURCE ${APPX_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set(APPX_ASSETS
        "Assets/LockScreenLogo.scale-200.png"
        "Assets/SplashScreen.scale-200.png"
        "Assets/Square44x44Logo.scale-200.png"
        "Assets/Square44x44Logo.targetsize-24_altform-unplated.png"
        "Assets/Square150x150Logo.scale-200.png"
        "Assets/StoreLogo.png"
        "Assets/Wide310x150Logo.scale-200.png")

set_property(SOURCE ${APPX_ASSETS} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${APPX_ASSETS} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
set(SOURCES ${SOURCES} ${APPX_FILES} ${APPX_ASSETS} "Source/App.cpp" "Source/App.h")

add_executable(Playground ${BABYLON_SCRIPTS} ${SCRIPTS} ${SOURCES} ${RESOURCE_FILES})

warnings_as_errors(Playground)

target_compile_definitions(Playground PRIVATE UNICODE)
target_compile_definitions(Playground PRIVATE _UNICODE)
target_compile_options(Playground PRIVATE /ZW)
target_compile_options(Playground PRIVATE /await)

target_include_directories(Playground PRIVATE "Source" ".")

target_link_to_dependencies(Playground
    PRIVATE AppRuntime
    PRIVATE NativeEngine
    PRIVATE NativeInput
    PRIVATE NativeOptimizations
    PRIVATE Console
    PRIVATE Window
    PRIVATE ScriptLoader
    PRIVATE XMLHttpRequest
    PRIVATE Canvas
    ${BABYLON_NATIVE_PLAYGROUND_EXTENSION_LIBRARIES})

set_property(SOURCE ${SCRIPTS} ${BABYLON_SCRIPTS} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${SCRIPTS} ${BABYLON_SCRIPTS} PROPERTY VS_DEPLOYMENT_LOCATION "Scripts")

set_property(TARGET Playground PROPERTY FOLDER Apps)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/node_modules PREFIX Scripts FILES ${BABYLON_SCRIPTS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SCRIPTS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Playground)