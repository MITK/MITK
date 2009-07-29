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

#include "QmitkRenderWindowMenu.h"

/**
 * \brief MITK implementation of the QVTKWidget
 * \ingroup Renderer
 */
class QMITK_EXPORT QmitkRenderWindow : public QVTKWidget //, public mitk::RenderWindow
{
  Q_OBJECT

public:

  QmitkRenderWindow(QWidget *parent = 0, QString name = "unnamed renderwindow", mitk::VtkPropRenderer* renderer = NULL);
  virtual ~QmitkRenderWindow();

  void InitRenderer();

  virtual mitk::SliceNavigationController * GetSliceNavigationController();
  virtual mitk::CameraRotationController * GetCameraRotationController();
  virtual mitk::BaseController * GetController();
  virtual mitk::VtkPropRenderer* GetRenderer();

  /**
   * \brief Whether Qt events should be passed to parent (default: true)
   *
   * With introduction of the QVTKWidget the behaviour regarding Qt events changed.
   * QVTKWidget "accepts" Qt events like mouse clicks (i.e. set an "accepted" flag).
   * When this flag is set, Qt fininshed handling of this event -- otherwise it is
   * reached through to the widget's parent.
   *
   * This reaching through to the parent was implicitly required by QmitkMaterialWidget / QmitkMaterialShowCase.
   *
   * The default behaviour of QmitkRenderWindow is now to clear the "accepted" flag
   * of Qt events after they were handled by QVTKWidget. This way parents can also
   * handle events.
   *
   * If you don't want this behaviour, call SetResendQtEvents(true) on your render window.
   */
  virtual void SetResendQtEvents(bool resend);

  // Set Layout Index to define the Layout Type
  void SetLayoutIndex( unsigned int layoutIndex );
    
  // Get Layout Index to define the Layout Type
  unsigned int GetLayoutIndex();

  void LayoutDesignListChanged( int layoutDesignIndex );
  
  void HideMenuWidget();
  
  void SetMenuWidgetActivated( bool state )
  {  m_MenuWidgetActivated = state; }

  bool GetMenuWidgetActivated()
  {  return m_MenuWidgetActivated; }

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

    // overloaded enter handler
    virtual void enterEvent(QEvent*);
    // overloaded leave handler
    virtual void leaveEvent(QEvent*);

#ifndef QT_NO_WHEELEVENT
    // overload wheel mouse event
    virtual void wheelEvent(QWheelEvent*);
#endif

signals:
  
  void ShowCrosshair(bool show);
  void ResetView();
  // \brief int parameters are enum from QmitkStdMultiWidget
  void ChangeCrosshairRotationMode(int); 
  void SetCrosshairRotationLinked(bool);
  void SignalLayoutDesignChanged( int layoutDesignIndex );

protected slots:  

  void OnChangeLayoutDesign(int layoutDesignIndex);

private:
  
  mitk::VtkPropRenderer::Pointer m_Renderer;

  vtkMitkRenderProp*             m_RenderProp;

  bool                           m_InResize;

  bool                           m_ResendQtEvents;

  QmitkRenderWindowMenu*         m_MenuWidget;

  bool                           m_MenuWidgetActivated;
};

#endif /* QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66 */
