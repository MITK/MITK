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

#include "mitkDataStorage.h"

#include "mitkDataNode.h"
#include "mitkProperties.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkGroupTagProperty.h"
#include "mitkImage.h"
#include "itkMutexLockHolder.h"
#include "itkCommand.h"

mitk::DataStorage::DataStorage() : itk::Object()
  , m_BlockNodeModifiedEvents(false)
{
}

mitk::DataStorage::~DataStorage()
{
  ///// we can not call GetAll() in destructor, because it is implemented in a subclass
  //SetOfObjects::ConstPointer all = this->GetAll();
  //for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  //  this->RemoveListeners(it->Value());
  //m_NodeModifiedObserverTags.clear();
  //m_NodeDeleteObserverTags.clear();
}

void mitk::DataStorage::Add(mitk::DataNode* node, mitk::DataNode* parent)
{
  mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
  if (parent != NULL) //< Return empty set if parent is null
    parents->InsertElement(0, parent);
  this->Add(node, parents);
}

void mitk::DataStorage::Remove(const mitk::DataStorage::SetOfObjects* nodes)
{
  if (nodes == NULL)
    return;
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it != nodes->End(); it++)
    this->Remove(it.Value());
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSubset(const NodePredicateBase* condition) const
{
  mitk::DataStorage::SetOfObjects::ConstPointer result = this->FilterSetOfObjects(this->GetAll(), condition);
  return result;
}

mitk::DataNode* mitk::DataStorage::GetNamedNode(const char* name) const

{
  if (name == NULL)
    return NULL;

  mitk::StringProperty::Pointer s(mitk::StringProperty::New(name));
  mitk::NodePredicateProperty::Pointer p = mitk::NodePredicateProperty::New("name", s);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(p);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}

mitk::DataNode* mitk::DataStorage::GetNode(const NodePredicateBase* condition) const
{
  if (condition == NULL)
    return NULL;

  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(condition);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}

mitk::DataNode* mitk::DataStorage::GetNamedDerivedNode(const char* name, const mitk::DataNode* sourceNode, bool onlyDirectDerivations) const
{
  if (name == NULL)
    return NULL;

  mitk::StringProperty::Pointer s(mitk::StringProperty::New(name));
  mitk::NodePredicateProperty::Pointer p = mitk::NodePredicateProperty::New("name", s);
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDerivations(sourceNode, p, onlyDirectDerivations);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}

void mitk::DataStorage::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  //Superclass::PrintSelf(os, indent);
  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
  os << indent << "DataStorage " << this << " is managing " << all->Size() << " objects. List of objects:" << std::endl;
  for (mitk::DataStorage::SetOfObjects::ConstIterator allIt = all->Begin(); allIt != all->End(); allIt++)
  {
    std::string name;
    allIt.Value()->GetName(name);
    std::string datatype;
    if (allIt.Value()->GetData() != NULL)
      datatype = allIt.Value()->GetData()->GetNameOfClass();
    os << indent << " " << allIt.Value().GetPointer() << "<" << datatype << ">: " << name << std::endl;
    mitk::DataStorage::SetOfObjects::ConstPointer parents = this->GetSources(allIt.Value());
    if (parents->Size() > 0)
    {
      os << indent << "  Direct sources: ";
      for (mitk::DataStorage::SetOfObjects::ConstIterator parentIt = parents->Begin(); parentIt != parents->End(); parentIt++)
        os << parentIt.Value().GetPointer() << ", ";
      os << std::endl;
    }
    mitk::DataStorage::SetOfObjects::ConstPointer derivations = this->GetDerivations(allIt.Value());
    if (derivations->Size() > 0)
    {
      os << indent << "  Direct derivations: ";
      for (mitk::DataStorage::SetOfObjects::ConstIterator derivationIt = derivations->Begin(); derivationIt != derivations->End(); derivationIt++)
        os << derivationIt.Value().GetPointer() << ", ";
      os << std::endl;
    }
  }
  os << std::endl;
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::FilterSetOfObjects(const SetOfObjects* set, const NodePredicateBase* condition) const
{
  if (set == NULL)
    return NULL;

  mitk::DataStorage::SetOfObjects::Pointer result = mitk::DataStorage::SetOfObjects::New();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = set->Begin(); it != set->End(); it++)
    if (condition == NULL || condition->CheckNode(it.Value()) == true) //alway copy the set, otherwise the iterator in mitk::DataStorage::Remove() will crash
      result->InsertElement(result->Size(), it.Value());

  return mitk::DataStorage::SetOfObjects::ConstPointer(result);
}

const mitk::DataNode::GroupTagList mitk::DataStorage::GetGroupTags() const
{
  DataNode::GroupTagList result;
  SetOfObjects::ConstPointer all = this->GetAll();
  if (all.IsNull())
    return result;

  for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = all->Begin(); nodeIt != all->End(); nodeIt++)  // for each node
  {
    mitk::PropertyList* pl = nodeIt.Value()->GetPropertyList();
    for (mitk::PropertyList::PropertyMap::const_iterator propIt = pl->GetMap()->begin(); propIt != pl->GetMap()->end(); propIt++)
      if (dynamic_cast<mitk::GroupTagProperty*>(propIt->second.GetPointer()) != NULL)
        result.insert(propIt->first);
  }

  return result;
}

void mitk::DataStorage::EmitAddNodeEvent(const mitk::DataNode* node)
{
  AddNodeEvent.Send(node);
}

void mitk::DataStorage::EmitRemoveNodeEvent(const mitk::DataNode* node)
{
  RemoveNodeEvent.Send(node);
}

void mitk::DataStorage::OnNodeInteractorChanged( itk::Object *caller, const itk::EventObject& )
{
  const mitk::DataNode* _Node = dynamic_cast<const mitk::DataNode*>(caller);
  if(_Node)
  {
    InteractorChangedNodeEvent.Send( _Node );
  }
}

void mitk::DataStorage::OnNodeModifiedOrDeleted( const itk::Object *caller, const itk::EventObject &event )
{
  if( m_BlockNodeModifiedEvents )
    return;

  const mitk::DataNode* _Node = dynamic_cast<const mitk::DataNode*>(caller);
  if(_Node)
  {
    const itk::ModifiedEvent* modEvent = dynamic_cast<const itk::ModifiedEvent*>(&event);
    if(modEvent)
      ChangedNodeEvent.Send(_Node);
    else
      DeleteNodeEvent.Send(_Node);
  }
}

void mitk::DataStorage::AddListeners( const mitk::DataNode* _Node )
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_MutexOne);
  // node must not be 0 and must not be yet registered
  mitk::DataNode* NonConstNode = const_cast<mitk::DataNode*>(_Node);
  if(_Node && m_NodeModifiedObserverTags
    .find(NonConstNode) == m_NodeModifiedObserverTags.end())
  {
    itk::MemberCommand<mitk::DataStorage>::Pointer nodeModifiedCommand =
      itk::MemberCommand<mitk::DataStorage>::New();
    nodeModifiedCommand->SetCallbackFunction(this
      , &mitk::DataStorage::OnNodeModifiedOrDeleted);
    m_NodeModifiedObserverTags[NonConstNode]
    = NonConstNode->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);

    itk::MemberCommand<mitk::DataStorage>::Pointer interactorChangedCommand = itk::MemberCommand<mitk::DataStorage>::New();
    interactorChangedCommand->SetCallbackFunction(this, &mitk::DataStorage::OnNodeInteractorChanged);
    m_NodeInteractorChangedObserverTags[NonConstNode] = NonConstNode->AddObserver( mitk::DataNode::InteractorChangedEvent(), interactorChangedCommand);

    // add itk delete listener on datastorage
    itk::MemberCommand<mitk::DataStorage>::Pointer deleteCommand =
      itk::MemberCommand<mitk::DataStorage>::New();
    deleteCommand->SetCallbackFunction(this, &mitk::DataStorage::OnNodeModifiedOrDeleted);
    // add observer
    m_NodeDeleteObserverTags[NonConstNode]
    = NonConstNode->AddObserver(itk::DeleteEvent(), deleteCommand);
  }
}

void mitk::DataStorage::RemoveListeners( const mitk::DataNode* _Node )
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_MutexOne) ;
  // node must not be 0 and must be registered
  mitk::DataNode* NonConstNode = const_cast<mitk::DataNode*>(_Node);
  if(_Node && m_NodeModifiedObserverTags
    .find(NonConstNode) != m_NodeModifiedObserverTags.end())
  {
    // const cast is bad! but sometimes it is necessary. removing an observer does not really
    // touch the internal state
    NonConstNode->RemoveObserver(m_NodeModifiedObserverTags
      .find(NonConstNode)->second);
    NonConstNode->RemoveObserver(m_NodeDeleteObserverTags
      .find(NonConstNode)->second);
    NonConstNode->RemoveObserver(m_NodeInteractorChangedObserverTags
      .find(NonConstNode)->second);

    m_NodeModifiedObserverTags.erase(NonConstNode);
    m_NodeDeleteObserverTags.erase(NonConstNode);
    m_NodeInteractorChangedObserverTags.erase(NonConstNode);
  }
}

mitk::TimeGeometry::Pointer mitk::DataStorage::ComputeBoundingGeometry3D( const SetOfObjects* input, const char* boolPropertyKey, const mitk::BaseRenderer* renderer, const char* boolPropertyKey2) const
{
  if (input == NULL)
    throw std::invalid_argument("DataStorage: input is invalid");

  BoundingBox::PointsContainer::Pointer pointscontainer=BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid=0;
  Point3D point;

  Vector3D minSpacing;
  minSpacing.Fill(itk::NumericTraits<mitk::ScalarType>::max());

  ScalarType stmin, stmax;
  stmin= itk::NumericTraits<mitk::ScalarType>::NonpositiveMin();
  stmax= itk::NumericTraits<mitk::ScalarType>::max();

  ScalarType minimalIntervallSize = stmax;
  ScalarType minimalTime = stmax;
  ScalarType maximalTime = 0;

  // Needed for check of zero bounding boxes
  mitk::ScalarType nullpoint[]={0,0,0,0,0,0};
  BoundingBox::BoundsArrayType itkBoundsZero(nullpoint);

  for (SetOfObjects::ConstIterator it = input->Begin(); it != input->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if((node.IsNotNull()) && (node->GetData() != NULL) &&
      (node->GetData()->IsEmpty()==false) &&
      node->IsOn(boolPropertyKey, renderer) &&
      node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const TimeGeometry* timeGeometry = node->GetData()->GetUpdatedTimeGeometry();

      if (timeGeometry != NULL )
      {
        // bounding box (only if non-zero)
        BoundingBox::BoundsArrayType itkBounds = timeGeometry->GetBoundingBoxInWorld()->GetBounds();
        if (itkBounds == itkBoundsZero)
        {
          continue;
        }

        unsigned char i;
        for(i=0; i<8; ++i)
        {
          point = timeGeometry->GetCornerPointInWorld(i);
          if(point[0]*point[0]+point[1]*point[1]+point[2]*point[2] < large)
            pointscontainer->InsertElement( pointid++, point);
          else
          {
            itkGenericOutputMacro( << "Unrealistically distant corner point encountered. Ignored. Node: " << node );
          }
        }
        try
        {
          // time bounds
          // iterate over all time steps
          // Attention: Objects with zero bounding box are not respected in time bound calculation
          for (TimeStepType i=0; i<timeGeometry->CountTimeSteps(); i++)
          {
            Vector3D spacing = node->GetData()->GetGeometry(i)->GetSpacing();
            for (int axis = 0; axis < 3; ++ axis)
            {
              if (spacing[axis] < minSpacing[axis]) minSpacing[axis] = spacing[axis];
            }

            const TimeBounds & curTimeBounds = node->GetData()->GetTimeGeometry()->GetTimeBounds(i);
            // get the minimal time of all objects in the DataStorage
            if ((curTimeBounds[0]<minimalTime)&&(curTimeBounds[0]>stmin))
            {
              minimalTime=curTimeBounds[0];
            }
            // get the maximal time of all objects in the DataStorage
            if ((curTimeBounds[1]>maximalTime)&&(curTimeBounds[1]<stmax))
            {
              maximalTime = curTimeBounds[1];
            }
            // get the minimal TimeBound of all time steps of the current DataNode
            if (curTimeBounds[1]-curTimeBounds[0]<minimalIntervallSize)
            {
              minimalIntervallSize = curTimeBounds[1]-curTimeBounds[0];
            }
          }
        }
        catch(itk::ExceptionObject e)
        {
          MITK_ERROR << e << std::endl;
        }
      }
    }
  }

  BoundingBox::Pointer result = BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  // compute the number of time steps
  unsigned int numberOfTimeSteps = 1;
  if (maximalTime==0) // make sure that there is at least one time sliced geometry in the data storage
  {
    minimalTime = 0;
    maximalTime = 1;
    minimalIntervallSize = 1;
  }
  numberOfTimeSteps = static_cast<unsigned int>((maximalTime-minimalTime)/minimalIntervallSize);

  TimeGeometry::Pointer timeGeometry = NULL;
  if ( result->GetPoints()->Size()>0 )
  {
    // Initialize a geometry of a single time step
    Geometry3D::Pointer geometry = Geometry3D::New();
    geometry->Initialize();
    // correct bounding-box (is now in mm, should be in index-coordinates)
    // according to spacing
    BoundingBox::BoundsArrayType bounds = result->GetBounds();
    int i;
    for(i = 0; i < 6; ++i)
    {
      bounds[i] /= minSpacing[i/2];
    }
    geometry->SetBounds(bounds);
    geometry->SetSpacing(minSpacing);
    // Initialize the time sliced geometry
    timeGeometry = ProportionalTimeGeometry::New();
    dynamic_cast<ProportionalTimeGeometry*>(timeGeometry.GetPointer())->Initialize(geometry,numberOfTimeSteps);
    dynamic_cast<ProportionalTimeGeometry*>(timeGeometry.GetPointer())->SetFirstTimePoint(minimalTime);
    dynamic_cast<ProportionalTimeGeometry*>(timeGeometry.GetPointer())->SetStepDuration(minimalIntervallSize);
  }
  return timeGeometry;
}

mitk::TimeGeometry::Pointer mitk::DataStorage::ComputeBoundingGeometry3D( const char* boolPropertyKey, const mitk::BaseRenderer* renderer, const char* boolPropertyKey2) const
{
  return this->ComputeBoundingGeometry3D(this->GetAll(), boolPropertyKey, renderer, boolPropertyKey2);
}

mitk::TimeGeometry::Pointer mitk::DataStorage::ComputeVisibleBoundingGeometry3D( const mitk::BaseRenderer* renderer, const char* boolPropertyKey )
{
  return ComputeBoundingGeometry3D( "visible", renderer, boolPropertyKey );
}

mitk::BoundingBox::Pointer mitk::DataStorage::ComputeBoundingBox( const char* boolPropertyKey, const mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  BoundingBox::PointsContainer::Pointer pointscontainer=BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid=0;
  Point3D point;

  // Needed for check of zero bounding boxes
  mitk::ScalarType nullpoint[]={0,0,0,0,0,0};
  BoundingBox::BoundsArrayType itkBoundsZero(nullpoint);

  SetOfObjects::ConstPointer all = this->GetAll();
  for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if((node.IsNotNull()) && (node->GetData() != NULL) &&
      (node->GetData()->IsEmpty()==false) &&
      node->IsOn(boolPropertyKey, renderer) &&
      node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const TimeGeometry* geometry = node->GetData()->GetUpdatedTimeGeometry();
      if (geometry != NULL )
      {
        // bounding box (only if non-zero)
        BoundingBox::BoundsArrayType itkBounds = geometry->GetBoundingBoxInWorld()->GetBounds();
        if (itkBounds == itkBoundsZero)
        {
          continue;
        }

        unsigned char i;
        for(i=0; i<8; ++i)
        {
          point = geometry->GetCornerPointInWorld(i);
          if(point[0]*point[0]+point[1]*point[1]+point[2]*point[2] < large)
            pointscontainer->InsertElement( pointid++, point);
          else
          {
            itkGenericOutputMacro( << "Unrealistically distant corner point encountered. Ignored. Node: " << node );
          }
        }
      }
    }
  }

  BoundingBox::Pointer result = BoundingBox::New();
  result->SetPoints(pointscontainer);
  result->ComputeBoundingBox();

  return result;
}

mitk::TimeBounds mitk::DataStorage::ComputeTimeBounds( const char* boolPropertyKey, const mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  TimeBounds timeBounds;

  ScalarType stmin, stmax, cur;

  stmin= itk::NumericTraits<mitk::ScalarType>::NonpositiveMin();
  stmax= itk::NumericTraits<mitk::ScalarType>::max();

  timeBounds[0]=stmax; timeBounds[1]=stmin;

  SetOfObjects::ConstPointer all = this->GetAll();
  for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if((node.IsNotNull()) && (node->GetData() != NULL) &&
      (node->GetData()->IsEmpty()==false) &&
      node->IsOn(boolPropertyKey, renderer) &&
      node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const TimeGeometry* geometry = node->GetData()->GetUpdatedTimeGeometry();
      if (geometry != NULL )
      {
        const TimeBounds & curTimeBounds = geometry->GetTimeBounds();
        cur=curTimeBounds[0];
        //is it after -infinity, but before everything else that we found until now?
        if((cur > stmin) && (cur < timeBounds[0]))
          timeBounds[0] = cur;

        cur=curTimeBounds[1];
        //is it before infinity, but after everything else that we found until now?
        if((cur < stmax) && (cur > timeBounds[1]))
          timeBounds[1] = cur;
      }
    }
  }
  if(!(timeBounds[0] < stmax))
  {
    timeBounds[0] = stmin;
    timeBounds[1] = stmax;
  }
  return timeBounds;
}

void mitk::DataStorage::BlockNodeModifiedEvents( bool block )
{
  m_BlockNodeModifiedEvents = block;
}
