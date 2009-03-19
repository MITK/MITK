/*=========================================================================

Program:   MITK
Language:  C++
Date:      $Date$
Version:   $Revision: 14921 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPropertiesTableModel.h"

#include "mitkStringProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkEnumerationProperty.h"
#include "mitkRenderingManager.h"
#include "mitkPropertyManager.h"

#include "itkCommand.h"

#include <QColor>
#include <QStringList>

QmitkPropertiesTableModel::QmitkPropertiesTableModel(mitk::PropertyList* _PropertyList, QObject* parent)
: QAbstractTableModel(parent)
, m_PropertyList(0)
, m_PropertyListModifiedObserverTag(0)
, m_BlockEvents(false)
, m_SortDescending(false)
{
  this->setPropertyList(_PropertyList);
}

QmitkPropertiesTableModel::~QmitkPropertiesTableModel()
{
  // remove all event listeners
  setPropertyList(0);
}

void QmitkPropertiesTableModel::setPropertyList( mitk::PropertyList* _PropertyList )
{
  // if node is 0 the property list is also 0 => no items in this model
  // otherwise retrieve list from the node
  //mitk::PropertyList* _PropertyList = (_PropertyList.IsNotNull()) ? _PropertyList->GetPropertyList(): 0;

  if(m_PropertyList != 0)
  {
    // remove all property event listener
    for(mitk::PropertyList::PropertyMap::const_iterator it=m_PropertyList->GetMap()->begin()
      ; it!=m_PropertyList->GetMap()->end()
      ; it++)
    {
      it->second.first->RemoveObserver(m_PropertyModifiedObserverTags[it->first]);
    }

    // unsubscribe from all modified events for old list
    m_PropertyList->RemoveObserver(m_PropertyListModifiedObserverTag);
  }

  // set new list
  m_PropertyList = _PropertyList;  
  // set all other members to default values
  m_PropertyListElements.clear();
  m_PropertyModifiedObserverTags.clear();
  m_PropertyDeleteObserverTags.clear();
  m_PropertyListModifiedObserverTag = 0;
  m_PropertyListDeleteObserverTag = 0;
  //m_SortDescending = false;

  if(m_PropertyList != 0)
  {
    // subscribe for all modified events for new list
    itk::MemberCommand<QmitkPropertiesTableModel>::Pointer propertyListModifiedCommand =
      itk::MemberCommand<QmitkPropertiesTableModel>::New();
    propertyListModifiedCommand->SetCallbackFunction(this, &QmitkPropertiesTableModel::PropertyListModified);
    m_PropertyListModifiedObserverTag = m_PropertyList->AddObserver(itk::ModifiedEvent(), propertyListModifiedCommand);

    itk::MemberCommand<QmitkPropertiesTableModel>::Pointer propertyDeleteCommand =
      itk::MemberCommand<QmitkPropertiesTableModel>::New();
    propertyDeleteCommand->SetCallbackFunction(this, &QmitkPropertiesTableModel::PropertyListDelete);
    m_PropertyListDeleteObserverTag = m_PropertyList->AddObserver(itk::DeleteEvent(), propertyDeleteCommand);


    // subscribe for all properties modified events
    for(mitk::PropertyList::PropertyMap::const_iterator it=m_PropertyList->GetMap()->begin()
      ; it!=m_PropertyList->GetMap()->end()
      ; it++)
    {
      // add relevant property column values
      m_PropertyListElements.push_back((*it));

      // subscribe for property change events
      itk::MemberCommand<QmitkPropertiesTableModel>::Pointer propertyModifiedCommand =
         itk::MemberCommand<QmitkPropertiesTableModel>::New();
      propertyModifiedCommand->SetCallbackFunction(this, &QmitkPropertiesTableModel::PropertyModified);
 
       m_PropertyModifiedObserverTags[it->first] = it->second.first->AddObserver(itk::ModifiedEvent(), propertyModifiedCommand);
    }      
    // sort the list
    this->sort(m_SortDescending);
  }
  
  // model was resetted
  QAbstractTableModel::reset();

}

Qt::ItemFlags QmitkPropertiesTableModel::flags(const QModelIndex& index) const
{
  // no editing so far, return default (enabled, selectable)
  Qt::ItemFlags flags = QAbstractItemModel::flags(index);

  if (index.column() == 1)
  {
    // there are also read only property items -> do not allow editing them
    if(index.data(Qt::EditRole).isValid())
      flags |= Qt::ItemIsEditable;
  }

  if (index.column() == 2)
  {
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

QVariant QmitkPropertiesTableModel::data(const QModelIndex& index, int role) const
{
  // empty data by default
  QVariant data;

  if(!index.isValid() || m_PropertyListElements.empty() || index.row() > (m_PropertyListElements.size()-1))
    return data;

  // the properties name
  if(index.column() == 0)
  {
    if(role == Qt::DisplayRole)
      data = QString::fromStdString(m_PropertyListElements[index.row()].first);
  }
  // the real properties value
  else if(index.column() == 1)
  {
    mitk::BaseProperty* baseProp = m_PropertyListElements[index.row()].second.first;

    if (const mitk::ColorProperty* colorProp 
      = dynamic_cast<const mitk::ColorProperty*>(baseProp))
    {
      mitk::Color col = colorProp->GetColor();
      QColor qcol((int)(col.GetRed() * 255), (int)(col.GetGreen() * 255),(int)( col.GetBlue() * 255));
      if(role == Qt::DisplayRole)
        data.setValue<QColor>(qcol);
      else if(role == Qt::EditRole)
        data.setValue<QColor>(qcol);
    }

    else if(mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(baseProp))
    {
      if(role == Qt::DisplayRole)
        data.setValue<bool>( boolProp->GetValue() );
      else if(role == Qt::EditRole)
        data.setValue<bool>( boolProp->GetValue() );
    }

    else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
    {
      if(role == Qt::DisplayRole)
        data.setValue<QString>(QString::fromStdString(stringProp->GetValue()));
      else if(role == Qt::EditRole)
        data.setValue<QString>(QString::fromStdString(stringProp->GetValue()));

    }

    else if (mitk::IntProperty* intProp = dynamic_cast<mitk::IntProperty*>(baseProp))
    {
      if(role == Qt::DisplayRole)
        data.setValue<int>(intProp->GetValue());
      else if(role == Qt::EditRole)
        data.setValue<int>(intProp->GetValue());
    }

    else if (mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(baseProp))
    {
      if(role == Qt::DisplayRole)
        data.setValue<float>(floatProp->GetValue());
      else if(role == Qt::EditRole)
        data.setValue<float>(floatProp->GetValue());

    }

    else if (mitk::EnumerationProperty* enumerationProp = dynamic_cast<mitk::EnumerationProperty*>(baseProp))
    {
      if(role == Qt::DisplayRole)
        data.setValue<QString>(QString::fromStdString(baseProp->GetValueAsString()));
      else if(role == Qt::EditRole)
      {
        QStringList values;
        for(mitk::EnumerationProperty::EnumConstIterator it=enumerationProp->Begin(); it!=enumerationProp->End()
          ; it++)
        {
          values << QString::fromStdString(it->second);
        }
        data.setValue<QStringList>(values);
      }
    }

    else
    {
      if(role == Qt::DisplayRole)
        data.setValue<QString>(QString::fromStdString(m_PropertyListElements[index.row()].second.first->GetValueAsString()));
    }
  }

  // enabled/disabled value
  else if(index.column() == 2)
  {
    if(role == Qt::DisplayRole)
      data.setValue<bool>(m_PropertyListElements[index.row()].second.second);

    else if(role == Qt::EditRole)
      data.setValue<bool>(m_PropertyListElements[index.row()].second.second);
  }

  return data;
}

bool QmitkPropertiesTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (index.isValid() && !m_PropertyListElements.empty() && index.row() < m_PropertyListElements.size() && role == Qt::EditRole) 
  {
    // block all events now!
    m_BlockEvents = true;

    // the properties name
    if(index.column() == 1)
    {
      mitk::BaseProperty* baseProp = m_PropertyListElements[index.row()].second.first;

      if (mitk::ColorProperty* colorProp 
        = dynamic_cast<mitk::ColorProperty*>(baseProp))
      {
        QColor qcolor = value.value<QColor>();
        if(!qcolor.isValid())
          return false;

        mitk::Color col = colorProp->GetColor();
        col.SetRed(qcolor.red() / 255.0);
        col.SetGreen(qcolor.green() / 255.0);
        col.SetBlue(qcolor.blue() / 255.0);
        colorProp->SetColor(col);
        m_PropertyList->InvokeEvent(itk::ModifiedEvent());
        m_PropertyList->Modified();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }

      else if(mitk::BoolProperty* boolProp = dynamic_cast<mitk::BoolProperty*>(baseProp))
      {
        boolProp->SetValue(value.value<bool>());
        m_PropertyList->InvokeEvent(itk::ModifiedEvent());
        m_PropertyList->Modified();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }

      else if (mitk::StringProperty* stringProp = dynamic_cast<mitk::StringProperty*>(baseProp))
      {
        stringProp->SetValue((value.value<QString>()).toStdString());
        m_PropertyList->InvokeEvent(itk::ModifiedEvent());
        m_PropertyList->Modified();

        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      }

      else if (mitk::IntProperty* intProp = dynamic_cast<mitk::IntProperty*>(baseProp))
      {
        int intValue = value.value<int>();
        if (intValue != intProp->GetValue())
        {
          intProp->SetValue(intValue);
          m_PropertyList->InvokeEvent(itk::ModifiedEvent());
          m_PropertyList->Modified();

          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      }

      else if (mitk::FloatProperty* floatProp = dynamic_cast<mitk::FloatProperty*>(baseProp))
      {
        float floatValue = value.value<float>();
        if (floatValue != floatProp->GetValue())
        {
          floatProp->SetValue(floatValue);
          m_PropertyList->InvokeEvent(itk::ModifiedEvent());
          m_PropertyList->Modified();
          
          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      }

      else if (mitk::EnumerationProperty* enumerationProp = dynamic_cast<mitk::EnumerationProperty*>(baseProp))
      {
        std::string activatedItem = value.value<QString>().toStdString();
        if ( activatedItem != enumerationProp->GetValueAsString() )
        {
          if ( enumerationProp->IsValidEnumerationValue( activatedItem ) )
          {
            enumerationProp->SetValue( activatedItem );
            m_PropertyList->InvokeEvent( itk::ModifiedEvent() );
            m_PropertyList->Modified();

            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
          }
        }
      }
    }

    // enabled/disabled value
    else if(index.column() == 2)
    {
      bool active = value.value<bool>();
      std::string propertyName = m_PropertyListElements[index.row()].first;

      m_PropertyList->SetEnabled(propertyName, active);
	  m_PropertyListElements[index.row()].second.second = active;

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    // property was changed by us, now we can accept property changes triggered by someone else 
    m_BlockEvents = false;
    emit dataChanged(index, index);
    return true;
  }

  return false;
}

QVariant QmitkPropertiesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) 
    {
     case 0:
       return tr("Name");

     case 1:
       return tr("Value");

     case 2:
       return tr("Active");

     default:
       return QVariant();
    }
  }

  return QVariant();
}

int QmitkPropertiesTableModel::rowCount(const QModelIndex& parent) const
{
  // return the number of properties in the properties list.
  return m_PropertyListElements.size();
}

int QmitkPropertiesTableModel::columnCount(const QModelIndex &parent)const
{
  return 3;
}

void QmitkPropertiesTableModel::PropertyModified( const itk::Object *caller, const itk::EventObject &event )
{
  if(!m_BlockEvents)
  {
    int row = this->FindProperty(dynamic_cast<const mitk::BaseProperty*>(caller));

    QModelIndex indexOfChangedProperty = index(row, 1);

    emit dataChanged(indexOfChangedProperty, indexOfChangedProperty);
  }
}

void QmitkPropertiesTableModel::PropertyListModified( const itk::Object *caller, const itk::EventObject &event )
{
  if(!m_BlockEvents)
  {
    //const mitk::PropertyList* modifiedPropertyList = dynamic_cast<const mitk::PropertyList*>(caller);
    // reset
    this->setPropertyList(m_PropertyList);
  }
}

void QmitkPropertiesTableModel::sort( int column, Qt::SortOrder order /*= Qt::AscendingOrder */ )
{
  bool sortDescending = (order == Qt::DescendingOrder) ? true: false;

  // do not sort twice !!! (dont know why, but qt calls this func twice. STUPID!)
  if(sortDescending != m_SortDescending)
  {
    m_SortDescending = sortDescending;

    PropertyListElementCompareFunction::CompareCriteria _CompareCriteria 
      = PropertyListElementCompareFunction::CompareByName;

    PropertyListElementCompareFunction::CompareOperator _CompareOperator
      = m_SortDescending ? PropertyListElementCompareFunction::Greater: PropertyListElementCompareFunction::Less;

    if(column == 1)
      _CompareCriteria = PropertyListElementCompareFunction::CompareByValue;

    else if(column == 2)
      _CompareCriteria = PropertyListElementCompareFunction::CompareByValue;


    PropertyListElementCompareFunction compareFunc(_CompareCriteria, _CompareOperator);
    std::sort(m_PropertyListElements.begin(), m_PropertyListElements.end(), compareFunc);

    QAbstractTableModel::reset();

  }
}

void QmitkPropertiesTableModel::PropertyListDelete( const itk::Object *caller, const itk::EventObject &event )
{
  if(!m_BlockEvents)
  {
    int row = this->FindProperty(dynamic_cast<const mitk::BaseProperty*>(caller));

    // reset the whole model again. may be changed to emit rowsRemoved signal.
    if(row >= 0)
      this->setPropertyList(m_PropertyList);
  }
}

void QmitkPropertiesTableModel::PropertyDelete( const itk::Object *caller, const itk::EventObject &event )
{

}

int QmitkPropertiesTableModel::FindProperty( const mitk::BaseProperty* _Property )
{
  int row = -1;

  if(_Property)
  {
    // search for property that changed and emit datachanged on the corresponding ModelIndex
    std::vector<std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> > >::iterator propertyIterator;

    for( propertyIterator=m_PropertyListElements.begin(); propertyIterator!=m_PropertyListElements.end()
      ; propertyIterator++)
    {
      if(propertyIterator->second.first == _Property)
        break;
    }

    if(propertyIterator != m_PropertyListElements.end())
      row = std::distance(m_PropertyListElements.begin(), propertyIterator);
  }

  return row;
}

bool QmitkPropertiesTableModel::PropertyListElementCompareFunction::operator()
  ( const std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> >& _Left
  , const std::pair<std::string,std::pair<mitk::BaseProperty::Pointer,bool> >& _Right ) const
{
  switch(m_CompareCriteria)
  {
    case CompareByValue:
      if(m_CompareOperator == Less)
        return (_Left.second.first->GetValueAsString() < _Right.second.first->GetValueAsString());
      else
        return (_Left.second.first->GetValueAsString() > _Right.second.first->GetValueAsString());
    break;

    case CompareByActivity:
      if(m_CompareOperator == Less)
        return (_Left.second.second < _Right.second.second);
      else
        return (_Left.second.second > _Right.second.second);
    break;

    // CompareByName:
    default:
      if(m_CompareOperator == Less)
        return (_Left.first < _Right.first);
      else
      return (_Left.first > _Right.first);
    break;
  }
}

QmitkPropertiesTableModel::PropertyListElementCompareFunction::PropertyListElementCompareFunction( CompareCriteria _CompareCriteria
                                                                                  , CompareOperator _CompareOperator )
: m_CompareCriteria(_CompareCriteria)
, m_CompareOperator(_CompareOperator)
{
  
}