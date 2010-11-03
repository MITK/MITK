/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkCorrespondingPointSetsModel.h"
#include <itkCommand.h>
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkRenderingManager.h"
#include <mitkPointSetInteractor.h>
#include <mitkEvent.h>
#include <mitkStateEvent.h>
#include <mitkInteractionConst.h>

QmitkCorrespondingPointSetsModel::QmitkCorrespondingPointSetsModel( int t, QObject* parent )
:QAbstractTableModel(parent),
m_PointSetNode(NULL),
m_ReferencePointSetNode(NULL),
m_TimeStep(t),
m_row(-1),
m_column(-1)
{
  /*this->m_PointSetNode->SetProperty("layer", mitk::IntProperty::New(50));
  this->m_ReferencePointSetNode->SetProperty("layer", mitk::IntProperty::New(50));*/
}

Qt::ItemFlags QmitkCorrespondingPointSetsModel::flags(const QModelIndex& /*index*/) const
{
  // no editing so far, return default (enabled, selectable)
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QmitkCorrespondingPointSetsModel::~QmitkCorrespondingPointSetsModel()
{
  ;
}

void QmitkCorrespondingPointSetsModel::SetPointSetNodes( std::vector<mitk::DataNode*> nodes )
{
  if ( nodes.size() > 1 ) {
    m_PointSetNode = nodes.front();
    m_ReferencePointSetNode = nodes.back();
  }
  else if ( nodes.size() == 1 )
  {
    m_PointSetNode = nodes.front();
    m_ReferencePointSetNode = NULL;
  }
  else
  {
    m_PointSetNode = NULL;
    m_ReferencePointSetNode = NULL;
  }
  QAbstractTableModel::reset();
  //emit SignalUpdateSelection();
}

void QmitkCorrespondingPointSetsModel::SetTimeStep(int t)
{
  m_TimeStep = t;
  QAbstractTableModel::reset();
  emit SignalUpdateSelection();
}

int QmitkCorrespondingPointSetsModel::GetTimeStep() const
{
  return m_TimeStep;
}

int QmitkCorrespondingPointSetsModel::rowCount( const QModelIndex&  /*parent*/ ) const
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  mitk::PointSet::Pointer referencePointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);
  int sizePS = 0;
  int sizeRPS = 0;
  if ( pointSet.IsNotNull() )
  {
    sizePS = pointSet->GetSize(m_TimeStep);
  }
  if ( referencePointSet.IsNotNull() )
  {
    sizeRPS = referencePointSet->GetSize(m_TimeStep);
  }
  if ( sizePS > sizeRPS )
    return sizePS;
  return sizeRPS;
}

int QmitkCorrespondingPointSetsModel::columnCount( const QModelIndex&  /*parent*/ ) const
{
  return 2;
}

QVariant QmitkCorrespondingPointSetsModel::data(const QModelIndex& index, int role) const
{
  mitk::PointSet::Pointer pointSet = NULL;
  if ( index.column() == 0 )
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if ( index.column() == 1 )
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if ( pointSet.IsNull() )
  {
    return QVariant();
  }

  if ( !index.isValid() )
  {
    return QVariant();
  }

  if ( index.row() >= pointSet->GetSize(m_TimeStep) )
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    mitk::PointSet::PointsContainer::ElementIdentifier id;
    mitk::PointSet::PointType p;
    bool pointFound = this->GetPointForModelIndex(index, p, id);
    if (pointFound == false)
      return QVariant();

    QString s = "";
    bool firstProp = true;

    if (this->QTPropIdsEnabled())
    {
      s.append(QString("%0").arg( id, 3));
      firstProp = false;
    }
    if (this->QTPropCoordinatesEnabled())
    {
      if(!firstProp)
        s.append(QString(": "));
      s.append(QString("(%0, %1, %2)")
      .arg( p[0], 0, 'f', 2 )
      .arg( p[1], 0, 'f', 2 )
      .arg( p[2], 0, 'f', 2 ));
    }
    return QVariant(s);
  }
  else
  {
    return QVariant();
  }
}

QVariant QmitkCorrespondingPointSetsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
  {
    return QVariant();
  }

  if (orientation == Qt::Horizontal)
  {
    //return QString("Coordinates").arg(section);
    if (section == 0)
    {
      if ( m_PointSetNode )
        return QString::fromStdString(this->m_PointSetNode->GetName());
    }
    else if (section == 1)
    {
      if ( m_ReferencePointSetNode )
        return QString::fromStdString(this->m_ReferencePointSetNode->GetName());
    }
    return QString();
  }
  return QString("%1").arg(section);
}

bool QmitkCorrespondingPointSetsModel::GetPointForModelIndex( const QModelIndex &index, mitk::PointSet::PointType& p, 
                                                             mitk::PointSet::PointIdentifier& id) const
{
  mitk::PointSet::Pointer pointSet = NULL;
  if ( index.column() == 0 )
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if ( index.column() == 1 )
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if (pointSet.IsNull())
    return false;

  if ((index.row() < 0) || (index.row() >= (int)pointSet->GetPointSet(m_TimeStep)->GetPoints()->Size()))
    return false;

  // get the nth. element, if it exists.
  // we can not use the index directly, because PointSet uses a map container, 
  // where the index is not necessarily the same as the key.
  // Therefore we have to count the elements
  mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_TimeStep)->GetPoints()->Begin();
  for (int i = 0; i < index.row(); ++i)
  {
    ++it;
    if (it == pointSet->GetPointSet(m_TimeStep)->GetPoints()->End())
      return false;
  }

  if (it != pointSet->GetPointSet(m_TimeStep)->GetPoints()->End()) // not at the end, 
  {
    p = it->Value();
    id = it->Index();
    return true;
  }

  return false;
}
bool QmitkCorrespondingPointSetsModel::GetPointForModelIndex( int row, int column, mitk::PointSet::PointType& p, 
                                                             mitk::PointSet::PointIdentifier& id) const
{
  mitk::PointSet::Pointer pointSet = NULL;
  if (column == 0 )
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if ( column == 1 )
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if (pointSet.IsNull())
    return false;

  if ((row < 0) || (row >= (int)pointSet->GetPointSet(m_TimeStep)->GetPoints()->Size()))
    return false;

  // get the nth. element, if it exists.
  // we can not use the index directly, because PointSet uses a map container, 
  // where the index is not necessarily the same as the key.
  // Therefore we have to count the elements
  mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_TimeStep)->GetPoints()->Begin();
  for (int i = 0; i < row; ++i)
  {
    ++it;
    if (it == pointSet->GetPointSet(m_TimeStep)->GetPoints()->End())
    return false;
  }

  if (it != pointSet->GetPointSet(m_TimeStep)->GetPoints()->End()) // not at the end, 
  {
    p = it->Value();
    id = it->Index();
    return true;
  }

  return false;
}

bool QmitkCorrespondingPointSetsModel::GetModelIndexForPointID(mitk::PointSet::PointIdentifier id, QModelIndex& index, int column) const
{
  mitk::PointSet::Pointer pointSet = NULL;
  if (column == 0)
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if (column == 1)
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if (!pointSet.IsNull())
  {
    mitk::PointSet::PointsContainer::Pointer points = pointSet->GetPointSet(m_TimeStep)->GetPoints();

    if (!points->IndexExists(id))
      return false;

    unsigned int idx = 0;
    for (mitk::PointSet::PointsContainer::Iterator it = points->Begin(); it != points->End(); ++it)
    {
      if (it->Index() == id) // we found the correct element
      {
        index = this->index(idx, column);
        return true;
      }

      idx++;
    }
  }
  return false; // nothing found
}

void QmitkCorrespondingPointSetsModel::MoveSelectedPointUp()
{
  mitk::DataNode* dataNode = NULL; 

  if (this->m_column == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_column == 1)
    dataNode = this->m_ReferencePointSetNode;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID;   
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::PointSet::PointType point = pointSet->GetPoint(selectedID, m_TimeStep);
  mitk::ScalarType tsInMS = pointSet->GetTimeSlicedGeometry()->TimeStepToMS(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP,tsInMS, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}

void QmitkCorrespondingPointSetsModel::MoveSelectedPointDown()
{
  mitk::DataNode* dataNode = NULL; 

  if (this->m_column == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_column == 1)
    dataNode = this->m_ReferencePointSetNode;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return;

  mitk::PointSet::PointIdentifier selectedID; 
  selectedID = pointSet->SearchSelectedPoint(m_TimeStep);
  mitk::ScalarType tsInMS = pointSet->GetTimeSlicedGeometry()->TimeStepToMS(m_TimeStep);
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, tsInMS, pointSet->GetPoint(selectedID, m_TimeStep), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in Pointset/Mapper
}


void QmitkCorrespondingPointSetsModel::RemoveSelectedPoint()
{
  mitk::DataNode* dataNode = NULL; 

  if (this->m_column == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_column == 1)
    dataNode = this->m_ReferencePointSetNode;

  if (dataNode == NULL)
    return;

  this->SetSelectedIndex(-1, this->m_column);

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return;

  //get corresponding interactor to PointSet
  mitk::PointSetInteractor::Pointer interactor = dynamic_cast<mitk::PointSetInteractor*>(dataNode->GetInteractor());  
  if (interactor.IsNull())
  {
    if (dataNode->GetInteractor()==NULL && dataNode != NULL) //no Interactor set to node
    {
      interactor = mitk::PointSetInteractor::New("pointsetinteractor",dataNode);
      dataNode->SetInteractor(interactor);
    }
    else
    {
      MITK_WARN<<"Unexpected interactor found!\n";
      return;
    }
  }

  //send a DEL event to pointsetinteractor
  const mitk::Event* delEvent = new mitk::Event(NULL, mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
  mitk::StateEvent* delStateEvent = new mitk::StateEvent(mitk::EIDDELETE, delEvent);
  interactor->HandleEvent(delStateEvent);
  delete delEvent;
  delete delStateEvent;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // Workaround for update problem in PointSet/Mapper
}

mitk::PointSet* QmitkCorrespondingPointSetsModel::CheckForPointSetInNode(mitk::DataNode* node) const
{
  if (node != NULL)
  {
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());
    if (pointSet.IsNotNull())
      return pointSet;
  }
  return NULL;
}

bool QmitkCorrespondingPointSetsModel::QTPropCoordinatesEnabled() const
{
  return this->QTPropShowCoordinates;
}

void QmitkCorrespondingPointSetsModel::QTPropSetCoordinatesEnabled(bool showCoordinates)
{
  this->QTPropShowCoordinates = showCoordinates;
}

bool QmitkCorrespondingPointSetsModel::QTPropIdsEnabled() const
{
  return this->QTPropShowIds;
}

void QmitkCorrespondingPointSetsModel::QTPropSetIdsEnabled(bool showIds)
{
  this->QTPropShowIds = showIds;
}
std::vector<mitk::DataNode*> QmitkCorrespondingPointSetsModel::GetPointSetNodes(){
  std::vector<mitk::DataNode*> pointSetNodes;
  if ( this->m_PointSetNode )
    pointSetNodes.push_back(this->m_PointSetNode);
  if ( this->m_ReferencePointSetNode )
    pointSetNodes.push_back(this->m_ReferencePointSetNode);
  return pointSetNodes;
}
  
void QmitkCorrespondingPointSetsModel::SetSelectedIndex(int row, int column)
{
  this->m_row = row;
  this->m_column = column;
}

void QmitkCorrespondingPointSetsModel::NodeRemoved( mitk::DataNode::ConstPointer removedNode)
{
  ;
}
void QmitkCorrespondingPointSetsModel::ClearSelectedPointSet()
{
  mitk::DataNode* dataNode = NULL; 

  if (this->m_column == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_column == 1)
    dataNode = this->m_ReferencePointSetNode;

  if (dataNode == NULL)
    return;

  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(dataNode->GetData());

  mitk::PointSet::PointsContainer::Iterator it;
  for (int t=0; t<pointSet->GetTimeSteps(); t++){
    int bla = pointSet->GetSize();
    while( !pointSet->IsEmpty(t) )
    {
      it = pointSet->GetPointSet(t)->GetPoints()->Begin();
      pointSet->SetSelectInfo(it->Index(),true, t);
      this->RemoveSelectedPoint();
    }
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
void QmitkCorrespondingPointSetsModel::ClearCurrentTimeStep()
{
  mitk::DataNode* dataNode = NULL; 

  if (this->m_column == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_column == 1)
    dataNode = this->m_ReferencePointSetNode;

  if (dataNode == NULL)
    return;

  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(dataNode->GetData());

  mitk::PointSet::PointsContainer::Iterator it;
  while( !pointSet->IsEmpty(m_TimeStep) )
  {
    it = pointSet->GetPointSet(m_TimeStep)->GetPoints()->Begin();
    pointSet->SetSelectInfo(it->Index(),true, m_TimeStep);
    this->RemoveSelectedPoint();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
