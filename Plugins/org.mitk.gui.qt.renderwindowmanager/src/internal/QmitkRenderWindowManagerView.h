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
#include "QmitkLayerManagerAddNodeWidget.h"
// render window manager module
#include <QmitkRenderWindowDataModel.h>
#include "QmitkVisibilityDelegate.h"
// core module
#include <mitkRenderWindowLayerController.h>

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

public Q_SLOTS:
  /**
  * @brief called when the user changes the render window selection in the combo box
  */
  void OnRenderWindowSelectionChanged(const QString &renderWindowId);

  void ShowAddDataNodeWidget();
  void AddDataNode(mitk::DataNode* dataNode);
  void RemoveDataNode();
  void SetAsBaseDataNode();

  void MoveDataNode(const QString &direction);

protected:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget* parent) override;

  void SetUpConnections();

private:

  // the Qt parent of our GUI
  QWidget* m_Parent;
  Ui::QmitkRenderWindowManagerControls m_Controls;

  QMap<QString,QmitkRenderWindowDataModel*> m_RenderWindowModel;
  QmitkVisibilityDelegate* m_VisibilityDelegate;
  QString m_CurrentRendererName;

  mitk::RenderWindowLayerController* m_RenderWindowLayerController;
  QmitkLayerManagerAddNodeWidget* m_AddDataNodeWidget;
};

#endif // QMITKRENDERWINDOWMANAGERVIEW_H
