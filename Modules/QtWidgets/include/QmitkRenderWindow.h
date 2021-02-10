/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKRENDERWINDOW_H
#define QMITKRENDERWINDOW_H

#include "mitkRenderWindowBase.h"

#include "QmitkRenderWindowMenu.h"
#include <MitkQtWidgetsExports.h>

#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLNativeWidget.h>

#include "mitkBaseRenderer.h"
#include "mitkInteractionEventConst.h"

class QDragEnterEvent;
class QDropEvent;
class QInputEvent;
class QMouseEvent;

/**
 * \ingroup QmitkModule
 * \brief MITK implementation of the QVTKWidget
 */
class MITKQTWIDGETS_EXPORT QmitkRenderWindow : public QVTKOpenGLNativeWidget, public mitk::RenderWindowBase
{
  Q_OBJECT

public:
  QmitkRenderWindow(
    QWidget *parent = nullptr,
    const QString &name = "unnamed renderwindow",
    mitk::VtkPropRenderer *renderer = nullptr);
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
   *
   * The default behaviour of QmitkRenderWindow is now to clear the "accepted" flag
   * of Qt events after they were handled by QVTKWidget. This way parents can also
   * handle events.
   *
   * If you don't want this behaviour, call SetResendQtEvents(true) on your render window.
   */
  virtual void SetResendQtEvents(bool resend);

  // Set Layout Index to define the Layout Type
  void SetLayoutIndex(QmitkRenderWindowMenu::LayoutIndex layoutIndex);

  // Get Layout Index to define the Layout Type
  QmitkRenderWindowMenu::LayoutIndex GetLayoutIndex();

  // MenuWidget need to update the Layout Design List when Layout had changed
  void UpdateLayoutDesignList(QmitkRenderWindowMenu::LayoutDesign layoutDesign);

  void UpdateCrosshairVisibility(bool);

  void UpdateCrosshairRotationMode(int);

  // Activate or Deactivate MenuWidget.
  void ActivateMenuWidget(bool state);

  bool GetActivateMenuWidgetFlag() { return m_MenuWidgetActivated; }
  // Get it from the QVTKWidget parent
  vtkRenderWindow *GetVtkRenderWindow() override { return this->renderWindow(); }

  vtkRenderWindowInteractor *GetVtkRenderWindowInteractor() override { return nullptr; }

protected:

  // catch-all event handler
  bool event(QEvent *e) override;
  // overloaded move handler
  void moveEvent(QMoveEvent *event) override;
  // overloaded show handler
  void showEvent(QShowEvent *event) override;
  // overloaded enter handler
  void enterEvent(QEvent *) override;
  // overloaded leave handler
  void leaveEvent(QEvent *) override;

  // Overloaded resize handler, see decs in QVTKOpenGLWidget.
  // Basically, we have to ensure the VTK rendering is updated for each change in window size.
  void resizeGL(int w, int h) override;

  /// \brief Simply says we accept the event type.
  void dragEnterEvent(QDragEnterEvent *event) override;

  /// \brief If the dropped type is application/x-mitk-datanodes we process the request by converting to mitk::DataNode
  /// pointers and emitting the NodesDropped signal.
  void dropEvent(QDropEvent *event) override;

Q_SIGNALS:

  void LayoutDesignChanged(QmitkRenderWindowMenu::LayoutDesign);

  void ResetView();

  void CrosshairRotationModeChanged(int);

  void CrosshairVisibilityChanged(bool);

  void moved();

  /// \brief Emits a signal to say that this window has had the following nodes dropped on it.
  void NodesDropped(QmitkRenderWindow *thisWindow, std::vector<mitk::DataNode *> nodes);

  void mouseEvent(QMouseEvent *);

private Q_SLOTS:

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

  QmitkRenderWindowMenu::LayoutIndex m_LayoutIndex;

  vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_InternalRenderWindow;

};

#endif // QMITKRENDERWINDOW_H
