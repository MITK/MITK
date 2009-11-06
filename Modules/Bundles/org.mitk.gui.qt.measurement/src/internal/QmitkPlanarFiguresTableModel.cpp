#include "QmitkPlanarFiguresTableModel.h"
#include <mitkPlanarFigure.h>
#include <QmitkNodeDescriptorManager.h>

QmitkPlanarFiguresTableModel::QmitkPlanarFiguresTableModel( mitk::DataStorage::Pointer _DataStorage, mitk::NodePredicateBase* _Predicate, QObject* parent )
: QmitkDataStorageTableModel(_DataStorage, _Predicate, parent)
{

}

QmitkPlanarFiguresTableModel::~QmitkPlanarFiguresTableModel()
{

}

QVariant QmitkPlanarFiguresTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  QVariant headerData;

  // show only horizontal header
  if ( role == Qt::DisplayRole )
  {
    if( orientation == Qt::Horizontal )
    {
      // first column: "Name"
      if(section == 0)
        headerData = "Name";
      else if(section == 1)
        headerData = "Info";
    }
    else if( orientation == Qt::Vertical )
    {
      // show numbers for rows
      headerData = section+1;
    }
  }

  return headerData;
}

Qt::ItemFlags QmitkPlanarFiguresTableModel::flags( const QModelIndex &index ) const
{
  // name & visibility is editable
  if (index.column() == 0)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
  else
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

int QmitkPlanarFiguresTableModel::columnCount( const QModelIndex &parent ) const
{
  return 2;
}

QVariant QmitkPlanarFiguresTableModel::data( const QModelIndex &index, int role ) const
{
  QVariant data;
  if(!index.isValid())
    return data;
  mitk::DataTreeNode::Pointer node = this->m_NodeSet.at(index.row());

  if(!node)
    return data;
   
  if(index.column() == 0 && role == Qt::DecorationRole)
  {
    if(node)
    {
      QmitkNodeDescriptor* nodeDescriptor 
        = QmitkNodeDescriptorManager::GetInstance()->GetDescriptor(node);
      data = nodeDescriptor->GetIcon();
    }
  }
  else if(index.column() == 1)
  {
    if(role == Qt::DisplayRole || role == Qt::ToolTipRole)
    {
      QString info;
      mitk::PlanarFigure* planarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
      if(!planarFigure)
        return data;

      for(int i=0; i<planarFigure->GetNumberOfFeatures(); ++i)
      {
        info.append(QString("%1: %2 %3")
          .arg(QString(planarFigure->GetFeatureName(i)))
          .arg(planarFigure->GetQuantity(i), 0, 'g', 2)
          .arg(QString(planarFigure->GetFeatureUnit(i))));
        if((i+1) != planarFigure->GetNumberOfFeatures())
          info.append(", ");

        data = info;
      }
    }
  }
  else
    return QmitkDataStorageTableModel::data(index, role);

  return data;
}