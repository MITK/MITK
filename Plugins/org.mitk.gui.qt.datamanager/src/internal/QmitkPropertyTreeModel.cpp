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

#include "QmitkPropertyTreeItem.h"
#include "QmitkPropertyTreeModel.h"
#include <mitkProperties.h>
#include <mitkColorProperty.h>
#include <mitkEnumerationProperty.h>
#include <mitkStringProperty.h>
#include <mitkRenderingManager.h>
#include <QColor>

QmitkPropertyTreeModel::QmitkPropertyTreeModel(QObject *parent)
: QAbstractItemModel(parent),
  m_Properties(NULL),
  m_RootItem(NULL)
{
  CreateRootItem();
}

void QmitkPropertyTreeModel::SetProperties(mitk::PropertyList *properties)
{
  if (properties == m_Properties)
    return;

  beginResetModel();

  if (m_RootItem != NULL)
  {
    delete m_RootItem;
    m_RootItem = NULL;
    m_Properties = NULL;
  }

  CreateRootItem();

  if (properties != NULL && !properties->IsEmpty())
  {
    m_Properties = properties;

    std::map<std::string, mitk::BaseProperty::Pointer>::const_iterator end = properties->GetMap()->end();

    for (std::map<std::string, mitk::BaseProperty::Pointer>::const_iterator iter = properties->GetMap()->begin(); iter != end; ++iter)
    {
      QList<QVariant> data;
      data << iter->first.c_str() << QVariant::fromValue<void *>((reinterpret_cast<void *>(iter->second.GetPointer())));
      m_RootItem->AppendChild(new QmitkPropertyTreeItem(data));
    }
  }

  endResetModel();
}

void QmitkPropertyTreeModel::CreateRootItem()
{
  if (m_RootItem == NULL)
  {
    QList<QVariant> rootData;
    rootData << "Property" << "Value";
    m_RootItem = new QmitkPropertyTreeItem(rootData);
  }
}

QmitkPropertyTreeModel::~QmitkPropertyTreeModel()
{
  delete m_RootItem;
}

int QmitkPropertyTreeModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return static_cast<QmitkPropertyTreeItem *>(parent.internalPointer())->GetColumnCount();
  else
    return m_RootItem->GetColumnCount();
}

QVariant QmitkPropertyTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.column() == 0 && role == Qt::DisplayRole)
    return static_cast<QmitkPropertyTreeItem *>(index.internalPointer())->GetData(index.column());

  if (index.column() == 1 && static_cast<QmitkPropertyTreeItem *>(index.internalPointer())->GetData(index.column()).isValid())
  {
    mitk::BaseProperty *property = reinterpret_cast<mitk::BaseProperty *>(static_cast<QmitkPropertyTreeItem *>(index.internalPointer())->GetData(index.column()).value<void *>());

    if (mitk::ColorProperty *colorProperty = dynamic_cast<mitk::ColorProperty *>(property))
    {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
      {
        mitk::Color mitkColor = colorProperty->GetColor();
        QColor qtColor(static_cast<int>(mitkColor.GetRed() * 255), static_cast<int>(mitkColor.GetGreen() * 255), static_cast<int>(mitkColor.GetBlue() * 255));
        return QVariant::fromValue<QColor>(qtColor);
      }
    }
    else if (mitk::BoolProperty *boolProperty = dynamic_cast<mitk::BoolProperty *>(property))
    {
      if (role == Qt::CheckStateRole)
        return boolProperty->GetValue() ? Qt::Checked : Qt::Unchecked;
    }
    else if (mitk::StringProperty *stringProperty = dynamic_cast<mitk::StringProperty *>(property))
    {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
        return QString(stringProperty->GetValue());
    }
    else if (mitk::IntProperty *intProperty = dynamic_cast<mitk::IntProperty *>(property))
    {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
        return intProperty->GetValue();
    }
    else if (mitk::FloatProperty *floatProperty = dynamic_cast<mitk::FloatProperty *>(property))
    {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
        return floatProperty->GetValue();
    }
    else if (mitk::DoubleProperty *doubleProperty = dynamic_cast<mitk::DoubleProperty *>(property))
    {
      if (role == Qt::DisplayRole || role == Qt::EditRole)
        return doubleProperty->GetValue();
    }
    else if (mitk::EnumerationProperty *enumProperty = dynamic_cast<mitk::EnumerationProperty *>(property))
    {
      if (role == Qt::DisplayRole)
      {
        return QString::fromStdString(enumProperty->GetValueAsString());
      }
      else if (role == Qt::EditRole)
      {
        QStringList values;

        for (mitk::EnumerationProperty::EnumConstIterator iter = enumProperty->Begin(); iter != enumProperty->End(); ++iter)
          values << QString::fromStdString(iter->second);

        return QVariant(values);
      }
    }
    else
    {
      if (role == Qt::DisplayRole)
        return QString::fromStdString(property->GetValueAsString());
    }
  }

  return QVariant();
}

Qt::ItemFlags QmitkPropertyTreeModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if (index.column() == 1)
  {
    if (index.data(Qt::EditRole).isValid())
      flags |= Qt::ItemIsEditable;

    if (index.data(Qt::CheckStateRole).isValid())
      flags |= Qt::ItemIsUserCheckable;
  }

  return flags;
}

QVariant QmitkPropertyTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    return m_RootItem->GetData(section);

  return QVariant();
}

QModelIndex QmitkPropertyTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  QmitkPropertyTreeItem *parentItem = parent.isValid() ? static_cast<QmitkPropertyTreeItem *>(parent.internalPointer()) : m_RootItem;
  QmitkPropertyTreeItem *childItem = parentItem->GetChild(row);

  if (childItem != NULL)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QmitkPropertyTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  QmitkPropertyTreeItem *parentItem = static_cast<QmitkPropertyTreeItem *>(child.internalPointer())->GetParent();

  if (parentItem == m_RootItem)
    return QModelIndex();

  return createIndex(parentItem->GetRow(), 0, parentItem);
}

int QmitkPropertyTreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0)
    return 0;

  QmitkPropertyTreeItem *parentItem = parent.isValid() ? static_cast<QmitkPropertyTreeItem *>(parent.internalPointer()) : m_RootItem;

  return parentItem->GetChildCount();
}

bool QmitkPropertyTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid() && (role == Qt::EditRole || role == Qt::CheckStateRole))
  {
    if (index.column() == 1)
    {
      mitk::BaseProperty *property = reinterpret_cast<mitk::BaseProperty *>(static_cast<QmitkPropertyTreeItem *>(index.internalPointer())->GetData(index.column()).value<void *>());

      if (mitk::ColorProperty *colorProperty = dynamic_cast<mitk::ColorProperty *>(property))
      {
        QColor qtColor = value.value<QColor>();

        if (!qtColor.isValid())
          return false;

        mitk::Color mitkColor = colorProperty->GetColor();
        mitkColor.SetRed(qtColor.red() / 255.0);
        mitkColor.SetGreen(qtColor.green() / 255.0);
        mitkColor.SetBlue(qtColor.blue() / 255.0);
        colorProperty->SetColor(mitkColor);

        m_Properties->InvokeEvent(itk::ModifiedEvent());
        m_Properties->Modified();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
      else if (mitk::BoolProperty *boolProperty = dynamic_cast<mitk::BoolProperty *>(property))
      {
        boolProperty->SetValue(value.toInt() == Qt::Checked ? true : false);

        m_Properties->InvokeEvent(itk::ModifiedEvent());
        m_Properties->Modified();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
      else if (mitk::StringProperty *stringProperty = dynamic_cast<mitk::StringProperty *>(property))
      {
        stringProperty->SetValue(value.toString().toStdString());

        m_Properties->InvokeEvent(itk::ModifiedEvent());
        m_Properties->Modified();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }
      else if (mitk::IntProperty *intProperty = dynamic_cast<mitk::IntProperty *>(property))
      {
        int intValue = value.toInt();

        if (intValue != intProperty->GetValue())
        {
          intProperty->SetValue(intValue);
          m_Properties->InvokeEvent(itk::ModifiedEvent());
          m_Properties->Modified();

          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      }
      else if (mitk::FloatProperty *floatProperty = dynamic_cast<mitk::FloatProperty *>(property))
      {
        float floatValue = value.toFloat();

        if (abs(floatValue - floatProperty->GetValue()) >= mitk::eps)
        {
          floatProperty->SetValue(floatValue);
          m_Properties->InvokeEvent(itk::ModifiedEvent());
          m_Properties->Modified();

          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      }
      else if (mitk::EnumerationProperty *enumProperty = dynamic_cast<mitk::EnumerationProperty *>(property))
      {
        std::string activatedItem = value.toString().toStdString();

        if (activatedItem != enumProperty->GetValueAsString())
        {
          enumProperty->SetValue(activatedItem);
          m_Properties->InvokeEvent(itk::ModifiedEvent());
          m_Properties->Modified();

          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      }
    }

    emit dataChanged(index, index);
    return true;
  }

  return false;
}
