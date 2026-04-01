/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindow_h
#define QmitkRenderWindow_h

#include <mitkRenderWindowBase.h>

#include <QmitkRenderWindowMenu.h>
#include <MitkQtWidgetsExports.h>

#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLNativeWidget.h>

#include <mitkBaseRenderer.h>
#include <mitkInteractionEventConst.h>

#include <QmitkButtonOverlayWidget.h>

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

  /**
   * \brief Sets the layout index to define the layout type.
   * \param[in] layoutIndex The layout index to set.
   */
  void SetLayoutIndex(QmitkRenderWindowMenu::LayoutIndex layoutIndex);

  /** \brief Returns the current layout index. */
  QmitkRenderWindowMenu::LayoutIndex GetLayoutIndex();

  /**
   * \brief Updates the layout design list in the menu widget when the layout has changed.
   * \param[in] layoutDesign The new layout design.
   */
  void UpdateLayoutDesignList(QmitkRenderWindowMenu::LayoutDesign layoutDesign);

  /**
   * \brief Updates the crosshair visibility state in the menu widget.
   * \param[in] visible True to show the crosshair, false to hide it.
   */
  void UpdateCrosshairVisibility(bool visible);

  /**
   * \brief Updates the crosshair rotation mode in the menu widget.
   * \param[in] mode The new rotation mode.
   */
  void UpdateCrosshairRotationMode(int mode);

  /**
   * \brief Activates or deactivates the render window menu widget.
   * \param[in] state True to activate the menu, false to deactivate.
   */
  void ActivateMenuWidget(bool state);

  /** \brief Returns whether the menu widget is activated. */
  bool GetActivateMenuWidgetFlag() { return m_MenuWidgetActivated; }

  /**
   * \brief Shows or hides the geometry violation warning overlay.
   * \param[in] show True to show, false to hide.
   */
  void ShowOverlayMessage(bool show);

  /** \brief Returns the underlying VTK render window. */
  vtkRenderWindow *GetVtkRenderWindow() override { return this->renderWindow(); }

  /** \brief Returns nullptr since no interactor is used. */
  vtkRenderWindowInteractor *GetVtkRenderWindowInteractor() override { return nullptr; }

protected:

  // catch-all event handler
  bool event(QEvent *e) override;
  // overloaded move handler
  void moveEvent(QMoveEvent *event) override;
  // overloaded show handler
  void showEvent(QShowEvent *event) override;
  // overloaded enter handler
  void enterEvent(QEnterEvent *) override;
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

  void ResetGeometry();

  void CrosshairRotationModeChanged(int);

  void CrosshairVisibilityChanged(bool);

  void moved();

  /// \brief Emits a signal to say that this window has had the following nodes dropped on it.
  void NodesDropped(QmitkRenderWindow *thisWindow, std::vector<mitk::DataNode *> nodes);

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

  void UpdateStatusBar(mitk::Point2D pointerPositionOnScreen);

  bool m_ResendQtEvents;

  QmitkRenderWindowMenu *m_MenuWidget;

  bool m_MenuWidgetActivated;

  QmitkRenderWindowMenu::LayoutIndex m_LayoutIndex;

  vtkSmartPointer<vtkGenericOpenGLRenderWindow> m_InternalRenderWindow;

  QmitkButtonOverlayWidget* m_GeometryViolationWarningOverlay;

};

#endif
