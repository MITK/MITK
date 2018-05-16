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

#ifndef QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66
#define QMITKRENDERWINDOW_H_HEADER_INCLUDED_C1C40D66

#include "mitkRenderWindowBase.h"

#include "QmitkRenderWindowMenu.h"
#include <MitkQtWidgetsExports.h>

#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLWidget.h>

#include "mitkBaseRenderer.h"
#include "mitkInteractionEventConst.h"

class QmitkStdMultiWidget;
class QDragEnterEvent;
class QDropEvent;
class QInputEvent;

/**
 * \ingroup QmitkModule
 * \brief MITK implementation of the QVTKWidget
 */
class MITKQTWIDGETS_EXPORT QmitkRenderWindow : public QVTKOpenGLWidget, public mitk::RenderWindowBase
{
  Q_OBJECT

public:
  QmitkRenderWindow(
    QWidget *parent = nullptr,
    const QString &name = "unnamed renderwindow",
    mitk::VtkPropRenderer *renderer = nullptr,
    mitk::RenderingManager *renderingManager = nullptr,
    mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard);
  ~QmitkRenderWindow() override;

  /**
   * \brief Whether Qt events should be passed to parent (default: true)
   *
   * With introduction of the QVTKWidget the behaviour regarding Qt events changed.
   * QVTKWidget "accepts" Qt events like mouse clicks (i.e. set an "accepted" flag).
   * When this flag is set, Qt fininshed handling of this event -- otherwise it is
   * reached through to the widget's parent.
   *
   * This reaching through to the parent was implicitly required by QmitkMaterialWidget / QmitkMaterialShowCase.
   *QmitkStdMultiWidget
   * The default behaviour of QmitkRenderWindow is now to clear the "accepted" flag
   * of Qt events after they were handled by QVTKWidget. This way parents can also
   * handle events.
   *
   * If you don't want this behaviour, call SetResendQtEvents(true) on your render window.
   */
  virtual void SetResendQtEvents(bool resend);

  // Set Layout Index to define the Layout Type
  void SetLayoutIndex(unsigned int layoutIndex);

  // Get Layout Index to define the Layout Type
  unsigned int GetLayoutIndex();

  // MenuWidget need to update the Layout Design List when Layout had changed
  void LayoutDesignListChanged(int layoutDesignIndex);

  void HideRenderWindowMenu();

  // Activate or Deactivate MenuWidget.
  void ActivateMenuWidget(bool state, QmitkStdMultiWidget *stdMultiWidget = nullptr);

  bool GetActivateMenuWidgetFlag() { return m_MenuWidgetActivated; }
  // Get it from the QVTKWidget parent
  vtkRenderWindow *GetVtkRenderWindow() override { return GetRenderWindow(); }
  vtkRenderWindowInteractor *GetVtkRenderWindowInteractor() override { return nullptr; }
  void FullScreenMode(bool state);

protected:
  // overloaded move handler
  void moveEvent(QMoveEvent *event) override;
  // overloaded show handler
  void showEvent(QShowEvent *event) override;
  // overloaded mouse press handler
  void mousePressEvent(QMouseEvent *event) override;
  // overloaded mouse double-click handler
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  // overloaded mouse move handler
  void mouseMoveEvent(QMouseEvent *event) override;
  // overloaded mouse release handler
  void mouseReleaseEvent(QMouseEvent *event) override;
  // overloaded key press handler
  void keyPressEvent(QKeyEvent *event) override;
  // overloaded enter handler
  void enterEvent(QEvent *) override;
  // overloaded leave handler
  void leaveEvent(QEvent *) override;

  // Overloaded resize handler, see decs in QVTKOpenGLWidget.
  // Basically, we have to ensure the VTK rendering is updated for each change in window size.
  void resizeGL(int w, int h) Q_DECL_OVERRIDE;

  /// \brief Simply says we accept the event type.
  void dragEnterEvent(QDragEnterEvent *event) override;

  /// \brief If the dropped type is application/x-mitk-datanodes we process the request by converting to mitk::DataNode
  /// pointers and emitting the NodesDropped signal.
  void dropEvent(QDropEvent *event) override;

#ifndef QT_NO_WHEELEVENT
  // overload wheel mouse event
  void wheelEvent(QWheelEvent *) override;
#endif

  void AdjustRenderWindowMenuVisibility(const QPoint &pos);

signals:

  void ResetView();
  // \brief int parameters are enum from QmitkStdMultiWidget
  void ChangeCrosshairRotationMode(int);

  void SignalLayoutDesignChanged(int layoutDesignIndex);

  void moved();

  /// \brief Emits a signal to say that this window has had the following nodes dropped on it.
  void NodesDropped(QmitkRenderWindow *thisWindow, std::vector<mitk::DataNode *> nodes);

protected slots:

  void OnChangeLayoutDesign(int layoutDesignIndex);

  void OnWidgetPlaneModeChanged(int);

  void DeferredHideMenu();

private:
  // Helper Functions to Convert Qt-Events to Mitk-Events

  mitk::Point2D GetMousePosition(QMouseEvent *me) const;
  mitk::Point2D GetMousePosition(QWheelEvent *we) const;
  mitk::InteractionEvent::MouseButtons GetEventButton(QMouseEvent *me) const;
  mitk::InteractionEvent::MouseButtons GetButtonState(QMouseEvent *me) const;
  mitk::InteractionEvent::ModifierKeys GetModifiers(QInputEvent *me) const;
  mitk::InteractionEvent::MouseButtons GetButtonState(QWheelEvent *we) const;
  std::string GetKeyLetter(QKeyEvent *ke) const;
  int GetDelta(QWheelEvent *we) const;
  bool m_ResendQtEvents;

  QmitkRenderWindowMenu *m_MenuWidget;

  bool m_MenuWidgetActivated;

  unsigned int m_LayoutIndex;

  vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_InternalRenderWindow;
};

#endif
