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

#ifndef QMITKMXNMULTIWIDGET_H
#define QMITKMXNMULTIWIDGET_H

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "QmitkAbstractMultiWidget.h"

class QGridLayout;

namespace mitk
{
  class RenderingManager;
}

/**
* @brief The 'QmitkMxNMultiWidget' is a 'QmitkAbstractMultiWidget' that is used to display multiple render windows at once.
*        Render windows can dynamically be added and removed to change the layout of the multi widget. This
*        is done by using the 'ResetLayout'-function to define a layout. This will automatically add or remove
*        the appropriate number of render window widgets.
*/
class MITKQTWIDGETS_EXPORT QmitkMxNMultiWidget : public QmitkAbstractMultiWidget
{
  Q_OBJECT

public:

  QmitkMxNMultiWidget(QWidget* parent = 0,
                      Qt::WindowFlags f = 0,
                      mitk::RenderingManager* renderingManager = nullptr,
                      mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
                      const QString& multiWidgetName = "mxnmulti");

  virtual ~QmitkMxNMultiWidget() = default;

  virtual void InitializeMultiWidget() override;
  virtual void MultiWidgetOpened() override;
  virtual void MultiWidgetClosed() override;

  virtual void SetActiveRenderWindowWidget(RenderWindowWidgetPointer activeRenderWindowWidget) override;

  virtual void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) override;
  virtual const mitk::Point3D GetSelectedPosition(const QString& widgetName) const override;

  void ActivateAllCrosshairs(bool activate);

public Q_SLOTS:

  // mouse events
  virtual void wheelEvent(QWheelEvent* e) override;
  virtual void mousePressEvent(QMouseEvent* e) override;
  virtual void moveEvent(QMoveEvent* e) override;

Q_SIGNALS:

  void WheelMoved(QWheelEvent *);
  void Moved();

private:

  virtual void SetLayoutImpl() override;
  virtual void SynchronizeImpl() override { }
  virtual void SetInteractionSchemeImpl() override { }

  void InitializeLayout();
  void CreateRenderWindowWidget();
  void FillMultiWidgetLayout();

  QGridLayout* m_GridLayout;

};

#endif // QMITKMXNMULTIWIDGET_H
