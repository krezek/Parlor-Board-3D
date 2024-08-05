call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"

texassemble cube -o sky.dds "sky\sky+x.png" "sky\sky-x.png" "sky\sky+y.png" "sky\sky-y.png" "sky\sky+z.png" "sky\sky-z.png"

texconv /Y *.png

texconv -vflip /Y Monkey_Head.png
texconv -vflip /Y Monkey_Body.png
texconv -vflip /Y Monkey_Eye.png
texconv -vflip /Y Bird.png

move /Y *.dds ..\Textures


pause