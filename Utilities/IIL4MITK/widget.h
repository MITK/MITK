#ifndef IIL_WIDGET_WRAPPER
#define IIL_WIDGET_WRAPPER

#include <assert.h>
#include <vtkRenderWindow.h>
// #include <mitkGL.h>

#ifdef WIN32
  #include <windows.h>
#endif

#ifndef __APPLE__
  #include "GL/gl.h"
#else
  #include "OpenGL/gl.h"
#endif


class iil4mitkItem;

typedef vtkRenderWindow iil4mitkWidget;

#endif
