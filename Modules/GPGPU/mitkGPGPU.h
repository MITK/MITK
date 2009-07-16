/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-08-25 18:10:57 +0200 (Mon, 25 Aug 2008) $
Version:   $Revision: 15062 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGPGPU_H
#define MITKGPGPU_H

#include <GL/glew.h>


#if defined(_WIN32)
  #ifdef mitkGPGPU_EXPORTS
    #define GPGPU_DLL_API __declspec(dllexport)
  #else
    #define GPGPU_DLL_API __declspec(dllimport)
  #endif
#else
  #define GPGPU_DLL_API
#endif


#ifdef _WIN32
#include <windows.h>
#else
#include <GL/glx.h>
#include <X11/Xlib.h>
#endif

#include <GL/gl.h>


#include "mitkCommon.h"


namespace mitk {

//##Documentation
//## @brief GPGPU

class GPGPU_DLL_API GPGPU
{
  public:

    enum TextureFormat
    {
      FLOAT32_LUMINANCE,
      FLOAT32_LUMINANCE_ALPHA,
      FLOAT32_RGBA,
      UINT8_RGBA,
    };

    class GPGPU_DLL_API Texture {

      public:

        Texture(mitk::GPGPU::TextureFormat format,int width,int height,int depth=0);
        ~Texture();

        void ActivateAsSource(int unit);
        void ActivateAsDestination();
        void Upload(mitk::GPGPU::TextureFormat inputformat,const void *src);
        void Download(mitk::GPGPU::TextureFormat inputformat,void *dst);
		int GetWidth();
		int GetHeigth();
		int GetDepth();

      private:

        mitk::GPGPU::TextureFormat myFormat;
        int myWidth,myHeight,myDepth;

        int glTarget;
        int glInternalFormat;
        int glTextureHandle;
        int glFBOHandle;
    };

    class GPGPU_DLL_API Shader {

      public:

        Shader(char *source);
        ~Shader();

        void Activate();
        void SetUniform(char *name,int i0);
        void SetUniform(char *name,int i0,int i1);
        void SetUniform(char *name,int i0,int i1,int i2);
        void SetUniform(char *name,int i0,int i1,int i2,int i3);
        void SetUniform(char *name,float i0);
        void SetUniform(char *name,float i0,float i1);
        void SetUniform(char *name,float i0,float i1,float i2);
        void SetUniform(char *name,float i0,float i1,float i2,float i3);

      private:

        int GetUniformLocation(char *name);

        int glHandleVertex;
        int glHandleFragment;
        int glHandleProgram;


    };

    GPGPU();
    ~GPGPU();

    void Activate();
    void Deactivate();
    void Run();

  private:
#ifdef _WIN32
    HWND windowHandle;
    HDC windowsContext;
    HGLRC openGLContext;
#else
    GLXContext openGLContext;
    GLXDrawable GLX_drawable;
    Display *X_display;
    Window *windowHandle;
#endif


};

} // namespace mitk



/*{
  mitk::GPGPU *gpu=new mitk::GPGPU();

  gpu->Activate();

  mitk::GPGPU::Texture *input=new mitk::GPGPU::Texture( GLSL_FLOAT32_LUMINANCE , 128,128 );
  mitk::GPGPU::Texture *output=new mitk::GPGPU::Texture( GLSL_FLOAT32_LUMINANCE , 128,128 );
  mitk::GPGPU::Shader *shader = new mitk::GPGPU::Shader(
    "uniform float invsize;\n"
    "uniform sampler2D input;\n"
    "uniform sampler2D input2;\n"
    "uniform sampler2D input3;\n"
    "void main()\n"
    "{\n"
    "  vec2 pos=gl_FragCoord.xy*invsize; \n"
    "  int x; \n"
    "  float y; \n"
    "  vec2 z; \n"
    "  vec4 ddd; \n"
    "  vec4 gl_FragColor; \n"
    "  float x = texture2D(input,vec2(x,y) );
    " gl_FragColor.r = x;    \n"
    "}\n"          );

  input->Upload( GLSL_FLOAT32_LUMINANCE, eurerArray);


  shader->Activate();
  shader->SetUniform("input",0);

  gpu->ActivateAsSource(input,0);
  gpu->ActivateAsDestination(output);
  gpu->Run();


  input2->ActivateAsSource(0);
  output2->ActivateAsDestination();
  gpu->Run();

  output->Download( GLSL_FLOAT32_LUMINANCE, zielArray);

  input2->Upload( GLSL_FLOAT32_LUMINANCE, eurerArray);

  input2->ActivateAsSource(0);
  output2->ActivateAsDestination();
  shader->SetUniform("input",0);





  gpu->Deactivate();

  delete gpu;
}*/


#endif
