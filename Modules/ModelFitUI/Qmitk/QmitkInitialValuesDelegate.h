/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInitialValuesDelegate_h
#define QmitkInitialValuesDelegate_h


/// Toolkit includes.
#include <mitkDataStorage.h>
#include <mitkNodePredicateBase.h>

#include <QStyledItemDelegate>

#include "MitkModelFitUIExports.h"


/** \class QmitkInitialValuesDelegate
\brief An item delegate for rendering and editing the initial value (source) for a parameter.
The delegate assumes the following: 1) the data requested with the
edit role, will be an double if it is an simple scalar type. If the type is image,
it will be a pointer to the currently selected data node. If nothing is selected now it will be
a nullptr.*/
class MITKMODELFITUI_EXPORT QmitkInitialValuesDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  QmitkInitialValuesDelegate(QObject* parent = nullptr);

  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option
                        , const QModelIndex& index) const override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;

  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

  void setDataStorage(mitk::DataStorage* storage);
  void setNodePredicate(mitk::NodePredicateBase* predicate);

protected:
  int valueType(const QModelIndex& index) const;

  /**Storage is used as source for potantial initial value images.*/
  mitk::DataStorage::Pointer m_Storage;
  /**Predicates that is used to filter for potential/allowed images in the data storage*/
  mitk::NodePredicateBase::Pointer m_Predicate;

};

#endif
