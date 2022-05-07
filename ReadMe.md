Welcome to Raider Engine! To get started, go ahead and clone this repo. Then, run cmake to build the project (the first build will take some time to enumerate the vendored dependencies).  
  
Dependencies:  
 • assimp (model loading)  
 • freetype (font loading)  
 • glad (openGL loader)  
 • glfw (openGL framework)  
 • glm (openGL mathematics)  
 • physX (physics engine)  
 • openALsoft (audio playback)  
 • NvAPI (optional - Nvidia application profile configuration)  
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