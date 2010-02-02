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

#include "mitkDataStorage.h"

#include "mitkDataTreeNode.h"
#include "mitkProperties.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkGroupTagProperty.h"
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


void mitk::DataStorage::Add(mitk::DataTreeNode* node, mitk::DataTreeNode* parent)
{
  mitk::DataStorage::SetOfObjects::Pointer parents = mitk::DataStorage::SetOfObjects::New();
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


mitk::DataTreeNode* mitk::DataStorage::GetNamedNode(const char* name) const

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


mitk::DataTreeNode* mitk::DataStorage::GetNode(const NodePredicateBase* condition) const
{
  if (condition == NULL)
    return NULL;
  
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(condition);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return NULL;
}

mitk::DataTreeNode* mitk::DataStorage::GetNamedDerivedNode(const char* name, const mitk::DataTreeNode* sourceNode, bool onlyDirectDerivations) const
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
  if (condition == NULL)
    return set;
  mitk::DataStorage::SetOfObjects::Pointer result = mitk::DataStorage::SetOfObjects::New();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = set->Begin(); it != set->End(); it++)
    if (condition->CheckNode(it.Value()) == true)
      result->InsertElement(result->Size(), it.Value());
  return mitk::DataStorage::SetOfObjects::ConstPointer(result);
}


const mitk::DataTreeNode::GroupTagList mitk::DataStorage::GetGroupTags() const
{
  DataTreeNode::GroupTagList result;
  SetOfObjects::ConstPointer all = this->GetAll();
  if (all.IsNull())
    return result;

  for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = all->Begin(); nodeIt != all->End(); nodeIt++)  // for each node
  {
    mitk::PropertyList* pl = nodeIt.Value()->GetPropertyList();
    for (mitk::PropertyList::PropertyMap::const_iterator propIt = pl->GetMap()->begin(); propIt != pl->GetMap()->end(); propIt++)
      if (dynamic_cast<mitk::GroupTagProperty*>(propIt->second.first.GetPointer()) != NULL)
        result.insert(propIt->first);
  }

  return result;
}


void mitk::DataStorage::EmitAddNodeEvent(const mitk::DataTreeNode* node)
{
  AddNodeEvent.Send(node);
}


void mitk::DataStorage::EmitRemoveNodeEvent(const mitk::DataTreeNode* node)
{
  RemoveNodeEvent.Send(node);
}

void mitk::DataStorage::OnNodeModifiedOrDeleted( const itk::Object *caller, const itk::EventObject &event )
{
  if(m_BlockNodeModifiedEvents)
    return;

  const mitk::DataTreeNode* _Node = dynamic_cast<const mitk::DataTreeNode*>(caller);
  if(_Node)
  {
    const itk::ModifiedEvent* modEvent = dynamic_cast<const itk::ModifiedEvent*>(&event);
    if(modEvent)
      ChangedNodeEvent.Send(_Node);
    else
      DeleteNodeEvent.Send(_Node);
  }
}


void mitk::DataStorage::AddListeners( const mitk::DataTreeNode* _Node )
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_MutexOne);
  // node must not be 0 and must not be yet registered
  if(_Node && m_NodeModifiedObserverTags.find(_Node) == m_NodeModifiedObserverTags.end())
  {
    itk::MemberCommand<mitk::DataStorage>::Pointer nodeModifiedCommand =
      itk::MemberCommand<mitk::DataStorage>::New();
    nodeModifiedCommand->SetCallbackFunction(this, &mitk::DataStorage::OnNodeModifiedOrDeleted);
    m_NodeModifiedObserverTags[_Node] 
      = _Node->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);

    // add itk delete listener on datastorage
    itk::MemberCommand<mitk::DataStorage>::Pointer deleteCommand =
      itk::MemberCommand<mitk::DataStorage>::New();
    deleteCommand->SetCallbackFunction(this, &mitk::DataStorage::OnNodeModifiedOrDeleted);
    // add observer
    m_NodeDeleteObserverTags[_Node] = _Node->AddObserver(itk::DeleteEvent(), deleteCommand);
  }
}


void mitk::DataStorage::RemoveListeners( const mitk::DataTreeNode* _Node )
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_MutexOne) ;
  // node must not be 0 and must be registered
  if(_Node && m_NodeModifiedObserverTags.find(_Node) != m_NodeModifiedObserverTags.end())
  {
    // const cast is bad! but sometimes it is necessary. removing an observer does not really
    // touch the internal state
    mitk::DataTreeNode* NonConstNode = const_cast<mitk::DataTreeNode*>(_Node);
    NonConstNode->RemoveObserver(m_NodeModifiedObserverTags.find(_Node)->second);
    NonConstNode->RemoveObserver(m_NodeDeleteObserverTags.find(_Node)->second);
  }
}

mitk::TimeSlicedGeometry::Pointer mitk::DataStorage::ComputeBoundingGeometry3D( const SetOfObjects* input, const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  if (input == NULL)
    throw std::invalid_argument("DataStorage: input is invalid");

  BoundingBox::PointsContainer::Pointer pointscontainer=BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid=0;
  Point3D point;

  Vector3D minSpacing;
  minSpacing.Fill(ScalarTypeNumericTraits::max());

  ScalarType stmin, stmax;
  stmin= ScalarTypeNumericTraits::NonpositiveMin();
  stmax= ScalarTypeNumericTraits::max();

  ScalarType minimalIntervallSize = stmax;
  ScalarType minimalTime = stmax;
  ScalarType maximalTime = 0;

  // Needed for check of zero bounding boxes
  mitk::ScalarType nullpoint[]={0,0,0,0,0,0};
  BoundingBox::BoundsArrayType itkBoundsZero(nullpoint);

  for (SetOfObjects::ConstIterator it = input->Begin(); it != input->End(); ++it)
  {
    DataTreeNode::Pointer node = it->Value();
    if((node.IsNotNull()) && (node->GetData() != NULL) && 
      (node->GetData()->IsEmpty()==false) && 
      node->IsOn(boolPropertyKey, renderer) && 
      node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const TimeSlicedGeometry* geometry = node->GetData()->GetUpdatedTimeSlicedGeometry();
      if (geometry != NULL ) 
      {
        // bounding box (only if non-zero)
        BoundingBox::BoundsArrayType itkBounds = geometry->GetBoundingBox()->GetBounds();
        if (itkBounds == itkBoundsZero)
        {
          continue;
        }

        unsigned char i;
        for(i=0; i<8; ++i)
        {
          point = geometry->GetCornerPoint(i);
          if(point[0]*point[0]+point[1]*point[1]+point[2]*point[2] < large)
            pointscontainer->InsertElement( pointid++, point);
          else
          {
            itkGenericOutputMacro( << "Unrealistically distant corner point encountered. Ignored. Node: " << node );
          }
        }
        // spacing
        try
        {
          AffineTransform3D::Pointer inverseTransform = AffineTransform3D::New();
          geometry->GetIndexToWorldTransform()->GetInverse(inverseTransform);
          vnl_vector< AffineTransform3D::MatrixType::ValueType > unitVector(3);
          int axis;
          for(axis = 0; axis < 3; ++axis)
          {
            unitVector.fill(0);
            unitVector[axis] = 1.0;
            ScalarType mmPerPixel = 1.0/(inverseTransform->GetMatrix()*unitVector).magnitude();
            if(minSpacing[axis] > mmPerPixel)
            {
              minSpacing[axis] = mmPerPixel;
            }
          }
          // time bounds
          // iterate over all time steps
          // Attention: Objects with zero bounding box are not respected in time bound calculation
          TimeBounds minTB = geometry->GetTimeBounds(); 
          for (unsigned int i=0; i<geometry->GetTimeSteps(); i++)
          {
            const TimeBounds & curTimeBounds = node->GetData()->GetGeometry(i)->GetTimeBounds();
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
            // get the minimal TimeBound of all time steps of the current DataTreeNode
            if (curTimeBounds[1]-curTimeBounds[0]<minTB[1]-minTB[0])
            {
              minTB = curTimeBounds;
            }
          }
          // get the minimal interval size of all time steps of all objects of the DataStorage
          if (minTB[1]-minTB[0]<minimalIntervallSize)
          {
            minimalIntervallSize = minTB[1]-minTB[0];
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

  // minimal time bounds of a single time step for all geometries
  TimeBounds minTimeBounds; 
  minTimeBounds[0] = 0;
  minTimeBounds[1] = 1;
  // compute the number of time steps
  unsigned int numberOfTimeSteps = 1;
  if (maximalTime!=0) // make sure that there is at least one time sliced geometry in the data storage
  {
    minTimeBounds[0] = minimalTime;
    minTimeBounds[1] = minimalTime + minimalIntervallSize;
    numberOfTimeSteps = (maximalTime-minimalTime)/minimalIntervallSize;
  }

  TimeSlicedGeometry::Pointer timeSlicedGeometry = NULL;
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
    geometry->SetTimeBounds(minTimeBounds);
    // Initialize the time sliced geometry
    timeSlicedGeometry = TimeSlicedGeometry::New();
    timeSlicedGeometry->InitializeEvenlyTimed(geometry,numberOfTimeSteps);
  }
  return timeSlicedGeometry;
}

mitk::TimeSlicedGeometry::Pointer mitk::DataStorage::ComputeBoundingGeometry3D( const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  return this->ComputeBoundingGeometry3D(this->GetAll(), boolPropertyKey, renderer, boolPropertyKey2);
}

mitk::TimeSlicedGeometry::Pointer mitk::DataStorage::ComputeVisibleBoundingGeometry3D( mitk::BaseRenderer* renderer, const char* boolPropertyKey )
{
  return ComputeBoundingGeometry3D( "visible", renderer, boolPropertyKey );
}


mitk::BoundingBox::Pointer mitk::DataStorage::ComputeBoundingBox( const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
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
    DataTreeNode::Pointer node = it->Value();
    if((node.IsNotNull()) && (node->GetData() != NULL) && 
      (node->GetData()->IsEmpty()==false) && 
      node->IsOn(boolPropertyKey, renderer) && 
      node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedTimeSlicedGeometry();
      if (geometry != NULL ) 
      {
        // bounding box (only if non-zero)
        BoundingBox::BoundsArrayType itkBounds = geometry->GetBoundingBox()->GetBounds();
        if (itkBounds == itkBoundsZero)
        {
          continue;
        }

        unsigned char i;
        for(i=0; i<8; ++i)
        {
          point = geometry->GetCornerPoint(i);
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


mitk::TimeBounds mitk::DataStorage::ComputeTimeBounds( const char* boolPropertyKey, mitk::BaseRenderer* renderer, const char* boolPropertyKey2)
{
  TimeBounds timeBounds;

  ScalarType stmin, stmax, cur;

  stmin= ScalarTypeNumericTraits::NonpositiveMin();
  stmax= ScalarTypeNumericTraits::max();

  timeBounds[0]=stmax; timeBounds[1]=stmin;

  SetOfObjects::ConstPointer all = this->GetAll();
  for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataTreeNode::Pointer node = it->Value();
    if((node.IsNotNull()) && (node->GetData() != NULL) && 
      (node->GetData()->IsEmpty()==false) && 
      node->IsOn(boolPropertyKey, renderer) && 
      node->IsOn(boolPropertyKey2, renderer)
      )
    {
      const Geometry3D* geometry = node->GetData()->GetUpdatedTimeSlicedGeometry();
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
