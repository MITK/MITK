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

#include <QmitkExports.h>

#include "mitkRenderWindowBase.h"

#include "QVTKWidget.h"
#include "QmitkRenderWindowMenu.h"

/**
 * \brief MITK implementation of the QVTKWidget
 * \ingroup Renderer
 */
class QMITK_EXPORT QmitkRenderWindow : public QVTKWidget , public mitk::RenderWindowBase
{
  Q_OBJECT

public:

  QmitkRenderWindow(QWidget *parent = 0, QString name = "unnamed renderwindow", mitk::VtkPropRenderer* renderer = NULL, mitk::RenderingManager* renderingManager = NULL);
  virtual ~QmitkRenderWindow();
 
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

  //MenuWidget need to update the Layout Design List when Layout had changed
  void LayoutDesignListChanged( int layoutDesignIndex );
  
  
  void HideRenderWindowMenu( );

  //Activate or Deactivate MenuWidget. 
  void ActivateMenuWidget( bool state );
  
  bool GetActivateMenuWidgetFlag()
  {  return m_MenuWidgetActivated; }

  // Get it from the QVTKWidget parent
  virtual vtkRenderWindow* GetVtkRenderWindow()
  {  return GetRenderWindow();} 

  virtual vtkRenderWindowInteractor* GetVtkRenderWindowInteractor()
  {  return NULL;}

  void FullScreenMode( bool state );

protected:
    // overloaded move handler
    virtual void moveEvent( QMoveEvent* event );
    // overloaded show handler
    void showEvent( QShowEvent* event );
    // overloaded resize handler
    virtual void resizeEvent(QResizeEvent* event);
    // overloaded paint handler
    virtual void paintEvent(QPaintEvent* event);
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

    void AdjustRenderWindowMenuVisibility( const QPoint& pos );

signals:
  
  void ResetView();
  // \brief int parameters are enum from QmitkStdMultiWidget
  void ChangeCrosshairRotationMode(int); 

  void SignalLayoutDesignChanged( int layoutDesignIndex );

  void moved();

  void resized();

protected slots:  

  void OnChangeLayoutDesign(int layoutDesignIndex);

  void OnWidgetPlaneModeChanged( int );

  void DeferredHideMenu();

private:
  
  bool                           m_ResendQtEvents;

  QmitkRenderWindowMenu*         m_MenuWidget;

  bool                           m_MenuWidgetActivated;
  
  unsigned int                   m_LayoutIndex;
  
};

#endif
