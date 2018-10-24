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

#ifndef QMITKRENDERWINDOWMANAGERVIEW_H
#define QMITKRENDERWINDOWMANAGERVIEW_H

// render window manager plugin
#include "ui_QmitkRenderWindowManagerControls.h"

// render window manager UI module
#include <QmitkRenderWindowManipulatorWidget.h>

// blueberry
#include <berryISelectionListener.h>

#include <mitkIRenderWindowPartListener.h>

// qt
#include <QmitkAbstractView.h>

/**
* @brief RenderWindowManager
*/
class QmitkRenderWindowManagerView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  virtual void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  virtual void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  virtual void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

protected:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget* parent) override;

private Q_SLOTS:

  /**
  * @brief Called when the user changes the render window selection in the combo box.
  *
  * @param renderWindowId   The text inside the combo box.
  */
  void OnRenderWindowSelectionChanged(const QString &renderWindowId);
  /**
  * @brief Called when the 'AddLayer'-button of he render window manipulator widget has been pushed.
  */
  void OnAddLayerButtonClicked();

private:

  void SetControlledRenderer();

  /**
  * @brief Reacts to a node that has been added to the data storage.
  *     1. Insert new node into the node list of all render windows, if it is an "globalObject_RWM"-node.
  *  or else
  *     2. Set data node invisible in all render windows, as soon as the node is added to the data storage.
  */
  void NodeAdded(const mitk::DataNode* node) override;

  // the Qt parent of our GUI
  QWidget* m_Parent;
  Ui::QmitkRenderWindowManagerControls m_Controls;

  mitk::IRenderWindowPart* m_RenderWindowPart;

  QmitkRenderWindowManipulatorWidget* m_RenderWindowManipulatorWidget;
  RenderWindowLayerUtilities::RendererVector m_ControlledRenderer;
};

#endif // QMITKRENDERWINDOWMANAGERVIEW_H
