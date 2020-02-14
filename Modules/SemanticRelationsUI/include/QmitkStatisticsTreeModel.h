/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKSTATISTICSTREEMODEL_H
#define QMITKSTATISTICSTREEMODEL_H

// mitk semantic relations UI
#include "MitkSemanticRelationsUIExports.h"
#include "QmitkAbstractSemanticRelationsStorageModel.h"
#include "QmitkLesionTreeItem.h"
#include "QmitkStatisticsCalculator.h"

/*
* @brief The 'QmitkStatisticsTreeModel' is a subclass of 'QmitkAbstractSemanticRelationsStorageModel' and provides
*        functionality to serve as a tree model.
*        The tree model creates a new top-level tree item for each lesion that is stored inside the semantic relations storage.
*        The top-level lesion tree items hold child items for each information type.
*        Each lesion tree item contains lesion data that can be display inside a tree view. The lesion data
*        consists of a lesion with with its UID, name and lesion class. The name or UID is used for the top-level tree items.
*        Additionally the lesion data contains two vectors which define the lesion presence (bool) and the lesion volume (double)
*        for each control-point - information type pair. The lesion volume will be used inside this model for the child items.
*        The presence is used inside another tree model.
*
*        The model uses the 'QmitkStatisticsCalculator' to start the lesion volume calculation for each lesion.
*        This calculator is able to find an existing lesion volume or to trigger the computation of the required statistics.
*        If the required statistics are newly computed and added as a statistics container to the data storage,
*        this model will be notified about this event (see 'NodeAdded', 'NodeChanged' and 'NodeRemoved') and will update
*        its lesion tree items.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkStatisticsTreeModel : public QmitkAbstractSemanticRelationsStorageModel
{
  Q_OBJECT

public:

  /**
  *	@brief Initialize the root item of the model. The root item does not have a parent item.
  */
  QmitkStatisticsTreeModel(QObject* parent = nullptr);

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

protected:

  void DataStorageChanged() override;
  void NodePredicateChanged() override { }
  void NodeAdded(const mitk::DataNode*) override;
  void NodeChanged(const mitk::DataNode*) override;
  void NodeRemoved(const mitk::DataNode*) override;
  /**
  * @brief Overridden from 'QmitkAbstractSemanticRelationsStorageModel': This function retrieves all control points
  *        of the current case and stores them to define the header of the tree.
  *        Furthermore all lesions are retrieved and the lesion data is stored and show in the tree view.
  */
  void SetData() override;

private:

  void SetLesionData();
  void AddLesion(const mitk::SemanticTypes::Lesion& lesion);

  QmitkLesionTreeItem* GetItemByIndex(const QModelIndex& index) const;

  std::unique_ptr<QmitkStatisticsCalculator> m_StatisticsCalculator;

  std::shared_ptr<QmitkLesionTreeItem> m_RootItem;
  mitk::SemanticTypes::ControlPointVector m_ControlPoints;
  mitk::SemanticTypes::InformationTypeVector m_InformationTypes;
  mitk::SemanticTypes::LesionVector m_CurrentLesions;
};

#endif // QMITKSTATISTICSTREEMODEL_H
