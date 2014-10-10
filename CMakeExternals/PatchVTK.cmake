set(path "Rendering/OpenGL/vtkXOpenGLRenderWindow.cxx")
file(STRINGS ${path} contents NEWLINE_CONSUME)
string(REPLACE "//#define GLX_GLXEXT_LEGACY" "#define GLX_GLXEXT_LEGACY" contents ${contents})
