# T4ShaderTool
simple tool that automates compiling shaders for World At War

## Usage:
Download from [releases](https://github.com/Clippy95/T4ShaderTool/releases) then drag and drop contents into WaWDirectory\raw\shader_bin, run exe in CMD and it'll compile all shaders within shader_src folder and place compiled shaders within shader_bin, otherwise type out names for each shader to compile individually

## Credits:
Otso O: [original shader_tool for COD4: ](https://pastebin.com/4be66PEU)

xoxor4d: [Introduction to custom CoD4-shaders](https://xoxor4d.github.io/tutorials/hlsl-intro/) <-- recommend reading this 


### Note
I really recommend reading [Introduction to custom CoD4-shaders](https://xoxor4d.github.io/tutorials/hlsl-intro/) as the steps 99% apply to WaW just use this tool instead of the one within that tutorial, and at step "4. Technique > root\raw\techniques" you have to provide the full shader name with the extension "z_scrolling_hud.hlsl" rather than "z_scrolling_hud"

## Issues
Also linker_pc seems to not like anything with postfx in it?, for example z_postfx_cellshading wont be linked but z_cellshading will be? im not sure if this is an issue with my hashing or what right now..
