Welcome to my (currently unnamed) game engine! To get started, go ahead and clone this repo. Then, open the solution in Visual Studio 2019, and set the build target to Release x86.  
Before you can run the engine, you'll need to set up all of the dependencies. Please download the dependencies that I built and organized here: https://drive.google.com/open?id=1EBBp0EwNXkGeG7nwDEv8T_3B13rlTofL  
Place the libs folder directly in ".\CPPGameEngine\CPPGameEngine" and place all of the files from the dlls folder in ".\CPPGameEngine\Release" (you will need to create the Release folder if you haven't built yet).  
If you're not on Windows, You'll need to build the dependencies yourself. Please take a look at the libs folder from the download above to see how to structure the build (hopefully I'll automate this later).  

Here is the list of dependencies you'll need to build:
- assimp (model loading)
- freetype (font loading)
- glad (openGL loader)
- glfw (openGL framework)
- glm (openGL mathematics)
- physX (physics engine)
- openALsoft (audio playback)  

Here is the list of additional dependencies that are already bundled with the engine:
- stb_vorbis (audio loading and decoding)
- stb_image (image loading)  