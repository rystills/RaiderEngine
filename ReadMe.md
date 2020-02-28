Welcome to the Raider Engine! To get started, go ahead and clone this repo. Then, open the solution in Visual Studio 2019, and set the build target to Release x86.  
Before you can run the engine, you'll need to set up all of the dependencies. Please download the dependencies that I built and organized here: https://drive.google.com/file/d/1StlyWMz9VLqMng-m0tq16PJksK3BDG81/view?usp=sharing  
Place the libs folder directly in ".\RaiderEngine\RaiderEngine" and place all of the files from the dlls folder in ".\RaiderEngine\Release" (you will need to create the Release folder if you haven't built yet).  
If you're not on Windows, you'll need to build the dependencies yourself. Please take a look at the libs folder from the download above to see how to structure the build. This build process will be automated in the future.  
  
Here is the list of dependencies you'll need to build:  
 • assimp (model loading)  
 • freetype (font loading)  
 • glad (openGL loader)  
 • glfw (openGL framework)  
 • glm (openGL mathematics)  
 • physX (physics engine)  
 • openALsoft (audio playback)  
 • NvAPI (Nvidia application profile configuration)  
  
Here is the list of additional dependencies that are already bundled with the engine:  
 • stb_vorbis (audio loading and decoding)  
 • stb_image (image loading)  
  
Previews:  
<img src="previews\2_28_20 (3d carousel).png" width="960" height="540">  
<img src="previews\1_30_20 (brick breaker).png" width="960" height="540">  
<img src="previews\1_31_20 (2d particles).png" width="960" height="540">  
<img src="previews\2_17_20 (2d platformer).png" width="960" height="540">