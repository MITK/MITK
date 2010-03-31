/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//#ifndef FLMITKRENDERWINDOW_H_HEADER_INCLUDED
//#define FLMITKRENDERWINDOW_H_HEADER_INCLUDED
//#include <iostream>
//#include <Fl/Fl_Gl_Window.h>
//#include "mitkGL.h"
//#include "mitkRenderWindow.h"
//#include "mitkBaseRenderer.h"
//
//
//class FLmitkRenderWindow : public mitk::RenderWindow, public Fl_Gl_Window {
//  public:  
//  FLmitkRenderWindow(int x,int y,int width,int height,const char* label = "fltk gl win") : mitk::RenderWindow(label,NULL ), Fl_Gl_Window(x,y,width,height), m_InitNeeded(true),m_ResizeNeeded(true) { std::cout << "c'tor FLmitkRenderWindow" << std::endl;} 
//  
//  virtual void MakeCurrent() { make_current(); }
//
//  //##Documentation
//  //## @brief  Swaps the screen contents with an off-screen buffer. 
//  //##
//  //## This only works if the widget's format specifies double buffer mode.
//  //## Normally, there is no need to explicitly call this function because it is done automatically 
//  //## after each widget repaint, i.e. each time after paintGL() has been executed. 
//  virtual void SwapBuffers () { swap_buffers(); }
//
//  //##Documentation
//  //## @brief Returns @a true if display list sharing with another window
//  //##  was requested and could be provided.
//  //## 
//  //## For example, a OpenGL system may fail to provide display list sharing 
//  //## if the two windows use different formats.
//  virtual bool IsSharing () const { return false;}
//
//  //##Documentation
//  //## @brief Immediately repaints the contents of the renderwindow
//  //##
//  //## Renderwindow will be updated before repainting. 
//  virtual void paint() { std::cout << "paint() called" << std::endl; }
//  virtual void InitRenderer();
//  virtual void draw();
//  virtual int handle(int event) { return Fl_Gl_Window::handle(event); }
//  virtual ~FLmitkRenderWindow() {};
//  virtual void resize(int x, int y, int w, int h);
//
//protected:
//  virtual void Repaint() { invalidate(); }
//
//  bool m_InitNeeded;
//  bool m_ResizeNeeded;
//
//};
//
//#endif /* FLMITKRENDERWINDOW_H_HEADER_INCLUDED */
//
