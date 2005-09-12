/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
void QmitkSelectableGLWidget::init()
{
    QLayout *hlayout;
    hlayout=layout();
    hlayout->setMargin(2);
    hlayout->setAutoAdd(true);

    QString rendererName("Renderer::");
    rendererName += name();

    // create Renderer
    renderer= new mitk::OpenGLRenderer( rendererName ); 
    
    // create widget
    QString composedName("QSGLWt::");
    if(name()!=NULL)
        composedName+=name();
    else
        composedName+="QmitkGLWidget";
    /*
    * here is the place to define QT-Flags to enable and disable certain OpenGL elements, like StencilBuffer, RGBA and so on.
    * See QGLFormat for futher informations
    * QGL::AlphaChannel: Enable Alpha in Framebuffer
    * QGL::Rgba enable use of rgba rather than color_index
    * QGL::StencilBuffer for use of stencilbuffer in OpenGL
    */
    mitkWidget = new QmitkRenderWindow(renderer,QGLFormat( /*QGL::Rgba |*/ QGL::AlphaChannel ), this, composedName);
    
    
    //initialize SliceNavigationController
    sliceNavigationController = new mitk::SliceNavigationController("navigation");
    sliceNavigationController->ConnectGeometrySliceEvent(renderer.GetPointer());
    sliceNavigationController->ConnectGeometryTimeEvent(renderer.GetPointer(), false);
}


mitk::OpenGLRenderer* QmitkSelectableGLWidget::GetRenderer()
{
    return renderer.GetPointer();
}



QmitkRenderWindow* QmitkSelectableGLWidget::GetRenderWindow() const
{
    return mitkWidget;
}


mitk::SliceNavigationController* QmitkSelectableGLWidget::GetSliceNavigationController()
{
    return sliceNavigationController.GetPointer();
}


void QmitkSelectableGLWidget::wheelEvent( QWheelEvent * e )
{
  mitk::Stepper* stepper = sliceNavigationController->GetSlice();
  if(stepper->GetSteps() <= 1)
    stepper = sliceNavigationController->GetTime();
  if (e->orientation() * e->delta()  > 0) 
    stepper->Next();
  else
    stepper->Previous();
}
