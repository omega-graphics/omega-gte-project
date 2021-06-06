# OmegaGTE (Omega Graphics and Tessalation Engine)
A cross platform low level api for rendering 3D graphics, performing computations on a gpu, and performing tessalations on 2D and 3D geometric primatives.

## No OpenGL Code Policy

We will not be supporting OpenGL nor OpenGL ES.

#### Why no OpenGL support?

The latest update of OpenGL was released in 2017 and as Vulkan gets more mature, the Khronos Group is slowly fading out support for OpenGL and OpenGL ES. Both APIs are very well implemented however OpenGL, being almost 30 years old, has a lot of old and deprecated functions from older times when graphics hardware was more primitive. Additionally, Vulkan has proper builtin support for Ray Tracing unlike its predecessor.