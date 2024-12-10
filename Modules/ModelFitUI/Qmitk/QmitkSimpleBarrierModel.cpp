/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QColor>
#include <QSize>

#include "QmitkSimpleBarrierModel.h"

const int NUMBER_OF_CONSTRAINT_ASPECTS = 5;
const int INDEX_CONSTRAINT_PARAMS = 0;
const int INDEX_CONSTRAINT_TYPE = 1;
const int INDEX_CONSTRAINT_THRESHOLD = 2;
const int INDEX_CONSTRAINT_WIDTH = 3;
const int INDEX_CONSTRAINT_PARAM_UNITS = 4;

QmitkSimpleBarrierModel::
QmitkSimpleBarrierModel(QObject* parent) :
  QAbstractTableModel(parent), m_modified(false)
{
  m_Checker = mitk::SimpleBarrierConstraintChecker::New();
}

void
QmitkSimpleBarrierModel::
setChecker(mitk::SimpleBarrierConstraintChecker* pChecker,
           const mitk::ModelTraitsInterface::ParameterNamesType& names,
           const mitk::ModelTraitsInterface::ParamterUnitMapType units)
{
  if (pChecker != m_Checker)
  {
    emit beginResetModel();

    if (pChecker)
    {
      m_Checker = pChecker;
    }
    else
    {
      m_Checker = mitk::SimpleBarrierConstraintChecker::New();
    }

    m_ParameterNames = names;
    m_modified = false;
    m_ParameterUnits = units;

    emit endResetModel();
  }
};

int
QmitkSimpleBarrierModel::
rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_Checker->GetNumberOfConstraints();
}

int
QmitkSimpleBarrierModel::
columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return NUMBER_OF_CONSTRAINT_ASPECTS;
}

QVariant
QmitkSimpleBarrierModel::
data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  QVariant result;

  if (static_cast<unsigned int>(index.row()) < m_Checker->GetNumberOfConstraints())
  {
    const mitk::SimpleBarrierConstraintChecker::Constraint constraint = m_Checker->GetConstraint(
          static_cast<unsigned int>(index.row()));

    switch (index.column())
    {
    case INDEX_CONSTRAINT_PARAMS:
        if (role == Qt::DisplayRole)
        {
          QStringList names;

          for (mitk::SimpleBarrierConstraintChecker::ParameterIndexVectorType::const_iterator pos =
                 constraint.parameters.begin(); pos != constraint.parameters.end(); ++pos)
          {
            names.append(QString::fromStdString(this->m_ParameterNames[*pos]));
          }

          result = QVariant(names);
        }
        else if (role == Qt::EditRole)
        {
          QStringList names;

          for (mitk::ModelTraitsInterface::ParameterNamesType::const_iterator pos =
                 this->m_ParameterNames.begin(); pos != this->m_ParameterNames.end(); ++pos)
          {
            names.append(QString::fromStdString(*pos));
          }

          result = QVariant(names);
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Parameters that are relevant for this constraint. If more then one parameter is specified, it is the sum of all parameters.");
        }

        break;

      case INDEX_CONSTRAINT_TYPE:
        if (role == Qt::DisplayRole)
        {
          if (constraint.upperBarrier)
          {
            result = QVariant(QString("upper"));
          }
          else
          {
            result = QVariant(QString("lower"));
          }
        }
        else if (role == Qt::EditRole)
        {
          if (constraint.upperBarrier)
          {
            result = QVariant(1);
          }
          else
          {
            result = QVariant(0);
          }
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Type of boundary constraint.");
        }

        break;

      case INDEX_CONSTRAINT_THRESHOLD:
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
          result = QVariant(constraint.barrier);
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Barrier value for the constraint.");
        }

        break;

      case INDEX_CONSTRAINT_WIDTH:
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
          result = QVariant(constraint.width);
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Width of the penalty zone before the barrier value. Must be >=0.0.");
        }

        break;

      case INDEX_CONSTRAINT_PARAM_UNITS:
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
          QStringList units;

          for (mitk::SimpleBarrierConstraintChecker::ParameterIndexVectorType::const_iterator pos =
            constraint.parameters.begin(); pos != constraint.parameters.end(); ++pos)
          {
            const auto& finding = m_ParameterUnits.find(this->m_ParameterNames[*pos]);
            if (finding != m_ParameterUnits.end())
            {
              units.append(QString::fromStdString(finding->second));
            }
          }
          result = QVariant(units);
        }

        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Parameters that are relevant for this constraint. If more then one parameter is specified, it is the sum of all parameters.");
        }

        break;

    }
  }

  return result;
}

Qt::ItemFlags
QmitkSimpleBarrierModel::
flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if (static_cast<unsigned int>(index.row()) < m_Checker->GetNumberOfConstraints())
  {
    if (index.column() < NUMBER_OF_CONSTRAINT_ASPECTS - 1 && index.column() >= 0)
    {
      flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    else
    {
      flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
  }
  return flags;
}

QVariant
QmitkSimpleBarrierModel::
headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) &&
      (Qt::Horizontal == orientation))
  {
    if (section == INDEX_CONSTRAINT_PARAMS)
    {
      return QVariant("Parameters");
    }
    else if (section == INDEX_CONSTRAINT_TYPE)
    {
      return QVariant("Type");
    }
    else if (section == INDEX_CONSTRAINT_THRESHOLD)
    {
      return QVariant("Value");
    }
    else if (section == INDEX_CONSTRAINT_WIDTH)
    {
      return QVariant("Width");
    }
    else if (section == INDEX_CONSTRAINT_PARAM_UNITS)
    {
      return QVariant("Unit");
    }
  }

  return QVariant();
}

bool
QmitkSimpleBarrierModel::
setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || (m_Checker->GetNumberOfConstraints() <= static_cast<unsigned int>(index.row()))
      || (index.column() >= NUMBER_OF_CONSTRAINT_ASPECTS))
  {
    return false;
  }

  if (Qt::EditRole == role)
  {
    mitk::SimpleBarrierConstraintChecker::Constraint constraint = m_Checker->GetConstraint(
          static_cast<unsigned int>(index.row()));

    bool changed = false;

    switch (index.column())
    {
      case 0:
      {
        QStringList selectedNames = value.toStringList();
        constraint.parameters.clear();

        for (mitk::SimpleBarrierConstraintChecker::ParameterIndexVectorType::size_type pos = 0;
             pos < this->m_ParameterNames.size(); ++pos)
        {
          if (selectedNames.contains(QString::fromStdString(this->m_ParameterNames[pos])))
          {
            constraint.parameters.push_back(pos);
          }
        }
        changed = true;
        break;
      }

      case 1:
        constraint.upperBarrier = value.toInt() == 1;
        changed = true;
        break;

      case 2:
        constraint.barrier = value.toDouble();
        changed = true;
        break;

      case 3:
        constraint.width = value.toDouble();
        changed = true;
        break;

    }

    if (changed)
    {
      m_Checker->GetConstraint(static_cast<unsigned int>(index.row())) = constraint;
      emit dataChanged(index, index);
      m_modified = true;
    }

    return changed;
  }

  return false;
};

void QmitkSimpleBarrierModel::addConstraint()
{
  emit beginResetModel();
  m_Checker->SetLowerBarrier(0, 0.0);
  m_modified = true;
  emit endResetModel();
}

void QmitkSimpleBarrierModel::deleteConstraint(const QModelIndex& index)
{
  if (!index.isValid() || (m_Checker->GetNumberOfConstraints() <= static_cast<unsigned int>(index.row()))
      || (index.column() >= NUMBER_OF_CONSTRAINT_ASPECTS))
  {
    return;
  }

  emit beginResetModel();
  m_Checker->DeleteConstraint(static_cast<unsigned int>(index.row()));
  m_modified = true;
  emit endResetModel();
}

bool QmitkSimpleBarrierModel::isModified()
{
  return m_modified;
}
