@ECHO OFF

pushd ..\Astranox-Rasterization\assets\shaders
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vertex square.vert -o square-vert.spv
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=fragment square.frag -o square-frag.spv
popd

PAUSE