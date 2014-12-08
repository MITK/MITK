/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include <QApplication>
#include <QWidget>
#include "mitkGPGPU.h"

#include <iostream>

#define GPGPU_INFO MITK_INFO("mitk.gpgpu")
#define GPGPU_ERROR MITK_ERROR("mitk.gpgpu")
#define GPGPU_CHECKGLERR MITK_ERROR(glGetError()!=GL_NO_ERROR)("mitk.gpgpu") << "GL ERROR @ "


#define OPERATING_TEXTURE GL_TEXTURE15

static GLint convertTextureFormatToInternalFormatGL(mitk::GPGPU::TextureFormat format)
{
  switch(format)
  {
    case mitk::GPGPU::FLOAT32_LUMINANCE:       return GL_LUMINANCE_FLOAT32_ATI;
    case mitk::GPGPU::FLOAT32_LUMINANCE_ALPHA: return GL_LUMINANCE_ALPHA_FLOAT32_ATI;
    case mitk::GPGPU::FLOAT32_RGBA:            return GL_RGBA32F_ARB;
    case mitk::GPGPU::UINT8_RGBA:              return GL_RGBA8;
  }
  return 0;
}

static GLint convertTextureFormatToFormatGL(mitk::GPGPU::TextureFormat format)
{
  switch(format)
  {
    case mitk::GPGPU::FLOAT32_LUMINANCE:       return GL_LUMINANCE;
    case mitk::GPGPU::FLOAT32_LUMINANCE_ALPHA: return GL_LUMINANCE_ALPHA;
    case mitk::GPGPU::FLOAT32_RGBA:            return GL_RGBA;
    case mitk::GPGPU::UINT8_RGBA:              return GL_RGBA;
  }
  return 0;
}

static GLint convertTextureFormatToTypeGL(mitk::GPGPU::TextureFormat format)
{
  switch(format)
  {
    case mitk::GPGPU::FLOAT32_LUMINANCE:       return GL_FLOAT;
    case mitk::GPGPU::FLOAT32_LUMINANCE_ALPHA: return GL_FLOAT;
    case mitk::GPGPU::FLOAT32_RGBA:            return GL_FLOAT;
    case mitk::GPGPU::UINT8_RGBA:              return GL_UNSIGNED_BYTE;
  }
  return 0;
}


int mitk::GPGPU::Texture::GetWidth(){return myWidth;}
int mitk::GPGPU::Texture::GetHeigth(){return myHeight;}
int mitk::GPGPU::Texture::GetDepth(){return myDepth;}


mitk::GPGPU::Texture::Texture(mitk::GPGPU::TextureFormat format,int width,int height,int depth)
{
  if(depth==0)
    glTarget=GL_TEXTURE_2D;
  else
    glTarget=GL_TEXTURE_3D;

  myFormat=format;
  myWidth=width;
  myHeight=height;
  myDepth=depth;

  GLuint handle;

  glGenTextures(1,&handle);
  glTextureHandle = handle;
  glActiveTexture(OPERATING_TEXTURE);
  glBindTexture(glTarget,glTextureHandle);

  GPGPU_CHECKGLERR << "allocating texture handle";

  if(glTarget==GL_TEXTURE_2D)
  {
    glTexImage2D(GL_TEXTURE_2D,0,convertTextureFormatToInternalFormatGL(myFormat),width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
//    glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);


  }
  else
  {
    glTexImage3D(GL_TEXTURE_3D,0,convertTextureFormatToInternalFormatGL(myFormat),width,height,depth,0,GL_RGBA,GL_UNSIGNED_BYTE,0);
//  glGenerateMipmap(GL_TEXTURE_3D);

  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  }

  GPGPU_CHECKGLERR << "declaring texture format&dimensions";

  glGenFramebuffers(1,&handle);
  glFBOHandle = handle;

  GPGPU_CHECKGLERR << "allocating framebuffer object";
}

mitk::GPGPU::Texture::~Texture()
{
  GLuint handle;

  handle=glFBOHandle;
  glDeleteFramebuffers(1,&handle);
  GPGPU_CHECKGLERR << "deleting framebufferobject";

  handle=glTextureHandle;
  glDeleteTextures(1,&handle);
  GPGPU_CHECKGLERR << "deleting texture handle";
}

void mitk::GPGPU::Texture::ActivateAsSource(int unit)
{
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(glTarget,glTextureHandle);

  GPGPU_CHECKGLERR << "binding texture to unit";
}

void mitk::GPGPU::Texture::ActivateAsDestination()
{

  static GLenum buffers[5][4] =
  {
    { GL_NONE, GL_NONE, GL_NONE, GL_NONE },
    { GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE, GL_NONE },
    { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_NONE, GL_NONE },
    { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_NONE },
    { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 },
  };


  glBindFramebuffer( GL_FRAMEBUFFER, glFBOHandle );
  glDrawBuffers(4, buffers[1]);
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glTarget,glTextureHandle,0);

  GPGPU_CHECKGLERR << "associating texture to framebufferobject";

  int error = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(error)
  {
     case GL_FRAMEBUFFER_COMPLETE_EXT:
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
       GPGPU_ERROR << "Incomplete attachment\n";break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      GPGPU_ERROR << "Missing attachment\n";break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      GPGPU_ERROR << "Incomplete dimensions\n";break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      GPGPU_ERROR << "Incomplete formats\n";break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      GPGPU_ERROR << "Incomplete draw buffer\n";break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      GPGPU_ERROR << "Incomplete read buffer\n";break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      GPGPU_ERROR << "Framebufferobjects unsupported\n";break;
    default:
      GPGPU_ERROR << "unknown framebuffer status\n";break;
  }

  glViewport(0,0,myWidth,myHeight);

  GPGPU_CHECKGLERR << "setting viewport";
}

void mitk::GPGPU::Texture::Upload(TextureFormat inputformat,const void *src)
{
  glActiveTexture(OPERATING_TEXTURE);
  glBindTexture(glTarget,glTextureHandle);

  if(glTarget==GL_TEXTURE_2D)
  {
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,myWidth,myHeight,convertTextureFormatToFormatGL(inputformat),convertTextureFormatToTypeGL(inputformat),src);
  }
  else
  {
    glTexSubImage3D(GL_TEXTURE_3D,0,0,0,0,myWidth,myHeight,myDepth,convertTextureFormatToFormatGL(inputformat),convertTextureFormatToTypeGL(inputformat),src);

  }

  GPGPU_CHECKGLERR << "texture upload to gpu";

}

void mitk::GPGPU::Texture::Download(TextureFormat inputformat,void *dst)
{
  glActiveTexture(OPERATING_TEXTURE);
  glBindTexture(glTarget,glTextureHandle);

  if(glTarget==GL_TEXTURE_2D)
  {
    glGetTexImage( GL_TEXTURE_2D,0,convertTextureFormatToFormatGL(inputformat),convertTextureFormatToTypeGL(inputformat),dst);
    GPGPU_CHECKGLERR << "texture download to cpu";
  }
  else
  {
  }
}

static char stubVertexShader[] =
  "void main() { gl_Position = vec4( 2*gl_Vertex.xy-1,0,1 ); }\n";


mitk::GPGPU::Shader::Shader(char *source)
{
  //std::cout << "compiling shader:\n" << source << std::endl;

  glHandleVertex = glCreateShader(GL_VERTEX_SHADER);
  glHandleFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glHandleProgram = glCreateProgram();

  GLchar *src[2];

  src[0] = stubVertexShader;
  src[1] = 0;

  glShaderSource(glHandleVertex,1,(const GLchar **)src,0);

  src[0] = source;
  src[1] = 0;

  glShaderSource(glHandleFragment,1,(const GLchar **)src,0);

  bool failed=false;

  GLint _sv,_sf,_sl;

  glCompileShader( glHandleVertex );
  GPGPU_CHECKGLERR << "compiling vertex shader";
  glGetShaderiv( glHandleVertex, GL_COMPILE_STATUS, &_sv);
  if( !_sv)
  {
    GPGPU_ERROR << "vertex shader compilation failed\n";
    failed=true;
  }

  glCompileShader( glHandleFragment );
  GPGPU_CHECKGLERR << "compiling fragment shader";
  glGetShaderiv( glHandleFragment, GL_COMPILE_STATUS, &_sf);
  if( !_sf)
  {
    GPGPU_ERROR << "fragment shader compilation failed\n";
    failed=true;
  }

  glAttachShader( glHandleProgram,glHandleVertex );
  glAttachShader( glHandleProgram,glHandleFragment );
  glLinkProgram( glHandleProgram );
  GPGPU_CHECKGLERR << "linking shader program";
  glGetProgramiv( glHandleProgram, GL_LINK_STATUS, &_sl);
  if( !_sl)
  {
    GPGPU_ERROR << "shader linkage failed\n";
    failed=true;
  }

  if(failed)
  {
     int infologLength = 0;
     int charsWritten  = 0;
     char *infoLog;

     glGetProgramiv(glHandleProgram, GL_INFO_LOG_LENGTH,&infologLength);

     if (infologLength > 0)
     {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(glHandleProgram, infologLength, &charsWritten, infoLog);
        GPGPU_ERROR << "SHADER CREATION FAILED INFOLOG:\n" << infoLog;
        free(infoLog);
     }
  }

}

mitk::GPGPU::Shader::~Shader()
{
  glDeleteProgram( glHandleProgram);
  glDeleteShader( glHandleVertex );
  glDeleteShader( glHandleFragment );
}

void mitk::GPGPU::Shader::Activate()
{
  glUseProgram( glHandleProgram );
  GPGPU_CHECKGLERR << "activating shader";
}

int mitk::GPGPU::Shader::GetUniformLocation(char *name)
{
  return glGetUniformLocation(glHandleProgram,name);
}


void mitk::GPGPU::Shader::SetUniform(char *name,int i0)
{
  glUniform1i( GetUniformLocation(name) , i0);

  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,int i0,int i1)
{
  GLint i[2];
  i[0]=i0;
  i[1]=i1;
  glUniform2iv(  GetUniformLocation(name) , 1 , i );
  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,int i0,int i1,int i2)
{
  GLint i[3];
  i[0]=i0;
  i[1]=i1;
  i[2]=i2;
  glUniform3iv(  GetUniformLocation(name) , 1 , i );
  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,int i0,int i1,int i2,int i3)
{
  GLint i[4];
  i[0]=i0;
  i[1]=i1;
  i[2]=i2;
  i[3]=i3;
  glUniform4iv(  GetUniformLocation(name) , 1 , i );
  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,float i0)
{
  GLint location = GetUniformLocation(name);
  glUniform1f(location,i0);
  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,float i0,float i1)
{
  GLfloat i[2];
  i[0]=i0;
  i[1]=i1;
  glUniform2fv(GetUniformLocation(name),1,i);
  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,float i0,float i1,float i2)
{
  GLfloat i[3];
  i[0]=i0;
  i[1]=i1;
  i[2]=i2;
  glUniform3fv(GetUniformLocation(name),1,i);
  GPGPU_CHECKGLERR << "setting uniform";
}

void mitk::GPGPU::Shader::SetUniform(char *name,float i0,float i1,float i2,float i3)
{
  GLfloat i[4];
  i[0]=i0;
  i[1]=i1;
  i[2]=i2;
  i[3]=i3;
  glUniform4fv(GetUniformLocation(name),1,i);
  GPGPU_CHECKGLERR << "setting uniform";
}

#ifdef _WIN32
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
    case WM_CLOSE:
      DestroyWindow(hwnd);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}
#endif

mitk::GPGPU::GPGPU()
{

#ifdef _WIN32
               /*
  WNDCLASSEX wcx;

  // Fill in the window class structure with parameters
  // that describe the main window.

  wcx.cbSize = sizeof(wcx);          // size of structure
  wcx.style = CS_HREDRAW |
    CS_VREDRAW;                    // redraw if size changes
  wcx.lpfnWndProc = MainWndProc;     // points to window procedure
  wcx.cbClsExtra = 0;                // no extra class memory
  wcx.cbWndExtra = 0;                // no extra window memory
  wcx.hInstance = GetModuleHandle(NULL);         // handle to inst ance
  wcx.hIcon = LoadIcon(NULL,
    IDI_APPLICATION);              // predefined app. icon
  wcx.hCursor = LoadCursor(NULL,
    IDC_ARROW);                    // predefined arrow
  wcx.hbrBackground = NULL;                  // white background brush
  wcx.lpszMenuName = (LPCSTR) "MainMenu";    // name of menu resource
  wcx.lpszClassName = (LPCSTR) "MainWClass";  // name of window class
  wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  // Register the window class.

  if(!RegisterClassEx(&wcx))
    std::cout << "failed registering window class\n";

  HWND desktopWindow=CreateWindowEx(
    WS_EX_CLIENTEDGE,
    (LPCSTR)"MainWClass",
    (LPCSTR)"Anatomy of a Window",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
    NULL,
    NULL,
    GetModuleHandle(NULL),
    NULL);
  windowHandle = desktopWindow;
  ShowWindow(desktopWindow, SW_RESTORE);

  if(desktopWindow==0)
    std::cout << "failed creating window\n";
                 */

  HWND desktopWindow= (HWND)QApplication::topLevelWidgets().at(0)->winId();

  windowsContext = GetDC(desktopWindow);

  if(windowsContext==0)
    std::cout << "failed getting window device context\n";

  static PIXELFORMATDESCRIPTOR pfd =// pfd Tells Windows How We Want Things To Be
  {
    sizeof(PIXELFORMATDESCRIPTOR),  // Size Of This Pixel Format Descriptor
    1,                              // Version Number
    PFD_DRAW_TO_WINDOW |            // Format Must Support Window
    PFD_SUPPORT_OPENGL |            // Format Must Support OpenGL
    PFD_DOUBLEBUFFER |
    PFD_SWAP_EXCHANGE ,              // Must Support Double Buffering
    PFD_TYPE_RGBA,                  // Request An RGBA Format
    24,                              // Select Our Color Depth
    0, 0, 0, 0, 0, 0,                // Color Bits Ignored  if(openGLContext==0)
    0,                              // No Alpha Buffer
    0,                              // Shift Bit Ignored
    0,                              // No Accumulation Buffer
    0, 0, 0, 0,                      // Accumulation Bits Ignored
    0,                              // 16Bit Z-Buffer (Depth Buffer)
    0,                              // No Stencil Buffer
    0,                              // No Auxiliary Buffer
    PFD_MAIN_PLANE,                  // Main Drawing Layer
    0,                              // Reserved
    0, 0, 0                          // Layer Masks Ignored
  };


  // Sonstiges einstellen
  int iFormat = ChoosePixelFormat(windowsContext,&pfd);
  SetPixelFormat(windowsContext,iFormat,&pfd);


  openGLContext = wglCreateContext(windowsContext);

  int errw=GetLastError();

  if(openGLContext==0)
    std::cout << "failed creating openGL context "<<errw<<"\n";

#else

  X_display = XOpenDisplay(NULL);

  GPGPU_ERROR( !X_display ) << "cant open X display";

  GLX_drawable = QApplication::topLevelWidgets().at(0)->winId();

  GPGPU_ERROR( !GLX_drawable ) << "cant get toplevel widget from QT";

  static int visAttributes[] = {
    GLX_RGBA,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    GLX_DOUBLEBUFFER,
    None
  };

  XVisualInfo *visinfo = glXChooseVisual(X_display, 0, visAttributes);

  GPGPU_ERROR(!visinfo) << "Unable to choose specified visual!";

  openGLContext = glXCreateContext(X_display, visinfo, 0, true);

  if(visinfo)
     XFree(visinfo);

  GPGPU_ERROR(!openGLContext) << "cant create GLX context";

#endif

  Activate();

  GPGPU_INFO << "initializing glew";

  int err=glewInit();

  GPGPU_CHECKGLERR << "initializing glew";
  GPGPU_ERROR(GLEW_OK != err) << "glewInit() fails with " << err << " as text: " << glewGetErrorString(err);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,1,0,1,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  GPGPU_CHECKGLERR << "intializing projection&modelview matrix";

  glDisable(GL_CULL_FACE);
  glShadeModel(GL_SMOOTH);              // Enable Smooth Shading
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);        // Black Background
  glClearDepth(1.0f);                  // Depth Buffer Setup
  glDisable(GL_DEPTH_TEST);              // Enables Depth Testing
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Really Nice Perspective Calculations
  glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
  glDepthMask(false);

  GPGPU_CHECKGLERR << "setting up openGL context";
}

mitk::GPGPU::~GPGPU()
{
#ifdef _WIN32

  wglDeleteContext( openGLContext );

#else

  if(openGLContext)
    glXDestroyContext(X_display,openGLContext);

  if(X_display)
    XCloseDisplay(X_display);

#endif

}

void mitk::GPGPU::Activate()
{

#ifdef _WIN32
   wglMakeCurrent(windowsContext,openGLContext);
#else
   glXMakeCurrent(X_display, GLX_drawable, openGLContext);
#endif

   GPGPU_CHECKGLERR << "activating openGL context";
}

void mitk::GPGPU::Deactivate()
{
}

void mitk::GPGPU::Run()
{
  glBegin( GL_TRIANGLE_STRIP );
  glVertex2f( 0,0 );
  glVertex2f( 0,1 );
  glVertex2f( 1,0 );
  glVertex2f( 1,1 );
  glEnd();

  GPGPU_CHECKGLERR << "running a shader";
}

void mitk::GPGPU::Run(float start,float end)
{
  glBegin( GL_TRIANGLE_STRIP );
  glVertex2f( 0,start );
  glVertex2f( 0,end );
  glVertex2f( 1,start );
  glVertex2f( 1,end );
  glEnd();

  GPGPU_CHECKGLERR << "running a shader";
}


