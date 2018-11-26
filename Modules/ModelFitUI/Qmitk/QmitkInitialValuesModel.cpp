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

#include <QColor>
#include <QSize>

#include "mitkExceptionMacro.h"
#include "mitkImage.h"
#include "mitkImageBasedParameterizationDelegate.h"

#include "QmitkInitialValuesModel.h"


QmitkInitialValuesModel::
QmitkInitialValuesModel(QObject* parent) :
  QAbstractTableModel(parent), m_modified(false)
{
}

void
QmitkInitialValuesModel::
setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names,
                 const mitk::ModelTraitsInterface::ParametersType values)
{
  if (names.size() != values.size())
  {
    mitkThrow() <<
                "Error. Cannot set initial value model. Passed parameter names vector and default values vector have different size.";
  }

  emit beginResetModel();

  this->m_ParameterNames = names;
  this->m_Values = values;

  this->m_modified = false;

  emit endResetModel();
};

void
QmitkInitialValuesModel::
setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names)
{
  mitk::ModelTraitsInterface::ParametersType values;
  values.set_size(names.size());
  values.fill(0.0);

  setInitialValues(names, values);
};

void
QmitkInitialValuesModel::
addInitialParameterImage(const mitk::DataNode* node, mitk::ModelTraitsInterface::ParametersType::size_type paramIndex)
{
  if (!node) mitkThrow() << "Try to set a null ptr as initial value source image.";

  if (!dynamic_cast<mitk::Image*>(node->GetData())) mitkThrow() << "Error. Passed node does not contain an image.";

  emit beginResetModel();

  this->m_ParameterImageMap[paramIndex] = node;

  emit endResetModel();
};

void QmitkInitialValuesModel::resetInitialParameterImage()
{
  emit beginResetModel();

  this->m_ParameterImageMap.clear();

  emit endResetModel();
};

mitk::ModelTraitsInterface::ParametersType
QmitkInitialValuesModel::
getInitialValues() const
{
  return this->m_Values;
};

mitk::InitialParameterizationDelegateBase::Pointer
QmitkInitialValuesModel::
getInitialParametrizationDelegate() const
{
  mitk::ImageBasedParameterizationDelegate::Pointer initDelegate = mitk::ImageBasedParameterizationDelegate::New();
  initDelegate->SetInitialParameterization(m_Values);

  for (const auto& pos : this->m_ParameterImageMap)
  {
    initDelegate->AddInitialParameterImage(dynamic_cast<mitk::Image*>(pos.second->GetData()), pos.first);
  }

  return initDelegate.GetPointer();
};

bool
QmitkInitialValuesModel::
hasValidInitialValues() const
{
  for (const auto& pos : this->m_ParameterImageMap)
  {
    if (pos.second.IsNull()) return false;
  }
  return true;
};

int
QmitkInitialValuesModel::
rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return m_Values.size();
}

int
QmitkInitialValuesModel::
columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
  {
    return 0;
  }

  return 3;
}

int
QmitkInitialValuesModel::
valueType(const QModelIndex& index) const
{
  if (m_ParameterImageMap.find(index.row()) != m_ParameterImageMap.end())
  { //image type
    return 1;
  }
  else
  { //simple scalar
    return 0;
  }
}

QVariant
QmitkInitialValuesModel::
data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
  {
    return QVariant();
  }

  QVariant result;

  if (index.row() < static_cast<int>(m_Values.size()))
  {
    switch (index.column())
    {
      case 0:
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
          result = QVariant(QString::fromStdString(m_ParameterNames[index.row()]));
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Name of the parameter.");
        }

        break;

      case 1:
        if (role == Qt::DisplayRole)
        {
          if (valueType(index) == 1)
          { //image type
            result = QVariant("image");
          }
          else
          { //simple scalar
            result = QVariant("scalar");
          }
        }
        else if (role == Qt::EditRole)
        {
          result = QVariant(valueType(index));
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("type of the inital value.");
        }
        break;

      case 2:
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
          const auto& finding = m_ParameterImageMap.find(index.row());
          if (finding != m_ParameterImageMap.end())
          { //image type -> return the name
            if (finding->second.IsNotNull())
            {
              result = QVariant(finding->second->GetName().c_str());
            }
            else
            {
              result = QVariant("Invalid. Select image.");
            }
          }
          else
          { //simple scalar
            result = QVariant(m_Values(index.row()));
          }
        }
        else if (role == Qt::UserRole)
        {
          result = QVariant(valueType(index));
        }
        else if (role == Qt::ToolTipRole)
        {
          result = QVariant("Initial values for the parameter.");
        }

        break;
    }
  }

  return result;
}

Qt::ItemFlags
QmitkInitialValuesModel::
flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if (index.row() < static_cast<int>(m_Values.size()))
  {
    if (index.column() > 0)
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
QmitkInitialValuesModel::
headerData(int section, Qt::Orientation orientation, int role) const
{
  if ((Qt::DisplayRole == role) &&
      (Qt::Horizontal == orientation))
  {
    if (section == 0)
    {
      return QVariant("Parameters");
    }
    else if (section == 1)
    {
      return QVariant("Type");
    }
    else if (section == 2)
    {
      return QVariant("Value");
    }
  }

  return QVariant();
}

bool
QmitkInitialValuesModel::
setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (!index.isValid() || index.row() >= static_cast<int>(m_Values.size()) || (index.column() == 0))
  {
    return false;
  }

  if (Qt::EditRole == role)
  {
    emit dataChanged(index, index);

    emit beginResetModel();

    bool result = false;
    if (index.column() == 1)
    {
      if (value.toInt() == 0)
      {
        this->m_ParameterImageMap.erase(index.row());
        m_modified = true;
        result = true;
      }
      else
      {
        this->m_ParameterImageMap[index.row()] = nullptr;
        m_modified = true;
        result = true;
      }
    }
    else
    {
      if (valueType(index) == 0)
      {
        m_Values[index.row()] = value.toDouble();
        m_modified = true;
        result = true;
      }
      else
      {
        mitk::DataNode *node = static_cast<mitk::DataNode*>(value.value<void*>());
        if (node && dynamic_cast<mitk::Image*>(node->GetData()))
        {
          this->m_ParameterImageMap[index.row()] = node;
          m_modified = true;
          result = true;
        }
      }
    }

    emit endResetModel();

    return result;
  }

  return false;
};

bool QmitkInitialValuesModel::isModified()
{
  return m_modified;
}
