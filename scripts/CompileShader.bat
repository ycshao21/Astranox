@ECHO OFF
setlocal

set SHADER_DIR=..\Astranox-Rasterization\assets\shaders
set GLSLC=%VULKAN_SDK%\Bin\glslc.exe

pushd %SHADER_DIR%

%GLSLC% -fshader-stage=vertex PureColor.vert -o PureColor-Vert.spv
%GLSLC% -fshader-stage=fragment PureColor.frag -o PureColor-Frag.spv

%GLSLC% -fshader-stage=vertex Texture.vert -o Texture-Vert.spv
%GLSLC% -fshader-stage=fragment Texture.frag -o Texture-Frag.spv

popd

endlocal
PAUSE