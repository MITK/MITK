/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLesionTreeModel_h
#define QmitkLesionTreeModel_h

// mitk semantic relations UI
#include "MitkSemanticRelationsUIExports.h"
#include "QmitkAbstractSemanticRelationsStorageModel.h"
#include "QmitkLesionTreeItem.h"

/*
* @brief The 'QmitkLesionTreeModel' is a subclass of 'QmitkAbstractSemanticRelationsStorageModel' and provides
*        functionality to serve as a tree model.
*        The tree model creates a new top-level tree item for each lesion that is stored inside the semantic relations storage.
*        Each lesion tree item contains lesion data that can be display inside a tree view. The lesion data
*        consists of a lesion with with its UID, name and lesion class. The name or UID is used for the top-level tree items.
*        Additionally the lesion data contains two vectors which define the lesion presence (bool) and the lesion volume (double)
*        for each control-point - information type pair. The lesion presence will be used inside this model for the tree items.
*        The volume is used inside another tree model.
*
*        The model holds the last segmentation that is added to the data storage to support the process of defining a new lesion
*        (and linking it with the latest segmentation) (see 'NodeAdded').
*        Furthermore the model is able to accept a 'QList' of currently selected data nodes and to use it to change the background
*        color of each lesion tree item that is connected to this data node(s). This helps to see which lesion is already found and
*        defined for a given (set of) data node(s).
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkLesionTreeModel : public QmitkAbstractSemanticRelationsStorageModel
{
  Q_OBJECT

public:

  /**
  *	@brief Initialize the root item of the model. The root item does not have a parent item.
  */
  QmitkLesionTreeModel(QObject* parent = nullptr);

  //////////////////////////////////////////////////////////////////////////
  // overridden virtual functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  QModelIndex index(int row, int column, const QModelIndex& itemIndex = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& itemIndex) const override;

  int rowCount(const QModelIndex& itemIndex = QModelIndex()) const override;
  int columnCount(const QModelIndex& itemIndex = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  //////////////////////////////////////////////////////////////////////////
  // end override
  //////////////////////////////////////////////////////////////////////////

  const mitk::DataNode* GetLastSegmentation() const;

protected:

  // the following functions have to be overridden but are not implemented in this model
  void NodePredicateChanged() override { }
  void NodeAdded(const mitk::DataNode*) override;
  void NodeChanged(const mitk::DataNode*) override { }
  void NodeRemoved(const mitk::DataNode*) override { }
  /**
  * @brief Overridden from 'QmitkAbstractSemanticRelationsStorageModel': This function retrieves all control points
  *        of the current case and stores them to define the header of the tree.
  *        Furthermore all lesions are retrieved and the lesion data is stored and show in the tree view.
  */
  void SetData() override;

private:

  void SetLesionData();
  void AddLesion(const mitk::SemanticTypes::Lesion& lesion);
  void SetSelectedDataNodesPresence();
  /**
  * @brief The function uses the ID of the lesion to see if a data node presence was already set.
  *        If not, the given bool value is used and stored inside a member variable. If the lesion presence
  *        was already set, it will be overwritten.
  *        The function is used by the 'SetSelectedDataNodesPresence' function.
  *
  * @param lesion             The lesion whose data node presence should be set
  * @param dataNodePresence   The bool value that defines the data node presence of the given lesion
  */
  void SetDataNodePresenceOfLesion(const mitk::SemanticTypes::Lesion* lesion, bool dataNodePresence);

  QmitkLesionTreeItem* GetItemByIndex(const QModelIndex& index) const;

  std::map<mitk::SemanticTypes::ID, bool> m_DataNodePresence;
  const mitk::DataNode* m_LastSegmentation;

  std::shared_ptr<QmitkLesionTreeItem> m_RootItem;
  mitk::SemanticTypes::ControlPointVector m_ControlPoints;
  mitk::SemanticTypes::LesionVector m_CurrentLesions;
};

#endif
