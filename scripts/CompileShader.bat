@ECHO OFF

pushd ..\Astranox-Rasterization\assets\shaders
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=vertex triangle.vert -o vert.spv
%VULKAN_SDK%\Bin\glslc.exe -fshader-stage=fragment triangle.frag -o frag.spv
popd

PAUSE