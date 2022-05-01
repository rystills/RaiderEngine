Welcome to the Raider Engine! To get started, go ahead and clone this repo. Then, open the solution in Visual Studio 2019, and set the build target to Release x86.  
Before you can run the engine, you'll need to set up all of the dependencies. If you're on  Windows, go ahead and download the dependencies that I built and organized here: https://drive.google.com/file/d/1StlyWMz9VLqMng-m0tq16PJksK3BDG81/view?usp=sharing  
Place the libs folder directly in ".\RaiderEngine\RaiderEngine" and place all of the files from the dlls folder in ".\RaiderEngine\Release" (you will need to create the Release folder if you haven't built yet).  
If you're not on Windows, you'll need to build the dependencies yourself. Reference the libs folder in the above drive link to see how to structure the dependencies. This build process will be automated in the future.  
  
Dependencies:  
 • assimp (model loading)  
 • freetype (font loading)  
 • glad (openGL loader)  
 • glfw (openGL framework)  
 • glm (openGL mathematics)  
 • physX (physics engine)  
 • openALsoft (audio playback)  
 • NvAPI (Nvidia application profile configuration)  
  
Additional dependencies (bundled with the engine):  
 • stb_vorbis (audio loading and decoding)  
 • stb_image (image loading)  
  
Current Features:  
 • 2D GameObjects  
 • 2D collision system  
 • 2D Tilemaps  
 • 2D Particle Emitters  
 • 2D rendering  
 • 3D GameObjects  
 • 3D physics  
 • 3D PointLights  
 • 3D scene loading (using FBX files + node names for simplicity and potential round tripping)  
 • 3D deferring rendering with lighting, shadows, and dithered (screen door) transparency  
 • normal, specular, diffuse, and emissive map support  
 • point and line rendering  
 • font loading / text rendering  
 • sound loading / playback  
  
Planned Features:  
 • HDR  
 • spot / directional lights  
 • baked shadow maps  
 • skyboxes  
 • SMAA  
 • automatic dependency building  
  
Previews:  
<img src="previews\2_28_20 (3d carousel - hallway).png" width="960" height="540">  
<img src="previews\4_5_20 (3d carousel - storage closet).png" width="960" height="540">  
<img src="previews\1_30_20 (brick breaker).png" width="960" height="540">  
<img src="previews\1_31_20 (2d particles).png" width="960" height="540">  
<img src="previews\2_17_20 (2d platformer).png" width="960" height="540">