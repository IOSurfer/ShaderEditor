cd ..
cd build 
mkdir Shader
cd ..
glslc.exe Shader/Shader.vert -o build/Shader/Vert.spv
glslc.exe Shader/Shader.frag -o build/Shader/Frag.spv