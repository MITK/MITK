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


#include "mitkQMAPAlgorithmModel.h"

#include "mapMetaProperty.h"

namespace mitk
{

QMAPAlgorithmModel::
QMAPAlgorithmModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_pMetaInterface(nullptr)
{

}

void
QMAPAlgorithmModel::
SetAlgorithm(map::algorithm::RegistrationAlgorithmBase *pAlgorithm)
{
    this->SetAlgorithm(dynamic_cast<map::algorithm::facet::MetaPropertyAlgorithmInterface*>(pAlgorithm));
};

void
QMAPAlgorithmModel::
SetAlgorithm(map::algorithm::facet::MetaPropertyAlgorithmInterface *pMetaInterface)
{
    emit beginResetModel();

    m_pMetaInterface = pMetaInterface;
    this->UpdateMetaProperties();

    emit endResetModel();
};

int
QMAPAlgorithmModel::
rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
    {
        return 0;
    }

    return m_MetaProperties.size();
}

int
QMAPAlgorithmModel::
columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;

    return 2;
}

QVariant
QMAPAlgorithmModel::
data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if(!m_pMetaInterface)
    {
        return QVariant();
    }

    QVariant result;

    if(index.row()<m_MetaProperties.size())
    {
        map::algorithm::MetaPropertyInfo* pInfo = m_MetaProperties[index.row()];

        switch(index.column())
        {
        case 0:
            if(Qt::DisplayRole == role && index.row()<m_MetaProperties.size())
            {
                result = QVariant(pInfo->getName().c_str());
            }
            break;
        case 1:
            if (Qt::DisplayRole == role && !pInfo->isReadable())
            {
              result =  QVariant("value is not accessible");
            }
            else if (pInfo->isReadable() && (Qt::DisplayRole == role || (Qt::EditRole == role && pInfo->isWritable())))
            { //should also be readable to be sensible editable in the GUI
              result = GetPropertyValue(pInfo,role);
            }
            break;
        }
    }

    return result;
}

Qt::ItemFlags
QMAPAlgorithmModel::
flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if(index.row()<m_MetaProperties.size())
  {
      map::algorithm::MetaPropertyInfo* pInfo = m_MetaProperties[index.row()];
      if (index.column() == 1)
      {
        if (index.data(Qt::EditRole).isValid() && pInfo->isWritable())
        {
          flags |= Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
        }
      }
  }

  return flags;
}

QVariant
QMAPAlgorithmModel::
headerData(int section, Qt::Orientation orientation, int role) const
{
    if( (Qt::DisplayRole == role) &&
        (Qt::Horizontal == orientation))
    {
        if (section==0)
        {
            return QVariant("Property");
        }
        else if (section==1)
        {
            return QVariant("Value");
        }
    }
    return QVariant();
}

bool
QMAPAlgorithmModel::
setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid() || (m_MetaProperties.size() <= index.row()) || (1 != index.column()))
    {
        return false;
    }

    if(Qt::EditRole == role)
    {
        map::algorithm::MetaPropertyInfo* pInfo = m_MetaProperties[index.row()];

        bool result = SetPropertyValue(pInfo,value);

        if (result)
        {
            emit beginResetModel();
            this->UpdateMetaProperties();
            emit endResetModel();
        }

        return result;
    }

    return false;
};


void
QMAPAlgorithmModel::
UpdateMetaProperties() const
{
    if(m_pMetaInterface)
    {
        m_MetaProperties = m_pMetaInterface->getPropertyInfos();
    }
    else
    {
        m_MetaProperties.clear();
    }
}

QVariant
QMAPAlgorithmModel::
GetPropertyValue(const map::algorithm::MetaPropertyInfo* pInfo, int role) const
{
    if(!m_pMetaInterface)
    {
           return QVariant();
    }

    map::algorithm::facet::MetaPropertyAlgorithmInterface::MetaPropertyPointer prop = m_pMetaInterface->getProperty(pInfo);

    if(!prop)
    {
           return QVariant();
    }

    QVariant result;

    if (role == Qt::DisplayRole)
    {
        result = QVariant(QString("Error. Cannot unwrap MetaProperty. Name: ")+QString::fromStdString(pInfo->getName()));
    }

    if (prop->getMetaPropertyTypeInfo()==typeid(bool))
    {
        bool val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(val);
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(int))
    {
        int val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(val);
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(unsigned int))
    {
        unsigned int val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(val);
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(long))
    {
        long val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(qlonglong(val));
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(unsigned long))
    {
        unsigned long val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(qulonglong(val));
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(float))
    {
        float val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(val);
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(double))
    {
        double val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(val);
        }
    }
    else if (prop->getMetaPropertyTypeInfo()==typeid(map::core::String))
    {
        map::core::String val;
        if (map::core::unwrapCastedMetaProperty(prop,val))
        {
            result = QVariant(QString::fromStdString(val));
        }
    }
    else
    {
        if (role == Qt::DisplayRole)
        {
          result = QVariant(QString("Error. Cannot offer MetaProperty because of unsupported type. Property name: ")+QString::fromStdString(pInfo->getName())+QString("; type name: ")+QString(prop->getMetaPropertyTypeName()));
        }
    }
    return result;
};

template <typename TValueType>
bool
QMAPAlgorithmModel::
CheckCastAndSetProp(const map::algorithm::MetaPropertyInfo* pInfo, const QVariant& value)
{
    bool result = false;
    if (pInfo->getTypeInfo()==typeid(TValueType) && value.canConvert<TValueType>())
    {
        /**@TODO: Not save, because canConvert may say true but the actual value is not really convertible (e.g. string to int for the value "a")*/
        TValueType val = value.value<TValueType>();
        map::core::MetaPropertyBase::Pointer spMetaProp = map::core::MetaProperty<TValueType>::New(val).GetPointer();

        result = m_pMetaInterface->setProperty(pInfo,spMetaProp);
    }
    return result;
};

bool
QMAPAlgorithmModel::
SetPropertyValue(const map::algorithm::MetaPropertyInfo* pInfo, const QVariant& value)
{
    if(!m_pMetaInterface)
    {
           return false;
    }

    bool result = CheckCastAndSetProp<bool>(pInfo,value);

    if (!result) result = CheckCastAndSetProp<int>(pInfo,value);
    if (!result) result = CheckCastAndSetProp<unsigned int>(pInfo,value);
    if (!result) result = CheckCastAndSetProp<long>(pInfo,value);
    if (!result) result = CheckCastAndSetProp<unsigned long>(pInfo,value);
    if (!result) result = CheckCastAndSetProp<float>(pInfo,value);
    if (!result) result = CheckCastAndSetProp<double>(pInfo,value);
    if (!result  && pInfo->getTypeInfo()==typeid(map::core::String))
    {
        map::core::String val = value.toString().toStdString();
        map::core::MetaPropertyBase::Pointer spMetaProp = map::core::MetaProperty<map::core::String>::New(val).GetPointer();

        result = m_pMetaInterface->setProperty(pInfo,spMetaProp);
    };

    return result;
};

}
