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

#include "QmitkUSNavigationCalibrationsDataModel.h"

#include <mitkUSCombinedModality.h>
#include <mitkUSControlInterfaceBMode.h>

#include <QBrush>
#include <QColor>
#include <QIcon>
#include <QFont>

QmitkUSNavigationCalibrationsDataModel::QmitkUSNavigationCalibrationsDataModel(QObject *parent)
   : QAbstractTableModel(parent),
     m_ListenerDeviceChanged(this, &QmitkUSNavigationCalibrationsDataModel::OnDeviceChanged)
 {
 }

 QmitkUSNavigationCalibrationsDataModel::~QmitkUSNavigationCalibrationsDataModel()
 {
   if ( m_CombinedModality.IsNotNull() )
   {
     m_CombinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
   }
 }

 void QmitkUSNavigationCalibrationsDataModel::SetCombinedModality(mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality)
 {
   if ( m_CombinedModality.IsNotNull() && m_CombinedModality->GetUltrasoundDevice().IsNotNull() )
   {
     m_CombinedModality->GetUltrasoundDevice()->RemovePropertyChangedListener(m_ListenerDeviceChanged);
   }

   m_CombinedModality = combinedModality;
   if ( m_CombinedModality.IsNotNull() )
   {
     m_ControlInterfaceBMode = m_CombinedModality->GetControlInterfaceBMode();

     // make sure that the combined modality is active as this may be
     // necessary to get the available depths
     if ( m_CombinedModality->GetUltrasoundDevice()->GetDeviceState() < mitk::USDevice::State_Connected ) { m_CombinedModality->GetUltrasoundDevice()->Connect(); }
     if ( m_CombinedModality->GetUltrasoundDevice()->GetDeviceState() == mitk::USDevice::State_Connected ) { m_CombinedModality->GetUltrasoundDevice()->Activate(); }

     if ( m_CombinedModality->GetUltrasoundDevice().IsNotNull() )
     {
       m_CombinedModality->GetUltrasoundDevice()->AddPropertyChangedListener(m_ListenerDeviceChanged);
     }
   }

   // as the combined modality was changed, an old table model
   // would not be valid anymore
   this->beginResetModel();
   this->endResetModel();
 }

 void QmitkUSNavigationCalibrationsDataModel::OnDeviceChanged(const std::string&, const std::string&)
 {
    this->beginResetModel();
    this->endResetModel();
 }

 /** \brief Return number of rows of the model. */
 int QmitkUSNavigationCalibrationsDataModel::rowCount ( const QModelIndex & ) const
 {
   if ( m_ControlInterfaceBMode.IsNull() )
   {
     return 1; // only one default depth can be assumed
   }
   else
   {
     return m_ControlInterfaceBMode->GetScanningDepthValues().size();
   }
 }

 /** \brief Return number of columns (3) of the model. */
 int QmitkUSNavigationCalibrationsDataModel::columnCount ( const QModelIndex & ) const
 {
   return 3;
 }

 /** \brief Return names for the columns, numbers for the rows and invalid for DisplayRole. */
 QVariant QmitkUSNavigationCalibrationsDataModel::headerData ( int section, Qt::Orientation orientation, int role ) const
 {
   if ( role != Qt::DisplayRole ) { return QVariant(QVariant::Invalid); }

   if ( orientation == Qt::Horizontal )
   {
     switch ( section )
     {
     case 0: return QVariant("Depth");
     case 1: return QVariant("Calibrated");
     case 2: return QVariant("");
     }
   }

   return QVariant(QVariant::Invalid);
 }

 /** \brief Return selectable and enabled for column 1 (size); selectable, enabled and editable for every other column. */
 Qt::ItemFlags QmitkUSNavigationCalibrationsDataModel::flags ( const QModelIndex & ) const
 {
   return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
 }

 /** \brief Return model data of the selected cell. */
 QVariant QmitkUSNavigationCalibrationsDataModel::data ( const QModelIndex & index, int role ) const
 {
   if ( m_CombinedModality.IsNull() ) { return QVariant(QVariant::Invalid); }

   std::vector<double> scanningDepthValues = m_ControlInterfaceBMode.IsNull() ?
     std::vector<double>(1,0) : m_ControlInterfaceBMode->GetScanningDepthValues();

   // make sure that row and column index fit data borders
   if (index.row() >= this->rowCount()
      || index.column() >= this->columnCount())
  {
    return QVariant(QVariant::Invalid);
  }

   double currentDepth = 0;
   if ( m_ControlInterfaceBMode.IsNotNull() ) { currentDepth = m_ControlInterfaceBMode->GetScanningDepth(); }

   bool isCalibratedForCurrentDepth =
     m_CombinedModality->GetCalibration(QString::number(scanningDepthValues.at(index.row())).toStdString()).IsNotNull();

  switch (role)
  {
  case Qt::BackgroundColorRole:
  {
    if ( isCalibratedForCurrentDepth )
    {
      return QVariant(QBrush(QColor(125, 255, 125)));
    }
    else
    {
      return QVariant(QBrush(QColor(255, 125, 125)));
    }

    break;
  }

  case Qt::FontRole:
  {
    if ( scanningDepthValues.at(index.row()) == currentDepth )
    {
      QFont qFont;
      qFont.setBold(true);
      return qFont;
    }
    else
    {
      return QVariant::Invalid;
    }
  }

  case Qt::DecorationRole:
  {
    if ( index.column() == 2 )
    {
      if ( isCalibratedForCurrentDepth )
      {
        return QIcon(":/USNavigation/process-stop.png");
      }
    }
    else { return QVariant::Invalid; }
  }

  case Qt::EditRole:
  case Qt::DisplayRole:
  {
    switch ( index.column() )
    {
    case 0:
    {
      return QVariant::fromValue<double>(scanningDepthValues.at(index.row()));
    }
    case 1:
    {
      if ( m_ControlInterfaceBMode.IsNull() )
      {
        // use the current zoom level (which is assumed to be the only one),
        // when no b mode controls are available
        return QVariant(m_CombinedModality->GetCalibration().IsNotNull());
      }
      else
      {
        return QVariant(isCalibratedForCurrentDepth);
      }
    }
    case 2:
    {
      return QVariant("");
    }
    }
    break;
  }

  case Qt::ToolTipRole:
  {
    if ( index.column() == 2 && isCalibratedForCurrentDepth )
    {
      return QVariant(QString("Remove calibration for depth ") + QString::number(scanningDepthValues.at(index.row())) + " on mouse click.");
    }
    break;
  }
  }

  return QVariant(QVariant::Invalid);
 }

/** \brief Set model data for the selected cell. */
bool QmitkUSNavigationCalibrationsDataModel::setData ( const QModelIndex & index, const QVariant & value, int )
{
  if ( m_CombinedModality.IsNull() || index.column() != 2 || value != false )
    return false;

  if ( m_ControlInterfaceBMode.IsNull() )
  {
    m_CombinedModality->RemoveCalibration();
  }
  else
  {
    m_CombinedModality->RemoveCalibration(QString::number(m_ControlInterfaceBMode->GetScanningDepthValues().at(index.row())).toStdString());
  }

  emit dataChanged(this->index(index.row(), 0), this->index(index.row(), 1));
  return true;
}

/** \brief Remove given rows from the model.
*  \param removeFromDataStorage zone nodes are removed from the data storage too, if this is set to true
*/
bool QmitkUSNavigationCalibrationsDataModel::removeRows ( int, int, const QModelIndex&, bool )
{
  return false;
}
