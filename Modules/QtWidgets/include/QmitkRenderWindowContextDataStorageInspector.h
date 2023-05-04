/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowContextDataStorageInspector_h
#define QmitkRenderWindowContextDataStorageInspector_h

// qt widgets module
#include "MitkQtWidgetsExports.h"
#include "ui_QmitkRenderWindowContextDataStorageInspector.h"

// render window manager module
#include <mitkRenderWindowLayerController.h>
#include <QmitkRenderWindowDataStorageTreeModel.h>

// qt widgets module
#include <QmitkAbstractDataStorageInspector.h>

// mitk core
#include <mitkBaseRenderer.h>

/**
* The 'QmitkRenderWindowContextDataStorageInspector' offers a GUI to manipulate the base renderer / render windows of the MITK workbench.
*
* In order to use this widget, a (e.g.) plugin has to set the controlled renderer, which will be forwarded to
* a render window view direction controller.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowContextDataStorageInspector : public QmitkAbstractDataStorageInspector
{
  Q_OBJECT

public:

  QmitkRenderWindowContextDataStorageInspector(QWidget* parent = nullptr, mitk::BaseRenderer* renderer = nullptr);

  // override from 'QmitkAbstractDataStorageInspector'
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  QAbstractItemView* GetView() override;
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  const QAbstractItemView* GetView() const override;
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  void SetSelectionMode(SelectionMode mode) override;
  /**
  * @brief See 'QmitkAbstractDataStorageInspector'
  */
  SelectionMode GetSelectionMode() const override;

  QItemSelectionModel* GetDataNodeSelectionModel() const;

Q_SIGNALS:

  void ReinitAction(QList<mitk::DataNode::Pointer> selectedNodes);
  void ResetAction(QList<mitk::DataNode::Pointer> selectedNodes);

private Q_SLOTS:

  void ModelRowsInserted(const QModelIndex& parent, int start, int end);

  void OnContextMenuRequested(const QPoint& pos);
  void OnReinit();
  void OnReset();

private:

  void Initialize() override;
  void SetUpConnections();

  Ui::QmitkRenderWindowContextDataStorageInspector m_Controls;

  std::unique_ptr<QmitkRenderWindowDataStorageTreeModel> m_StorageModel;
  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
};

#endif
