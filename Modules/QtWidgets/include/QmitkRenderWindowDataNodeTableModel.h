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

/**
* \brief Table model that manages data nodes for a specific render window.
*
* The QmitkRenderWindowDataNodeTableModel extends QAbstractItemModel to provide
* a table view of data nodes assigned to a specific renderer. It supports
* drag-and-drop reordering of layers and toggling node visibility.
*/
class MITKQTWIDGETS_EXPORT QmitkRenderWindowDataNodeTableModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  QmitkRenderWindowDataNodeTableModel(QObject* parent = nullptr);

  /** \brief Updates the model data from the data storage for the current renderer. */
  void UpdateModelData();

  /**
   * \brief Sets the data storage to use.
   * \param[in] dataStorage Pointer to the data storage.
   */
  void SetDataStorage(mitk::DataStorage* dataStorage);
  /**
   * \brief Sets the current renderer whose layer stack is displayed.
   * \param[in] baseRenderer Pointer to the base renderer.
   */
  void SetCurrentRenderer(mitk::BaseRenderer* baseRenderer);
  /** \brief Returns the current renderer. */
  mitk::BaseRenderer::Pointer GetCurrentRenderer() const;

  /** \brief Convenience type alias for a list of data node pointers. */
  using NodeList = QList<mitk::DataNode::Pointer>;
  /**
   * \brief Sets the current selection of data nodes.
   * \param[in] selectedNodes The list of nodes to select.
   */
  void SetCurrentSelection(NodeList selectedNodes);
  /** \brief Returns the currently selected data nodes. */
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

  void moveNodesLayer(QSet<mitk::DataNode*> movedNodes, int targetLayer);
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

Q_SIGNALS:

  void ModelUpdated();
  void NodesLayerMoved(QSet<mitk::DataNode*> nodes, int layer);

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
