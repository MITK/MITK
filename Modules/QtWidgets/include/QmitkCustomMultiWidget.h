/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKCUSTOMMULTIWIDGET_H
#define QMITKCUSTOMMULTIWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkRenderWindowWidget.h"

// mitk core
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkMouseModeSwitcher.h>

// qt
#include <QFrame>
#include <qsplitter.h>
#include <qwidget.h>

class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;
class QmitkLevelWindowWidget;
class QmitkRenderWindow;
class vtkCornerAnnotation;

namespace mitk
{
  class RenderingManager;
}

/*
* @brief The 'QmitkCustomMultiWidget' is a 'QWidget' that is used to display multiple render windows at once.
*
*     Render windows can dynamically be added and removed to change the layout of the multi widget.
*/
class MITKQTWIDGETS_EXPORT QmitkCustomMultiWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkCustomMultiWidget(QWidget *parent = 0, Qt::WindowFlags f = 0, mitk::RenderingManager *renderingManager = 0, mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard, const QString& name = "custommulti");
  virtual ~QmitkCustomMultiWidget();

  void SetDataStorage(mitk::DataStorage* dataStorage);

  mitk::MouseModeSwitcher* GetMouseModeSwitcher();

  std::map<std::string, QmitkRenderWindowWidget*> GetRenderWindowWidgets() const;
  QmitkRenderWindowWidget* GetRenderWindowWidget(unsigned int id) const;
  QmitkRenderWindowWidget* GetActiveRenderWindowWidget() const;
  QmitkRenderWindowWidget* GetFirstRenderWindowWidget() const;
  QmitkRenderWindowWidget* GetLastRenderWindowWidget() const;
  
  unsigned int GetNumberOfRenderWindowWidgets() const;

  const mitk::Point3D GetCrossPosition() const;

  void RequestUpdate(unsigned int id);
  void RequestUpdateAll();
  void ForceImmediateUpdate(unsigned int id);
  void ForceImmediateUpdateAll();

  // color and style settings

  /** \brief Listener to the CrosshairPositionEvent

    Ensures the CrosshairPositionEvent is handled only once and at the end of the Qt-Event loop
  */
  void HandleCrosshairPositionEvent();

protected:

  mitk::DataNode::Pointer GetTopLayerNode(mitk::DataStorage::SetOfObjects::ConstPointer nodes);

public slots:

  void ShowLevelWindowWidget(unsigned int widgetNumber, bool show);
  void ShowAllLevelWindowWidgets(bool show);

  /*
  * @brief Set a background color gradient for a specific render window.
  *
  *   If two different input colors are used, a gradient background is generated.
  *
  * @param upper          The color of the gradient background.
  * @param lower          The color of the gradient background.
  * @param widgetNumber   The widget identifier.
  */
  void SetBackgroundColorGradient(const mitk::Color& upper, const mitk::Color& lower, unsigned int widgetNumber);
  /*
  * @brief Set a background color gradient for all available render windows.
  *
  *   If two different input colors are used, a gradient background is generated.
  *
  * @param upper          The color of the gradient background.
  * @param lower          The color of the gradient background.
  */
  // #TODO: 'backgroundgradientcolor'
  void SetAllBackgroundColorGradients(const mitk::Color& upper, const mitk::Color& lower);
  void FillAllBackgroundColorGradientsWithBlack();
  void ShowBackgroundColorGradient(unsigned int widgetNumber, bool show);
  void ShowAllBackgroundColorGradients(bool show);
  /*
  * @rief Return a render window (widget) specific background color gradient
  *
  * @param widgetNumber   The widget identifier.
  *
  * @return               A color gradient as a pair of colors.
  *                       First entry: upper color value
  *                       Second entry: lower color value
  */
  std::pair<mitk::Color, mitk::Color> GetBackgroundColorGradient(unsigned int widgetNumber) const;
  bool GetBackgroundColorGradientFlag(unsigned int widgetNumber) const;

  void SetDepartmentLogoPath(const char* path);
  void ShowDepartmentLogo(unsigned int widgetNumber, bool show);
  void ShowAllDepartmentLogos(bool show);

  void SetDecorationColor(unsigned int widgetNumber, const mitk::Color& color);
  mitk::Color GetDecorationColor(unsigned int widgetNumber) const;

  void ShowColoredRectangle(unsigned int widgetNumber, bool show);
  void ShowAllColoredRectangles(bool show);
  bool IsColoredRectangleVisible(unsigned int widgetNumber) const;

  void ShowCornerAnnotation(unsigned int widgetNumber, bool show);
  void ShowAllCornerAnnotations(bool show);
  bool IsCornerAnnotationVisible(unsigned int widgetNumber) const;
  void SetCornerAnnotationText(unsigned int widgetNumber, const std::string& cornerAnnotation);
  std::string GetCornerAnnotationText(unsigned int widgetNumber) const;

  void ActivateRenderWindowMenu(unsigned int widgetNumber, bool state);
  void ActivateAllRenderWindowMenus(bool state);
  bool IsRenderWindowMenuActivated(unsigned int widgetNumber) const;

  void ShowGeometryPlanes(unsigned int widgetNumber, bool state);
  void ShowAllGeometryPlanes(bool state);
  mitk::DataNode::Pointer GetGeometryPlane(unsigned int widgetNumber, unsigned int planeNumber) const;




  /// Receives the signal from HandleCrosshairPositionEvent, executes the StatusBar update
  void HandleCrosshairPositionEventDelayed();

  void Fit();

  void wheelEvent(QWheelEvent *e) override;

  void mousePressEvent(QMouseEvent *e) override;

  void moveEvent(QMoveEvent *e) override;

  void EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D &p);

  void MoveCrossToPosition(const mitk::Point3D &newPosition);

  void SetWidgetPlanesLocked(bool locked);

  void SetWidgetPlanesRotationLocked(bool locked);

  void SetWidgetPlanesRotationLinked(bool link);

  void SetWidgetPlaneMode(int mode);

  void SetWidgetPlaneModeToSlicing(bool activate);

  void SetWidgetPlaneModeToRotation(bool activate);

  void SetWidgetPlaneModeToSwivel(bool activate);

  void ResetCrosshair();

signals:

  void LeftMouseClicked(mitk::Point3D pointValue);
  void WheelMoved(QWheelEvent *);
  void WidgetPlanesRotationLinked(bool);
  void WidgetPlanesRotationEnabled(bool);
  void ViewsInitialized();
  void WidgetPlaneModeSlicing(bool);
  void WidgetPlaneModeRotation(bool);
  void WidgetPlaneModeSwivel(bool);
  void WidgetPlaneModeChange(int);
  void WidgetNotifyNewCrossHairMode(int);
  void Moved();

public:

  enum
  {
    PLANE_MODE_SLICING = 0,
    PLANE_MODE_ROTATION,
    PLANE_MODE_SWIVEL
  };

  enum
  {
    AXIAL,
    SAGITTAL,
    CORONAL,
    THREE_D
  };

private:

  void InitializeGUI();
  void AddRenderWindowWidget();
  void InitializeWidget();

  QGridLayout* m_CustomMultiWidgetLayout;
  std::map<std::string, QmitkRenderWindowWidget*> m_RenderWindowWidgets;

  QmitkRenderWindowWidget* m_ActiveRenderWindowWidget;

  int m_PlaneMode;

  mitk::RenderingManager* m_RenderingManager;
  mitk::BaseRenderer::RenderingMode::Type m_RenderingMode;
  QString m_MultiWidgetName;

  mitk::MouseModeSwitcher::Pointer m_MouseModeSwitcher;
  mitk::SliceNavigationController *m_TimeNavigationController;

  mitk::DataStorage::Pointer m_DataStorage;

  bool m_PendingCrosshairPositionEvent;
  bool m_CrosshairNavigationEnabled;

};

#endif // QMITKCUSTOMMULTIWIDGET_H
