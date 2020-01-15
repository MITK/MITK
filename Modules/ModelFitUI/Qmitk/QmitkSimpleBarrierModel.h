/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleBarrierModel_h
#define QmitkSimpleBarrierModel_h

#include <QAbstractTableModel>

#include "mitkSimpleBarrierConstraintChecker.h"
#include "mitkModelTraitsInterface.h"

#include "MitkModelFitUIExports.h"


/*!
\class QmitkSimpleBarrierModel
Model that handles a SimpleBarrierConstraintChecker and his defined constraints.
It allows to couple a SimpleBarrierConstraintChecker with a Qt table view in
Order to view and edit its contents. E.g. used in the QmitkSimpleBarrierManagerWidget.
*/
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  QmitkSimpleBarrierModel(QObject* parent = nullptr);
  ~QmitkSimpleBarrierModel() override {};

  /** Sets the data handled by the model and resets the modified flag
   @param pChecker Pointer to the checker instance that should be managed.
   @param names List of all possible parameter names. It is assumed that the
   index of the list equals the parameter index in the respective fitting model.*/
  void setChecker(mitk::SimpleBarrierConstraintChecker* pChecker,
                  const mitk::ModelTraitsInterface::ParameterNamesType& names);

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  void addConstraint();
  void deleteConstraint(const QModelIndex& index);

  /**Indicates if the content of the model was modified since the data was set via setChecker()*/
  bool isModified();

private:
  mitk::SimpleBarrierConstraintChecker::Pointer m_Checker;
  mitk::ModelTraitsInterface::ParameterNamesType m_ParameterNames;

  /** Indicates if the data of the model was modified, since the model was set. */
  bool m_modified;

};

#endif // QmitkSimpleBarrierModel_h

