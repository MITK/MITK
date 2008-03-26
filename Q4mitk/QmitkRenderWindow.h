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


#ifndef QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66
#define QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66

//#include <qgl.h>
#include "mitkCommon.h"

//#include "qlayout.h"
#include "QVTKWidget.h"

#include "mitkVtkPropRenderer.h"
#include "vtkMitkRenderProp.h"
#include "mitkSliceNavigationController.h"
#include "mitkCameraRotationController.h"

/**
 * \brief MITK implementation of the QVTKWidget
 * \ingroup Renderer
 */
class QMITK_EXPORT QmitkRenderWindow : public QVTKWidget //, public mitk::RenderWindow
{
public:
 
  QmitkRenderWindow(QWidget *parent = 0, QString name = "unnamed renderwindow", mitk::VtkPropRenderer* renderer = NULL);
  virtual ~QmitkRenderWindow();

  void InitRenderer();
  
  virtual mitk::SliceNavigationController * GetSliceNavigationController(); 
  virtual mitk::CameraRotationController * GetCameraRotationController();
  virtual mitk::BaseController * GetController();
  virtual mitk::VtkPropRenderer* GetRenderer() { return m_Renderer; }
  
protected:
    
    // overloaded resize handler
    virtual void resizeEvent(QResizeEvent* event);

    // overloaded mouse press handler
    virtual void mousePressEvent(QMouseEvent* event);
    // overloaded mouse move handler
    virtual void mouseMoveEvent(QMouseEvent* event);
    // overloaded mouse release handler
    virtual void mouseReleaseEvent(QMouseEvent* event);
    // overloaded key press handler
    virtual void keyPressEvent(QKeyEvent* event);
#ifndef QT_NO_WHEELEVENT
    // overload wheel mouse event
    virtual void wheelEvent(QWheelEvent*);
#endif


private:
  mitk::VtkPropRenderer::Pointer m_Renderer;
  vtkMitkRenderProp*             m_RenderProp;

  bool                           m_InResize;

 

};

#endif /* QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66 */
