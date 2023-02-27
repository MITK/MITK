/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStdMultiWidget_h
#define QmitkStdMultiWidget_h

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkAbstractMultiWidget.h"

/**
* @brief The 'QmitkStdMultiWidget' is a 'QmitkAbstractMultiWidget' that is used to display multiple render windows at once.
*        Render windows are predefined in a 2x2 design with 3 different 2D view planes and a 3D render window.
*/
class MITKQTWIDGETS_EXPORT QmitkStdMultiWidget : public QmitkAbstractMultiWidget
{
  Q_OBJECT

public:
  QmitkStdMultiWidget(
    QWidget *parent = nullptr,
    Qt::WindowFlags f = nullptr,
    const QString &name = "stdmulti");

  ~QmitkStdMultiWidget() override;

  virtual void InitializeMultiWidget() override;

  virtual QmitkRenderWindow* GetRenderWindow(const QString& widgetName) const override;
  virtual QmitkRenderWindow* GetRenderWindow(const mitk::AnatomicalPlane& orientation) const override;

  /**
  * @brief Initialize all render windows of the StdMultiWidget to the given geometry.
  *        Overridem from 'QmitkAbstractMultiWidget'.
  *
  * @param geometry       The geometry to be used to initialize / update all
  *                       render window's time and slice navigation controller.
  * @param resetCamera    If true, the camera and crosshair will be reset to the default view (centered, no zoom).
  *                       If false, the current crosshair position and the camera zoom will be stored and reset
  *                       after the reference geometry has been updated.
  */
  void InitializeViews(const mitk::TimeGeometry* geometry, bool resetCamera) override;

  /**
  * @brief Not implemented in this class.
  *        Overridem from 'QmitkAbstractMultiWidget'.
  */
  void SetInteractionReferenceGeometry(const mitk::TimeGeometry* referenceGeometry) override;

  /**
  * @brief Returns true if the render windows are coupled; false if not.
  *
  * For the StdMultiWidget the render windows are typically coupled.
  */
  bool HasCoupledRenderWindows() const override;

  virtual void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) override;
  virtual const mitk::Point3D GetSelectedPosition(const QString& widgetName) const override;

  virtual void SetCrosshairVisibility(bool) override;
  virtual bool GetCrosshairVisibility() const override;
  void SetCrosshairGap(unsigned int gapSize) override;

  virtual void ResetCrosshair() override;

  virtual void SetWidgetPlaneMode(int mode) override;

  mitk::SliceNavigationController* GetTimeNavigationController();

  void AddPlanesToDataStorage();
  void RemovePlanesFromDataStorage();

  /**
   * @brief Convenience method to get a render window widget.
   * @param   number of the widget (0-3)
   * @return  The render window widget
   */
  QmitkRenderWindow* GetRenderWindow(unsigned int number) const;
  QmitkRenderWindow* GetRenderWindow1() const;
  QmitkRenderWindow* GetRenderWindow2() const;
  QmitkRenderWindow* GetRenderWindow3() const;
  QmitkRenderWindow* GetRenderWindow4() const;

  /**
  * @brief Convenience method to get a widget plane.
  * @param   number of the widget plane (1-3)
  * @return  The widget plane as data node
  */
  mitk::DataNode::Pointer GetWidgetPlane(unsigned int number) const;
  mitk::DataNode::Pointer GetWidgetPlane1() const;
  mitk::DataNode::Pointer GetWidgetPlane2() const;
  mitk::DataNode::Pointer GetWidgetPlane3() const;

  /**
  * @brief SetDecorationColor Set the color of the decoration of the 4 widgets.
  *
  * This is used to color the frame of the renderwindow and the corner annatation.
  * For the first 3 widgets, this color is a property of the helper object nodes
  * which contain the respective plane geometry. For widget 4, this is a member,
  * since there is no data node for this widget.
  */
  void SetDecorationColor(unsigned int widgetNumber, mitk::Color color);
  /**
   * @brief GetDecorationColorForWidget Get the color for annotation, crosshair and rectangle.
   * @param widgetNumber Number of the renderwindow (0-3).
   * @return Color in mitk format.
   */
  mitk::Color GetDecorationColor(unsigned int widgetNumber);

public Q_SLOTS:

  // mouse events
  virtual void mousePressEvent(QMouseEvent*) override;
  virtual void moveEvent(QMoveEvent* e) override;
  virtual void wheelEvent(QWheelEvent* e) override;

  void Fit();

  void AddDisplayPlaneSubTree();

  void EnsureDisplayContainsPoint(mitk::BaseRenderer *renderer, const mitk::Point3D &p);

  void SetWidgetPlaneVisibility(const char *widgetName, bool visible, mitk::BaseRenderer *renderer = nullptr);

  void SetWidgetPlanesVisibility(bool visible, mitk::BaseRenderer *renderer = nullptr);

Q_SIGNALS:

  void NotifyCrosshairVisibilityChanged(bool visible);
  void NotifyCrosshairRotationModeChanged(int mode);

  void WheelMoved(QWheelEvent *);
  void Moved();

private:

  virtual void SetLayoutImpl() override;
  virtual void SetInteractionSchemeImpl() override { }

  void CreateRenderWindowWidgets();

  mitk::SliceNavigationController* m_TimeNavigationController;

  /**
  * @brief The 3 helper objects which contain the plane geometry.
  */
  mitk::DataNode::Pointer m_PlaneNode1;
  mitk::DataNode::Pointer m_PlaneNode2;
  mitk::DataNode::Pointer m_PlaneNode3;

  /**
   * @brief m_ParentNodeForGeometryPlanes This helper object is added to the datastorage
   * and contains the 3 planes for displaying the image geometry (crosshair and 3D planes).
   */
  mitk::DataNode::Pointer m_ParentNodeForGeometryPlanes;

  /**
   * @brief m_DecorationColorWidget4 color for annotation and rectangle of widget 4.
   *
   * For other widgets1-3, the color is a property of the respective data node.
   * There is no node for widget 4, hence, we need an extra member.
   */
  mitk::Color m_DecorationColorWidget4;

};

#endif
