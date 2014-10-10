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

#include "QmitkCorrespondingPointSetsModel.h"
#include <itkCommand.h>
#include "mitkInteractionConst.h"
#include "mitkPointOperation.h"
#include "mitkRenderingManager.h"
#include <mitkEvent.h>
#include <mitkStateEvent.h>
#include <mitkInteractionConst.h>
#include <mitkStepper.h>
#include <QStringListModel>
#include <mitkPointSetInteractor.h>
#include <mitkGlobalInteraction.h>

QmitkCorrespondingPointSetsModel::QmitkCorrespondingPointSetsModel(int, QObject* parent)
  : QAbstractTableModel(parent),
    m_PointSetNode(NULL),
    m_ReferencePointSetNode(NULL),
    m_TimeStepper(NULL),
    m_SelectedPointSetIndex(-1),
    m_Interactor(NULL),
    m_MultiWidget( NULL ),
    m_PointSetModifiedObserverTag(0),
    m_ReferencePointSetModifiedObserverTag(0)
{
}

Qt::ItemFlags QmitkCorrespondingPointSetsModel::flags(const QModelIndex& index) const
{
   if (index.isValid())
       return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
   else
       return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

Qt::DropActions QmitkCorrespondingPointSetsModel::supportedDropActions() const
{
   return Qt::CopyAction | Qt::MoveAction;
}

bool QmitkCorrespondingPointSetsModel::dropMimeData(const QMimeData*, Qt::DropAction action, int row, int, const QModelIndex &parent)
{
  if (action == Qt::IgnoreAction)
    return true;

  int targetRow;

  if (row != -1)
    targetRow = row;
  else if (parent.isValid())
    targetRow = parent.row();
  else
    targetRow = rowCount(QModelIndex());

  this->MoveSelectedPoint(targetRow);
  return true;
}

QmitkCorrespondingPointSetsModel::~QmitkCorrespondingPointSetsModel()
{
  ;
}

void QmitkCorrespondingPointSetsModel::RemoveObservers(){
  if (m_PointSetNode)
  {
    mitk::PointSet::Pointer oldPointSet = dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());
    if (oldPointSet.IsNotNull())
    {
      oldPointSet->RemoveObserver(m_PointSetModifiedObserverTag);
    }
  }
  if (m_ReferencePointSetNode)
  {
    mitk::PointSet::Pointer oldPointSet = dynamic_cast<mitk::PointSet*>(m_ReferencePointSetNode->GetData());
    if (oldPointSet.IsNotNull())
    {
      oldPointSet->RemoveObserver(m_ReferencePointSetModifiedObserverTag);
    }
  }
}

void QmitkCorrespondingPointSetsModel::AddObservers()
{
  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  if ( pointSet.IsNotNull())
  {
    // add new observer for modified if necessary
    itk::ReceptorMemberCommand<QmitkCorrespondingPointSetsModel>::Pointer modCommand = itk::ReceptorMemberCommand<QmitkCorrespondingPointSetsModel>::New();
    modCommand->SetCallbackFunction( this, &QmitkCorrespondingPointSetsModel::OnPointSetChanged );
    m_PointSetModifiedObserverTag = pointSet->AddObserver( itk::ModifiedEvent(), modCommand );
  }
  else
  {
    m_PointSetModifiedObserverTag = 0;
  }

  pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);
  if ( pointSet.IsNotNull())
  {
    // add new observer for modified if necessary
    itk::ReceptorMemberCommand<QmitkCorrespondingPointSetsModel>::Pointer modCommand = itk::ReceptorMemberCommand<QmitkCorrespondingPointSetsModel>::New();
    modCommand->SetCallbackFunction( this, &QmitkCorrespondingPointSetsModel::OnPointSetChanged );
    m_ReferencePointSetModifiedObserverTag = pointSet->AddObserver( itk::ModifiedEvent(), modCommand );
  }
  else
  {
    m_ReferencePointSetModifiedObserverTag = 0;
  }
}

void QmitkCorrespondingPointSetsModel::OnPointSetChanged(const itk::EventObject&)
{
  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
}
void QmitkCorrespondingPointSetsModel::SetPointSetNodes( std::vector<mitk::DataNode*> nodes )
{
  this->RemoveObservers();
  if ( nodes.size() > 1 )
  {
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
  this->AddObservers();
  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
}

void QmitkCorrespondingPointSetsModel::SetTimeStep(int t)
{
  if (!m_TimeStepper)
    return;

  m_TimeStepper->SetPos(t);
  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
}

int QmitkCorrespondingPointSetsModel::GetTimeStep() const
{
  if (!m_TimeStepper)
    return 0;

  return m_TimeStepper->GetPos();
}

int QmitkCorrespondingPointSetsModel::rowCount(const QModelIndex&) const
{
  if (!m_TimeStepper)
    return 0;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  mitk::PointSet::Pointer referencePointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);
  int sizePS = 0;
  int sizeRPS = 0;
  if ( pointSet.IsNotNull() )
  {
    sizePS = pointSet->GetSize(m_TimeStepper->GetPos());
  }
  if ( referencePointSet.IsNotNull() )
  {
    sizeRPS = referencePointSet->GetSize(m_TimeStepper->GetPos());
  }
  if ( sizePS > sizeRPS )
    return sizePS;
  return sizeRPS;
}

int QmitkCorrespondingPointSetsModel::columnCount(const QModelIndex&) const
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

  if ( index.row() >= pointSet->GetSize(m_TimeStepper->GetPos()) )
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    int id;
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

bool QmitkCorrespondingPointSetsModel::GetPointForModelIndex(const QModelIndex &index, mitk::PointSet::PointType& p,
                                                             int& id) const
{
  if (!m_TimeStepper)
    return false;

  mitk::PointSet::Pointer pointSet = NULL;
  if ( index.column() == 0 )
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if ( index.column() == 1 )
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if (pointSet.IsNull() || !pointSet->GetPointSet(m_TimeStepper->GetPos()))
    return false;

  if ((index.row() < 0) || (index.row() >= (int)pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->Size()))
    return false;

  // get the nth. element, if it exists.
  // we can not use the index directly, because PointSet uses a map container,
  // where the index is not necessarily the same as the key.
  // Therefore we have to count the elements
  mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->Begin();
  for (int i = 0; i < index.row(); ++i)
  {
    ++it;
    if (it == pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->End())
      return false;
  }

  if (it != pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->End()) // not at the end,
  {
    p = it->Value();
    id = it->Index();
    return true;
  }

  return false;
}
bool QmitkCorrespondingPointSetsModel::GetPointForModelIndex(int row, int column, mitk::PointSet::PointType& p,
                                                             int& id) const
{
  if (!m_TimeStepper)
    return false;

  mitk::PointSet::Pointer pointSet = NULL;
  if (column == 0 )
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if ( column == 1 )
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if (pointSet.IsNull() || !pointSet->GetPointSet(m_TimeStepper->GetPos()))
    return false;

  if ((row < 0) || (row >= (int)pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->Size()))
    return false;

  // get the nth. element, if it exists.
  // we can not use the index directly, because PointSet uses a map container,
  // where the index is not necessarily the same as the key.
  // Therefore we have to count the elements
  mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->Begin();
  for (int i = 0; i < row; ++i)
  {
    ++it;
    if (it == pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->End())
    return false;
  }

  if (it != pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->End()) // not at the end,
  {
    p = it->Value();
    id = it->Index();
    return true;
  }

  return false;
}

bool QmitkCorrespondingPointSetsModel::GetModelIndexForPointID(int id, QModelIndex& index, int column) const
{
  if (!m_TimeStepper)
    return false;

  mitk::PointSet::Pointer pointSet = NULL;
  if (column == 0)
    pointSet = this->CheckForPointSetInNode(m_PointSetNode);
  else if (column == 1)
    pointSet = this->CheckForPointSetInNode(m_ReferencePointSetNode);

  if (!pointSet.IsNull() || !pointSet->GetPointSet(m_TimeStepper->GetPos()))
  {
    mitk::PointSet::PointsContainer::Pointer points = pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints();

    if (!points->IndexExists(id))
      return false;

    unsigned int idx = 0;
    for (mitk::PointSet::PointsContainer::Iterator it = points->Begin(); it != points->End(); ++it)
    {
      if (static_cast<int>(it->Index()) == id) // we found the correct element
      {
        index = this->index(idx, column);
        return true;
      }

      idx++;
    }
  }
  return false; // nothing found
}

bool QmitkCorrespondingPointSetsModel::GetModelIndexForSelectedPoint(QModelIndex& index) const
{
  if (!m_TimeStepper)
    return false;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);

  if (pointSet.IsNull())
    return false;

  return this->GetModelIndexForPointID(
    pointSet->SearchSelectedPoint(m_TimeStepper->GetPos()),
    index,
    this->m_SelectedPointSetIndex);
}

void QmitkCorrespondingPointSetsModel::MoveSelectedPointUp()
{
  if (!m_TimeStepper)
    return;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return;

  int selectedID = pointSet->SearchSelectedPoint(m_TimeStepper->GetPos());

  if (selectedID == -1)
    return;

  mitk::ScalarType tsInMS = pointSet->GetTimeGeometry()->TimeStepToTimePoint(m_TimeStepper->GetPos());
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTUP,tsInMS, pointSet->GetPoint(selectedID, m_TimeStepper->GetPos()), selectedID, true);
  pointSet->ExecuteOperation(doOp);
  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalPointSetChanged();
}

void QmitkCorrespondingPointSetsModel::MoveSelectedPointDown()
{
  if (!m_TimeStepper)
    return;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return;

  int selectedID = pointSet->SearchSelectedPoint(m_TimeStepper->GetPos());

  if (selectedID == -1)
    return;

  mitk::ScalarType tsInMS = pointSet->GetTimeGeometry()->TimeStepToTimePoint(m_TimeStepper->GetPos());
  mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpMOVEPOINTDOWN, tsInMS, pointSet->GetPoint(selectedID, m_TimeStepper->GetPos()), selectedID, true);
  pointSet->ExecuteOperation(doOp);

  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalPointSetChanged();
}

int QmitkCorrespondingPointSetsModel::SearchSelectedPoint()
{
  if (!m_TimeStepper)
    return -1;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return -1;

  return pointSet->SearchSelectedPoint(m_TimeStepper->GetPos());
}

void QmitkCorrespondingPointSetsModel::RemoveSelectedPoint()
{
  if (!m_TimeStepper)
    return;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0){
    dataNode = this->m_PointSetNode;
  }
  else if (this->m_SelectedPointSetIndex == 1){
    dataNode = this->m_ReferencePointSetNode;
  }
  if (dataNode == NULL)
    return;

  //send a DEL event to pointsetinteractor
  const mitk::Event* delEvent = new mitk::Event(this->m_MultiWidget->GetRenderWindow1()->GetRenderer(), mitk::Type_KeyPress, mitk::BS_NoButton, mitk::BS_NoButton, mitk::Key_Delete);
  mitk::StateEvent* delStateEvent = new mitk::StateEvent(mitk::EIDDELETE, delEvent);
  m_Interactor->HandleEvent(delStateEvent);
  delete delEvent;
  delete delStateEvent;


  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalPointSetChanged();
}

void QmitkCorrespondingPointSetsModel::MoveSelectedPoint(int targetID)
{
  if (!m_TimeStepper)
    return;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  if (dataNode == NULL)
    return;

  mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(dataNode);
  if (pointSet.IsNull())
    return;

  int selectedID = pointSet->SearchSelectedPoint(m_TimeStepper->GetPos());

  if (targetID >= pointSet->GetSize())
    targetID = pointSet->GetSize()-1;

  mitk::PointSet::PointsContainer::Iterator it = pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->Begin();
  for (int i=0; i<targetID; ++i)
    it++;
  targetID = it->Index();

  if (selectedID < 0 || targetID < 0)
    return;

  int direction = mitk::OpNOTHING;
  if (selectedID > targetID)
    direction = mitk::OpMOVEPOINTUP;
  else if (selectedID < targetID)
    direction = mitk::OpMOVEPOINTDOWN;

  while (selectedID != targetID){
    mitk::ScalarType tsInMS = pointSet->GetTimeGeometry()->TimeStepToTimePoint(m_TimeStepper->GetPos());
    mitk::PointOperation* doOp = new mitk::PointOperation(direction, tsInMS, pointSet->GetPoint(selectedID, m_TimeStepper->GetPos()), selectedID, true);
    pointSet->ExecuteOperation(doOp);
    selectedID = pointSet->SearchSelectedPoint(m_TimeStepper->GetPos());
  }

  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalPointSetChanged();
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

void QmitkCorrespondingPointSetsModel::SetSelectedPointSetIndex(int index)
{
  if (index<-1 || index>1)
    return;
  this->m_SelectedPointSetIndex = index;
}

void QmitkCorrespondingPointSetsModel::ClearSelectedPointSet()
{
  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  if (dataNode == NULL)
    return;

  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(dataNode->GetData());

  mitk::PointSet::PointsContainer::Iterator it;
  if (this->m_TimeStepper->GetRangeMax()==-1)
  {
    while( !pointSet->IsEmptyTimeStep(0) )
    {
      if (pointSet->GetPointSet(0))
      {
        it = pointSet->GetPointSet(0)->GetPoints()->Begin();
        pointSet->SetSelectInfo(it->Index(),true, 0);
        this->RemoveSelectedPoint();
      }
      else
      {
        break;
      }
    }
  }
  else
  {
    int oldTimeStep = this->m_TimeStepper->GetPos();
    for (int i=0; i<this->m_TimeStepper->GetRangeMax(); i++)
    {
      this->m_TimeStepper->SetPos(i);
      while( !pointSet->IsEmptyTimeStep(i) )
      {
        if (pointSet->GetPointSet(i))
        {
          it = pointSet->GetPointSet(i)->GetPoints()->Begin();
          pointSet->SetSelectInfo(it->Index(),true, i);
          this->RemoveSelectedPoint();
        }
      }
    }
    this->m_TimeStepper->SetPos(oldTimeStep);
  }

  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalPointSetChanged();
}
void QmitkCorrespondingPointSetsModel::ClearCurrentTimeStep()
{
  if (!m_TimeStepper)
    return;

  mitk::DataNode* dataNode = NULL;

  if (this->m_SelectedPointSetIndex == 0)
    dataNode = this->m_PointSetNode;
  else if (this->m_SelectedPointSetIndex == 1)
    dataNode = this->m_ReferencePointSetNode;

  if (dataNode == NULL)
    return;

  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(dataNode->GetData());

  mitk::PointSet::PointsContainer::Iterator it;
  while( !pointSet->IsEmptyTimeStep(m_TimeStepper->GetPos()) )
  {
    it = pointSet->GetPointSet(m_TimeStepper->GetPos())->GetPoints()->Begin();
    pointSet->SetSelectInfo(it->Index(),true, m_TimeStepper->GetPos());
    this->RemoveSelectedPoint();
  }

  QAbstractTableModel::beginResetModel();
  QAbstractTableModel::endResetModel();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalPointSetChanged();
}

mitk::Stepper::Pointer QmitkCorrespondingPointSetsModel::GetStepper()
{
  return this->m_TimeStepper;
}
void QmitkCorrespondingPointSetsModel::SetStepper(mitk::Stepper::Pointer stepper)
{
  this->m_TimeStepper = stepper;
}
int QmitkCorrespondingPointSetsModel::GetSelectedPointSetIndex()
{
  return this->m_SelectedPointSetIndex;
}
void QmitkCorrespondingPointSetsModel::UpdateSelection(mitk::DataNode* selectedNode)
{
  this->RemoveInteractor();
  if(!selectedNode)
    return;
  m_Interactor = dynamic_cast<mitk::PointSetInteractor*>(selectedNode->GetInteractor());
  if (m_Interactor.IsNull())//if not present, instanciate one
    m_Interactor = mitk::PointSetInteractor::New("pointsetinteractor", selectedNode);

  //add it to global interaction to activate it
  mitk::GlobalInteraction::GetInstance()->AddInteractor( m_Interactor );
}
void QmitkCorrespondingPointSetsModel::RemoveInteractor()
{
  if (m_Interactor){
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_Interactor );
    m_Interactor = NULL;
  }
}
QmitkStdMultiWidget* QmitkCorrespondingPointSetsModel::GetMultiWidget()
{
  return this->m_MultiWidget;
}
void QmitkCorrespondingPointSetsModel::SetMultiWidget( QmitkStdMultiWidget* multiWidget )
{
  this->m_MultiWidget = multiWidget;
  this->m_TimeStepper = m_MultiWidget->GetTimeNavigationController()->GetTime();
}

