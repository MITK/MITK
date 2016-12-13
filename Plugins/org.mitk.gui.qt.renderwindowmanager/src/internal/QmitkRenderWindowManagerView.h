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
#include "QmitkLayerManagerAddLayerWidget.h"
// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowViewDirectionController.h>
#include <QmitkRenderWindowDataModel.h>
#include "QmitkVisibilityDelegate.h"

// blueberry
#include <berryISelectionListener.h>

// qt
#include <QmitkAbstractView.h>
#include <QStandardItemModel.h>

/**
* @brief RenderWindowManager
*/
class QmitkRenderWindowManagerView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

protected:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget* parent) override;

  void SetUpConnections();

private Q_SLOTS:

  /**
  * @brief called when the user changes the render window selection in the combo box
  */
  void OnRenderWindowSelectionChanged(const QString &renderWindowId);

  void ShowAddLayerWidget();
  void AddLayer(mitk::DataNode* dataNode);
  void RemoveLayer();
  void SetAsBaseLayer();

  void MoveLayer(const QString &direction);

  void ResetRenderer();
  void ClearRenderer();

  void ChangeViewDirection(const QString &viewDirection);

private:

  /**
  * @brief set each data node invisible in all render windows, as soon as the node is added to the data storage
  */
  void NodeAdded(const mitk::DataNode* node) override;

  // the Qt parent of our GUI
  QWidget* m_Parent;
  Ui::QmitkRenderWindowManagerControls m_Controls;

  std::unique_ptr<QmitkRenderWindowDataModel> m_RenderWindowDataModel;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  std::unique_ptr<mitk::RenderWindowViewDirectionController> m_RenderWindowViewDirectionController;
  QmitkLayerManagerAddLayerWidget* m_AddLayerWidget;
};

#endif // QMITKRENDERWINDOWMANAGERVIEW_H
