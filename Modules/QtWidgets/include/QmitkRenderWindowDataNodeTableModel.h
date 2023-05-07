/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRenderWindowDataNodeTableModel_h
#define QmitkRenderWindowDataNodeTableModel_h

#include <MitkQtWidgetsExports.h>

//mitk core
#include <mitkBaseRenderer.h>
#include <mitkBaseProperty.h>
#include <mitkDataStorage.h>
#include <mitkWeakPointer.h>

// qt widgets module
#include <mitkRenderWindowLayerController.h>
#include <mitkRenderWindowLayerUtilities.h>
#include <QmitkAbstractDataStorageModel.h>

#include <QIcon>

/*
* @brief The 'QmitkRenderWindowDataNodeTableModel' is a table model that extends the 'QAbstractItemModel'.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowDataNodeTableModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  QmitkRenderWindowDataNodeTableModel(QObject* parent = nullptr);

  void UpdateModelData();

  void SetDataStorage(mitk::DataStorage* dataStorage);
  void SetCurrentRenderer(mitk::BaseRenderer* baseRenderer);
  mitk::BaseRenderer::Pointer GetCurrentRenderer() const;

  using NodeList = QList<mitk::DataNode::Pointer>;
  void SetCurrentSelection(NodeList selectedNodes);
  NodeList GetCurrentSelection() const;

  // override from 'QAbstractItemModel'
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& child) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

  Qt::DropActions supportedDropActions() const override;
  Qt::DropActions supportedDragActions() const override;
  QStringList mimeTypes() const override;
  QMimeData* mimeData(const QModelIndexList& indexes) const override;

  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

Q_SIGNALS:

  void ModelUpdated();

private:

  std::unique_ptr<mitk::RenderWindowLayerController> m_RenderWindowLayerController;
  mitk::WeakPointer<mitk::BaseRenderer> m_BaseRenderer;
  NodeList m_CurrentSelection;

  QIcon m_VisibleIcon;
  QIcon m_InvisibleIcon;
  QIcon m_ArrowIcon;
  QIcon m_TimesIcon;
};

#endif
