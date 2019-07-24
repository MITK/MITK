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

// custom multi widget editor
#include <org_mitk_gui_qt_custommultiwidgeteditor_Export.h>

// mitk qt widgets module
#include "QmitkAbstractMultiWidget.h"

class QGridLayout;

/**
* @brief The 'QmitkCustomMultiWidget' is a 'QmitkAbstractMultiWidget' that is used to display multiple render windows at once.
*
*     Render windows can dynamically be added and removed to change the layout of the multi widget.
*/
class CUSTOMMULTIWIDGETEDITOR_EXPORT QmitkCustomMultiWidget : public QmitkAbstractMultiWidget
{
  Q_OBJECT

public:

  QmitkCustomMultiWidget(QWidget* parent = 0,
                         Qt::WindowFlags f = 0,
                         mitk::RenderingManager* renderingManager = nullptr,
                         mitk::BaseRenderer::RenderingMode::Type renderingMode = mitk::BaseRenderer::RenderingMode::Standard,
                         const QString& multiWidgetName = "custommulti");

  virtual ~QmitkCustomMultiWidget() override;
  
  virtual void InitializeMultiWidget() override;

  virtual void SetSelectedPosition(const mitk::Point3D& newPosition, const QString& widgetName) override;
  virtual const mitk::Point3D GetSelectedPosition(const QString& widgetName) const override;

public Q_SLOTS:

  // mouse events
  virtual void wheelEvent(QWheelEvent* e) override;

  virtual void moveEvent(QMoveEvent* e) override;

Q_SIGNALS:

  void WheelMoved(QWheelEvent *);
  void Moved();

private:

  /**
  * @brief Overridden from QmitkAbstractMultiWidget
  */
  virtual void SetLayoutImpl() override;
  /**
  * @brief Overridden from QmitkAbstractMultiWidget
  */
  virtual void SynchronizeImpl() override { }
  /**
  * @brief Overridden from QmitkAbstractMultiWidget
  */
  virtual void SetInteractionSchemeImpl() override { }

  void InitializeLayout();
  void CreateRenderWindowWidget();
  void FillMultiWidgetLayout();

  QGridLayout* m_GridLayout;
};

#endif // QMITKCUSTOMMULTIWIDGET_H
