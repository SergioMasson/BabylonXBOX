/// <reference path="node_modules/babylonjs/babylon.module.d.ts" />
/// <reference path="node_modules/babylonjs-loaders/babylonjs.loaders.module.d.ts" />
/// <reference path="node_modules/babylonjs-materials/babylonjs.materials.module.d.ts" />
/// <reference path="node_modules/babylonjs-gui/babylon.gui.module.d.ts" />

var logfps = true;

var engine = new BABYLON.NativeEngine();
var scene = new BABYLON.Scene(engine);

var box = BABYLON.Mesh.CreateBox("box1", 0.2, scene);
scene.createDefaultCamera(true, true, true);
scene.activeCamera.alpha += Math.PI;
scene.createDefaultLight(true);

if (logfps) {
    var logFpsLoop = function () {
        BABYLON.Tools.Log("FPS: " + Math.round(engine.getFps()));
        window.setTimeout(logFpsLoop, 1000);
    };

    window.setTimeout(logFpsLoop, 3000);
}

engine.runRenderLoop(function () {
    scene.render();
});

function SetGamepad(buttons, RightTrigger, LeftTrigger ,LeftThumbstickX, LeftThumbstickY, RightThumbstickX, RightThumbstickY, )
{
    box.rotate(new BABYLON.Vector3(0, -1, 0), LeftThumbstickX * 0.01, BABYLON.Space.WORLD);
    box.rotate(new BABYLON.Vector3(-1, 0, 0), LeftThumbstickY * 0.01, BABYLON.Space.WORLD);
}