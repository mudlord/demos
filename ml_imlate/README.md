This is not an introsystem, neither it is a tool nor the
source code of any intro. This is a base code or framework
that you can use to build your own introsystem. 

This has been used in some productions already.
Special thanks go to Inigo Quilez [http://www.iquilezles.org], for which this framework is based on.

It has been extensively modified to have:
 
* Support for GL 3.3 core with forward functionality
* Cut down GL function loading
* Initialization code for OGL, with extensions for compressed textures, shaders, and FBOs
* Code for loading textures to files
* A basic timing scheme for timing scenes to elapsed time. Can also use GNU Rocket if needed.
* FFT code for syncing visuals to audio.
* Support for linking against MSVCRT.DLL, not chunky runtimes like MSVC2013's
* Small footprint (12 or so kb when compressed)

To use, modify the intro_* files to hook into your relevant effects/shaders/routines. 
Use MSVC2013 or less to compile. It will take a lot of work to compile for newer MSVC versions.


 
