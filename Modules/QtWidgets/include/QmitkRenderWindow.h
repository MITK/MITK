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

#include <MitkQtWidgetsExports.h>
#include "QVTKWidget.h"
#include <vtkGenericOpenGLRenderWindow.h>
#include "QmitkRenderWindowMenu.h"

#include "mitkInteractionEventConst.h"
#include "mitkBaseRenderer.h"

class QmitkStdMultiWidget;
class QDragEnterEvent;
class QDropEvent;
class QInputEvent;
class QTextBrowser;

namespace mitk {
  class StructuredReport;
}

/**
 * \ingroup QmitkModule
 * \brief MITK implementation of the QVTKWidget
 */
class MITKQTWIDGETS_EXPORT QmitkRenderWindow: public QVTKWidget, public mitk::RenderWindowBase
{
  Q_OBJECT

public:

  QmitkRenderWindow(QWidget *parent = 0, QString name = "unnamed renderwindow", mitk::VtkPropRenderer* renderer = NULL,
                    mitk::RenderingManager* renderingManager = NULL,mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard, bool useFXAA = true);
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

  //MenuWidget need to update the Layout Design List when Layout had changed
  void LayoutDesignListChanged(int layoutDesignIndex);

  void HideRenderWindowMenu();

  void UpdateStructuredReportVisibility(mitk::StructuredReport* report, bool visible);

  //Activate or Deactivate MenuWidget.
  void ActivateMenuWidget(bool state, QmitkStdMultiWidget* stdMultiWidget = 0);

  bool GetActivateMenuWidgetFlag()
  {
    return m_MenuWidgetActivated;
  }

  // Get it from the QVTKWidget parent
  virtual vtkRenderWindow* GetVtkRenderWindow() override
  {
    return GetRenderWindow();
  }

  virtual vtkRenderWindowInteractor* GetVtkRenderWindowInteractor() override
  {
    return NULL;
  }

  void FullScreenMode(bool state);
  bool GetFullSreenMode();

  bool isWindow3d();

  bool windowPlaneIsSelected();
  void setWindowPlaneIsSelected(bool selected);

  void setDropMode(bool state);
  bool getDropMode();


  void setTwoDimensionalViewState(bool state);
  bool isTwoDimensionalViewState();

  void ConnectStructuredReportWidgetToDataStorage(mitk::DataStorage* ds);

  void updateAllWindows();

  void emit_annotationPositionChanged() { emit annotationPositionChanged(this); };

protected:
  // overloaded move handler
  virtual void moveEvent(QMoveEvent* event) override;
  // overloaded show handler
  void showEvent(QShowEvent* event) override;
  // overloaded paint handler
  virtual void paintEvent(QPaintEvent* event) override;
  // overloaded mouse press handler
  virtual void mousePressEvent(QMouseEvent* event) override;
  // overloaded mouse double-click handler
  virtual void mouseDoubleClickEvent( QMouseEvent *event ) override;
  // overloaded mouse move handler
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  // overloaded mouse release handler
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  // overloaded key press handler
  virtual void keyPressEvent(QKeyEvent* event) override;
  // overloaded enter handler
  virtual void enterEvent(QEvent*) override;
  // overloaded leave handler
  virtual void leaveEvent(QEvent*) override;

  /// \brief Simply says we accept the event type.
  virtual void dragEnterEvent(QDragEnterEvent *event) override;

  /// \brief If the dropped type is application/x-mitk-datanodes we process the request by converting to mitk::DataNode pointers and emitting the NodesDropped signal.
  virtual void dropEvent(QDropEvent * event) override;

#ifndef QT_NO_WHEELEVENT
  // overload wheel mouse event
  virtual void wheelEvent(QWheelEvent*) override;
#endif

  void AdjustRenderWindowMenuVisibility(const QPoint& pos);

  QWidget* m_StructuredReportWidget;
  QTextBrowser* m_StructuredReportText;
  std::set<mitk::StructuredReport*> m_VisibleReports;

  void initStructuredReportWidget();
  void freeStructuredReportWidget();
  void fillStructuredReportWidget(mitk::StructuredReport* report);

  void AddNodeCallback(const mitk::DataNode* node);
  void RemoveNodeCallback(const mitk::DataNode* node);
signals:

  // \brief int parameters are enum from QmitkStdMultiWidget
  void ChangeCrosshairRotationMode(int);

  void SignalLayoutDesignChanged(int layoutDesignIndex);

  void moved();
  
  void resized();

  /// \brief Emits a signal to say that this window has had the following nodes dropped on it.
  void NodesDropped(QmitkRenderWindow *thisWindow, std::vector<mitk::DataNode*> nodes);

  void windowPlaneSelectionChanged();

  void annotationPositionChanged(QmitkRenderWindow* renderWindow);

protected slots:

  void OnChangeLayoutDesign(int layoutDesignIndex);

  void OnWidgetPlaneModeChanged(int);

  void DeferredHideMenu();

  void OnFullScreenModeChanged(bool fullscreen);
  void resizeEvent(QResizeEvent* e) override;

private:

  // Helper Functions to Convert Qt-Events to Mitk-Events

  mitk::Point2D GetMousePosition(QMouseEvent* me) const;
  mitk::Point2D GetMousePosition(QWheelEvent* we) const;
  mitk::InteractionEvent::MouseButtons GetEventButton(QMouseEvent* me) const;
  mitk::InteractionEvent::MouseButtons GetButtonState(QMouseEvent* me) const;
  mitk::InteractionEvent::ModifierKeys GetModifiers(QInputEvent* me) const;
  mitk::InteractionEvent::MouseButtons GetButtonState(QWheelEvent* we) const;
  std::string GetKeyLetter(QKeyEvent* ke) const;
  int GetDelta(QWheelEvent* we) const;
  bool m_ResendQtEvents;

  QmitkRenderWindowMenu* m_MenuWidget;

  bool m_MenuWidgetActivated;
  bool m_WindowPlaneIsSelected;

  unsigned int m_LayoutIndex;
  bool m_FullScreenMode;

  mitk::Point2D m_CrosshairPosition;

  bool m_DropEnabled;
  bool m_IsTwoDimensionalView;
};

#endif
