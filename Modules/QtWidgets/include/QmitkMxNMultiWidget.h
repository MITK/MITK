/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMXNMULTIWIDGET_H
#define QMITKMXNMULTIWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkAbstractMultiWidget.h"

/**
* @brief The 'QmitkMxNMultiWidget' is a 'QmitkAbstractMultiWidget' that is used to display multiple render windows at once.
*        Render windows can dynamically be added and removed to change the layout of the multi widget. This
*        is done by using the 'SetLayout'-function to define a layout. This will automatically add or remove
*        the appropriate number of render window widgets.
*/
class MITKQTWIDGETS_EXPORT QmitkMxNMultiWidget : public QmitkAbstractMultiWidget
{
  Q_OBJECT

public:

  QmitkMxNMultiWidget(QWidget* parent = nullptr,
                      Qt::WindowFlags f = 0,
                      const QString& multiWidgetName = "mxnmulti");

  ~QmitkMxNMultiWidget();

  void InitializeMultiWidget() override;

  void Synchronize(bool synchronized) override;

  QmitkRenderWindow* GetRenderWindow(const QString& widgetName) const override;
  QmitkRenderWindow* GetRenderWindow(const mitk::AnatomicalPlane& orientation) const override;

  void SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget) override;
  /**
   * @brief Set the reference geometry for interaction inside the active render windows of the MxNMultiWidget.
   *
   * @param referenceGeometry   The reference geometry which is used for updating the
   *                            time geometry inside the active render window.
   * @param resetCamera         If true, the camera and crosshair will be reset to the default view (centered, no zoom).
   *                            If false, the current crosshair position and the camera zoom will be stored and reset
   *                            after the reference geometry has been updated.
   */
  void SetReferenceGeometry(const mitk::TimeGeometry* referenceGeometry, bool resetCamera) override;

  /**
  * @brief Returns true if the render windows are coupled; false if not.
  *
  * For the MxNMultiWidget the render windows are typically decoupled.
  */
  bool HasCoupledRenderWindows() const override;

  void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) override;
  const mitk::Point3D GetSelectedPosition(const QString& widgetName) const override;

  void SetCrosshairVisibility(bool visible) override;
  bool GetCrosshairVisibility() const override;
  void SetCrosshairGap(unsigned int gapSize) override;

  void ResetCrosshair() override;

  void SetWidgetPlaneMode(int userMode) override;

  mitk::SliceNavigationController* GetTimeNavigationController();

  void AddPlanesToDataStorage();
  void RemovePlanesFromDataStorage();

public Q_SLOTS:

  // mouse events
  void wheelEvent(QWheelEvent* e) override;
  void mousePressEvent(QMouseEvent* e) override;
  void moveEvent(QMoveEvent* e) override;

Q_SIGNALS:

  void WheelMoved(QWheelEvent *);
  void Moved();

protected:

  void RemoveRenderWindowWidget() override;

private:

  void SetLayoutImpl() override;
  void SetInteractionSchemeImpl() override { }

  void CreateRenderWindowWidget();

  mitk::SliceNavigationController* m_TimeNavigationController;

  bool m_CrosshairVisibility;

};

#endif // QMITKMXNMULTIWIDGET_H
