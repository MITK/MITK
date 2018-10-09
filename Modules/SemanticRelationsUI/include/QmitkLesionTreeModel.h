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

#ifndef QMITKLESIONTREEMODEL_H
#define QMITKLESIONTREEMODEL_H

// mitk semantic relations UI
#include "MitkSemanticRelationsUIExports.h"
#include "QmitkAbstractSemanticRelationsStorageModel.h"
#include "QmitkLesionTreeItem.h"

// c++
#include <memory>

/*
* @brief 
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkLesionTreeModel : public QmitkAbstractSemanticRelationsStorageModel
{
  Q_OBJECT 
   
public:

  /**
  *	@brief Initialize the root item of the model. The root item does not have a parent item.
  */
  QmitkLesionTreeModel(QObject* parent = nullptr);
  ~QmitkLesionTreeModel();

  //////////////////////////////////////////////////////////////////////////
  // overridden virtual functions from QAbstractItemModel
  //////////////////////////////////////////////////////////////////////////
  virtual QModelIndex index(int row, int column, const QModelIndex& itemIndex = QModelIndex()) const override;
  virtual QModelIndex parent(const QModelIndex& itemIndex) const override;

  virtual int rowCount(const QModelIndex& itemIndex = QModelIndex()) const override;
  virtual int columnCount(const QModelIndex& itemIndex = QModelIndex()) const override;

  virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  //////////////////////////////////////////////////////////////////////////
  // end override
  //////////////////////////////////////////////////////////////////////////

protected:
  
  // the following functions have to be overridden but are not implemented in this model
  virtual void NodePredicateChanged() override;
  virtual void NodeAdded(const mitk::DataNode* node) override;
  virtual void NodeChanged(const mitk::DataNode* node) override;
  virtual void NodeRemoved(const mitk::DataNode* node) override;
  /**
  * @brief Overridden from 'QmitkAbstractSemanticRelationsStorageModel': This function retrieves all control points
  *        of the current case and stores them to define the header of the tree.
  *        Furthermore all lesions are retrieved and the lesion data is stored and show in the tree view.
  */
  virtual void SetData() override;

private:

  void SetLesionData();
  void AddLesion(const mitk::SemanticTypes::Lesion& lesion);

  QmitkLesionTreeItem* GetItemByIndex(const QModelIndex& index) const;

  std::shared_ptr<QmitkLesionTreeItem> m_RootItem;
  std::vector<mitk::SemanticTypes::ControlPoint> m_ControlPoints;

};

#endif // QMITKLESIONTREEMODEL_H
