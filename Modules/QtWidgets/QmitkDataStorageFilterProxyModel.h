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


#ifndef QMITKDATASTORAGEFILTERPROXYMODEL_H_
#define QMITKDATASTORAGEFILTERPROXYMODEL_H_

#include <MitkQtWidgetsExports.h>

#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>
#include <mitkWeakPointer.h>

#include <QSortFilterProxyModel>

#include "QmitkEnums.h"
#include "QmitkCustomVariants.h"

#include <vector>
#include <string>
#include <QList>

/// \ingroup QmitkModule
class QMITK_EXPORT QmitkDataStorageFilterProxyModel : public QSortFilterProxyModel
{
public:
//# CTORS,DTOR
public:
    QmitkDataStorageFilterProxyModel(bool _ShowHelperObjects=false
                            , bool _ShowNodesContainingNoData=false
                            , QObject* parent = 0);
    ~QmitkDataStorageFilterProxyModel();

//# GETTER
public:

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

  ///
  /// Get the helper object visibility flag
  ///
  bool GetShowHelperObjectsFlag()
  {
    return m_ShowHelperObjects;
  }

  ///
  /// Get the visibility flag for showing nodes that contain no data
  ///
  bool GetShowNodesContainingNoDataFlag()
  {
    return m_ShowNodesContainingNoData;
  }

//# SETTER
public:

  ///
  /// Show or hide helper objects
  ///
  void SetShowHelperObjects(bool _ShowHelperObjects);

  ///
  /// Show or hide objects that contain no data
  ///
  void SetShowNodesContainingNoData(bool _ShowNodesContainingNoData);

//# 
protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

//# MISC
protected:
  ///
  /// Update Tree Model according to predicates
  ///
    void UpdateNodeVisibility();

  //# ATTRIBUTES
protected:
  mitk::NodePredicateBase::Pointer m_Predicate;
  bool m_ShowHelperObjects;
  bool m_ShowNodesContainingNoData;

  NodeTagMapType  m_HelperObjectObserverTags;
};

#endif /* QMITKDATASTORAGEFILTERPROXYMODEL_H_ */
