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

#include "itkCommand.h"
#include "itkMutexLockHolder.h"
#include "mitkDataNode.h"
#include "mitkGroupTagProperty.h"
#include "mitkImage.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"
#include "mitkArbitraryTimeGeometry.h"

mitk::DataStorage::DataStorage() : itk::Object(), m_BlockNodeModifiedEvents(false)
{
}

mitk::DataStorage::~DataStorage()
{
  ///// we can not call GetAll() in destructor, because it is implemented in a subclass
  // SetOfObjects::ConstPointer all = this->GetAll();
  // for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  //  this->RemoveListeners(it->Value());
  // m_NodeModifiedObserverTags.clear();
  // m_NodeDeleteObserverTags.clear();
}

void mitk::DataStorage::Add(DataNode *node, DataNode *parent)
{
  DataStorage::SetOfObjects::Pointer parents = DataStorage::SetOfObjects::New();
  if (parent != nullptr) //< Return empty set if parent is null
    parents->InsertElement(0, parent);
  this->Add(node, parents);
}

void mitk::DataStorage::Remove(const DataStorage::SetOfObjects *nodes)
{
  if (nodes == nullptr)
    return;
  for (DataStorage::SetOfObjects::ConstIterator it = nodes->Begin(); it != nodes->End(); it++)
    this->Remove(it.Value());
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::GetSubset(const NodePredicateBase *condition) const
{
  DataStorage::SetOfObjects::ConstPointer result = this->FilterSetOfObjects(this->GetAll(), condition);
  return result;
}

mitk::DataNode *mitk::DataStorage::GetNamedNode(const char *name) const

{
  if (name == nullptr)
    return nullptr;

  StringProperty::Pointer s(StringProperty::New(name));
  NodePredicateProperty::Pointer p = NodePredicateProperty::New("name", s);
  DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(p);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return nullptr;
}

mitk::DataNode *mitk::DataStorage::GetNode(const NodePredicateBase *condition) const
{
  if (condition == nullptr)
    return nullptr;

  DataStorage::SetOfObjects::ConstPointer rs = this->GetSubset(condition);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return nullptr;
}

mitk::DataNode *mitk::DataStorage::GetNamedDerivedNode(const char *name,
                                                       const DataNode *sourceNode,
                                                       bool onlyDirectDerivations) const
{
  if (name == nullptr)
    return nullptr;

  StringProperty::Pointer s(StringProperty::New(name));
  NodePredicateProperty::Pointer p = NodePredicateProperty::New("name", s);
  DataStorage::SetOfObjects::ConstPointer rs = this->GetDerivations(sourceNode, p, onlyDirectDerivations);
  if (rs->Size() >= 1)
    return rs->GetElement(0);
  else
    return nullptr;
}

void mitk::DataStorage::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  // Superclass::PrintSelf(os, indent);
  DataStorage::SetOfObjects::ConstPointer all = this->GetAll();
  os << indent << "DataStorage " << this << " is managing " << all->Size() << " objects. List of objects:" << std::endl;
  for (DataStorage::SetOfObjects::ConstIterator allIt = all->Begin(); allIt != all->End(); allIt++)
  {
    std::string name;
    allIt.Value()->GetName(name);
    std::string datatype;
    if (allIt.Value()->GetData() != nullptr)
      datatype = allIt.Value()->GetData()->GetNameOfClass();
    os << indent << " " << allIt.Value().GetPointer() << "<" << datatype << ">: " << name << std::endl;
    DataStorage::SetOfObjects::ConstPointer parents = this->GetSources(allIt.Value());
    if (parents->Size() > 0)
    {
      os << indent << "  Direct sources: ";
      for (DataStorage::SetOfObjects::ConstIterator parentIt = parents->Begin(); parentIt != parents->End();
           parentIt++)
        os << parentIt.Value().GetPointer() << ", ";
      os << std::endl;
    }
    DataStorage::SetOfObjects::ConstPointer derivations = this->GetDerivations(allIt.Value());
    if (derivations->Size() > 0)
    {
      os << indent << "  Direct derivations: ";
      for (DataStorage::SetOfObjects::ConstIterator derivationIt = derivations->Begin();
           derivationIt != derivations->End();
           derivationIt++)
        os << derivationIt.Value().GetPointer() << ", ";
      os << std::endl;
    }
  }
  os << std::endl;
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::DataStorage::FilterSetOfObjects(const SetOfObjects *set,
                                                                                    const NodePredicateBase *condition) const
{
  if (set == nullptr)
    return nullptr;

  DataStorage::SetOfObjects::Pointer result = DataStorage::SetOfObjects::New();
  for (DataStorage::SetOfObjects::ConstIterator it = set->Begin(); it != set->End(); it++)
    if (condition == nullptr ||
        condition->CheckNode(it.Value()) ==
          true) // alway copy the set, otherwise the iterator in DataStorage::Remove() will crash
      result->InsertElement(result->Size(), it.Value());

  return DataStorage::SetOfObjects::ConstPointer(result);
}

const mitk::DataNode::GroupTagList mitk::DataStorage::GetGroupTags() const
{
  DataNode::GroupTagList result;
  SetOfObjects::ConstPointer all = this->GetAll();
  if (all.IsNull())
    return result;

  for (DataStorage::SetOfObjects::ConstIterator nodeIt = all->Begin(); nodeIt != all->End();
       nodeIt++) // for each node
  {
    PropertyList *pl = nodeIt.Value()->GetPropertyList();
    for (auto propIt = pl->GetMap()->begin(); propIt != pl->GetMap()->end();
         ++propIt)
      if (dynamic_cast<GroupTagProperty *>(propIt->second.GetPointer()) != nullptr)
        result.insert(propIt->first);
  }

  return result;
}

void mitk::DataStorage::EmitAddNodeEvent(const DataNode *node)
{
  AddNodeEvent.Send(node);
}

void mitk::DataStorage::EmitRemoveNodeEvent(const DataNode *node)
{
  RemoveNodeEvent.Send(node);
}

void mitk::DataStorage::OnNodeInteractorChanged(itk::Object *caller, const itk::EventObject &)
{
  const auto *_Node = dynamic_cast<const DataNode *>(caller);
  if (_Node)
  {
    InteractorChangedNodeEvent.Send(_Node);
  }
}

void mitk::DataStorage::OnNodeModifiedOrDeleted(const itk::Object *caller, const itk::EventObject &event)
{
  if (m_BlockNodeModifiedEvents)
    return;

  const auto *_Node = dynamic_cast<const DataNode *>(caller);
  if (_Node)
  {
    const auto *modEvent = dynamic_cast<const itk::ModifiedEvent *>(&event);
    if (modEvent)
      ChangedNodeEvent.Send(_Node);
    else
      DeleteNodeEvent.Send(_Node);
  }
}

void mitk::DataStorage::AddListeners(const DataNode *_Node)
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_MutexOne);
  // node must not be 0 and must not be yet registered
  auto *NonConstNode = const_cast<DataNode *>(_Node);
  if (_Node && m_NodeModifiedObserverTags.find(NonConstNode) == m_NodeModifiedObserverTags.end())
  {
    itk::MemberCommand<DataStorage>::Pointer nodeModifiedCommand = itk::MemberCommand<DataStorage>::New();
    nodeModifiedCommand->SetCallbackFunction(this, &DataStorage::OnNodeModifiedOrDeleted);
    m_NodeModifiedObserverTags[NonConstNode] = NonConstNode->AddObserver(itk::ModifiedEvent(), nodeModifiedCommand);

    itk::MemberCommand<DataStorage>::Pointer interactorChangedCommand =
      itk::MemberCommand<DataStorage>::New();
    interactorChangedCommand->SetCallbackFunction(this, &DataStorage::OnNodeInteractorChanged);
    m_NodeInteractorChangedObserverTags[NonConstNode] =
      NonConstNode->AddObserver(DataNode::InteractorChangedEvent(), interactorChangedCommand);

    // add itk delete listener on datastorage
    itk::MemberCommand<DataStorage>::Pointer deleteCommand = itk::MemberCommand<DataStorage>::New();
    deleteCommand->SetCallbackFunction(this, &DataStorage::OnNodeModifiedOrDeleted);
    // add observer
    m_NodeDeleteObserverTags[NonConstNode] = NonConstNode->AddObserver(itk::DeleteEvent(), deleteCommand);
  }
}

void mitk::DataStorage::RemoveListeners(const DataNode *_Node)
{
  itk::MutexLockHolder<itk::SimpleFastMutexLock> locked(m_MutexOne);
  // node must not be 0 and must be registered
  auto *NonConstNode = const_cast<DataNode *>(_Node);
  if (_Node && m_NodeModifiedObserverTags.find(NonConstNode) != m_NodeModifiedObserverTags.end())
  {
    // const cast is bad! but sometimes it is necessary. removing an observer does not really
    // touch the internal state
    NonConstNode->RemoveObserver(m_NodeModifiedObserverTags.find(NonConstNode)->second);
    NonConstNode->RemoveObserver(m_NodeDeleteObserverTags.find(NonConstNode)->second);
    NonConstNode->RemoveObserver(m_NodeInteractorChangedObserverTags.find(NonConstNode)->second);

    m_NodeModifiedObserverTags.erase(NonConstNode);
    m_NodeDeleteObserverTags.erase(NonConstNode);
    m_NodeInteractorChangedObserverTags.erase(NonConstNode);
  }
}

mitk::TimeGeometry::ConstPointer mitk::DataStorage::ComputeBoundingGeometry3D(const SetOfObjects *input,
                                                                              const char *boolPropertyKey,
                                                                              const BaseRenderer *renderer,
                                                                              const char *boolPropertyKey2) const
{
  if (input == nullptr)
    throw std::invalid_argument("DataStorage: input is invalid");

  BoundingBox::PointsContainer::Pointer pointscontainer = BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid = 0;
  Point3D point;

  Vector3D minSpacing;
  minSpacing.Fill(itk::NumericTraits<ScalarType>::max());

  ScalarType stmax = itk::NumericTraits<ScalarType>::max();
  ScalarType stmin = itk::NumericTraits<ScalarType>::NonpositiveMin();

  std::set<ScalarType> existingTimePoints;
  ScalarType maximalTime = 0;

  // Needed for check of zero bounding boxes
  ScalarType nullpoint[] = {0, 0, 0, 0, 0, 0};
  BoundingBox::BoundsArrayType itkBoundsZero(nullpoint);

  for (SetOfObjects::ConstIterator it = input->Begin(); it != input->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if ((node.IsNotNull()) && (node->GetData() != nullptr) && (node->GetData()->IsEmpty() == false) &&
        node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer))
    {
      const TimeGeometry *timeGeometry = node->GetData()->GetUpdatedTimeGeometry();

      if (timeGeometry != nullptr)
      {
        // bounding box (only if non-zero)
        BoundingBox::BoundsArrayType itkBounds = timeGeometry->GetBoundingBoxInWorld()->GetBounds();
        if (itkBounds == itkBoundsZero)
        {
          continue;
        }

        unsigned char i;
        for (i = 0; i < 8; ++i)
        {
          point = timeGeometry->GetCornerPointInWorld(i);
          if (point[0] * point[0] + point[1] * point[1] + point[2] * point[2] < large)
            pointscontainer->InsertElement(pointid++, point);
          else
          {
            itkGenericOutputMacro(<< "Unrealistically distant corner point encountered. Ignored. Node: " << node);
          }
        }
        try
        {
          // time bounds
          // iterate over all time steps
          // Attention: Objects with zero bounding box are not respected in time bound calculation
          for (TimeStepType i = 0; i < timeGeometry->CountTimeSteps(); i++)
          {
            // We must not use 'node->GetData()->GetGeometry(i)->GetSpacing()' here, as it returns the spacing
            // in its original space, which, in case of an image geometry, can have the values in different
            // order than in world space. For the further calculations, we need to have the spacing values
            // in world coordinate order (sag-cor-ax).
            Vector3D spacing;
            spacing.Fill(1.0);
            node->GetData()->GetGeometry(i)->IndexToWorld(spacing, spacing);
            for (int axis = 0; axis < 3; ++ axis)
            {
              ScalarType space = std::abs(spacing[axis]);
              if (space < minSpacing[axis])
              {
                minSpacing[axis] = space;
              }
            }

            const auto curTimeBounds = timeGeometry->GetTimeBounds(i);
            if ((curTimeBounds[0] > stmin) && (curTimeBounds[0] < stmax))
            {
              existingTimePoints.insert(curTimeBounds[0]);
            }
            if ((curTimeBounds[1] > maximalTime) && (curTimeBounds[1] < stmax))
            {
               maximalTime = curTimeBounds[1];
            }
          }
        }
        catch (itk::ExceptionObject &e)
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
  if (existingTimePoints.empty()) // make sure that there is at least one time sliced geometry in the data storage
  {
    existingTimePoints.insert(0.0);
    maximalTime = 1.0;
  }

  ArbitraryTimeGeometry::Pointer timeGeometry = nullptr;
  if (result->GetPoints()->Size() > 0)
  {
    // Initialize a geometry of a single time step
    Geometry3D::Pointer geometry = Geometry3D::New();
    geometry->Initialize();
    // correct bounding-box (is now in mm, should be in index-coordinates)
    // according to spacing
    BoundingBox::BoundsArrayType bounds = result->GetBounds();
    AffineTransform3D::OutputVectorType offset;
    for (int i = 0; i < 3; ++i)
    {
      offset[i] = bounds[i * 2];
      bounds[i * 2] = 0.0;
      bounds[i * 2 + 1] = (bounds[i * 2 + 1] - offset[i]) / minSpacing[i];
    }
    geometry->GetIndexToWorldTransform()->SetOffset(offset);
    geometry->SetBounds(bounds);
    geometry->SetSpacing(minSpacing);

    // Initialize the time sliced geometry
    auto tsIterator = existingTimePoints.cbegin();
    auto tsPredecessor = tsIterator++;
    auto tsEnd = existingTimePoints.cend();
    timeGeometry = ArbitraryTimeGeometry::New();
    for (; tsIterator != tsEnd; ++tsIterator, ++tsPredecessor)
    {
      timeGeometry->AppendNewTimeStep(geometry, *tsPredecessor, *tsIterator);
    }
    timeGeometry->AppendNewTimeStep(geometry, *tsPredecessor, maximalTime);

    timeGeometry->Update();
  }
  return timeGeometry.GetPointer();
}

mitk::TimeGeometry::ConstPointer mitk::DataStorage::ComputeBoundingGeometry3D(const char *boolPropertyKey,
                                                                              const BaseRenderer *renderer,
                                                                              const char *boolPropertyKey2) const
{
  return this->ComputeBoundingGeometry3D(this->GetAll(), boolPropertyKey, renderer, boolPropertyKey2);
}

mitk::TimeGeometry::ConstPointer mitk::DataStorage::ComputeVisibleBoundingGeometry3D(const BaseRenderer *renderer,
                                                                                     const char *boolPropertyKey)
{
  return ComputeBoundingGeometry3D("visible", renderer, boolPropertyKey);
}

mitk::BoundingBox::Pointer mitk::DataStorage::ComputeBoundingBox(const char *boolPropertyKey,
                                                                 const BaseRenderer *renderer,
                                                                 const char *boolPropertyKey2)
{
  BoundingBox::PointsContainer::Pointer pointscontainer = BoundingBox::PointsContainer::New();

  BoundingBox::PointIdentifier pointid = 0;
  Point3D point;

  // Needed for check of zero bounding boxes
  ScalarType nullpoint[] = {0, 0, 0, 0, 0, 0};
  BoundingBox::BoundsArrayType itkBoundsZero(nullpoint);

  SetOfObjects::ConstPointer all = this->GetAll();
  for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if ((node.IsNotNull()) && (node->GetData() != nullptr) && (node->GetData()->IsEmpty() == false) &&
        node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer))
    {
      const TimeGeometry *geometry = node->GetData()->GetUpdatedTimeGeometry();
      if (geometry != nullptr)
      {
        // bounding box (only if non-zero)
        BoundingBox::BoundsArrayType itkBounds = geometry->GetBoundingBoxInWorld()->GetBounds();
        if (itkBounds == itkBoundsZero)
        {
          continue;
        }

        unsigned char i;
        for (i = 0; i < 8; ++i)
        {
          point = geometry->GetCornerPointInWorld(i);
          if (point[0] * point[0] + point[1] * point[1] + point[2] * point[2] < large)
            pointscontainer->InsertElement(pointid++, point);
          else
          {
            itkGenericOutputMacro(<< "Unrealistically distant corner point encountered. Ignored. Node: " << node);
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

mitk::TimeBounds mitk::DataStorage::ComputeTimeBounds(const char *boolPropertyKey,
                                                      const BaseRenderer *renderer,
                                                      const char *boolPropertyKey2)
{
  TimeBounds timeBounds;

  ScalarType stmin, stmax, cur;

  stmin = itk::NumericTraits<ScalarType>::NonpositiveMin();
  stmax = itk::NumericTraits<ScalarType>::max();

  timeBounds[0] = stmax;
  timeBounds[1] = stmin;

  SetOfObjects::ConstPointer all = this->GetAll();
  for (SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if ((node.IsNotNull()) && (node->GetData() != nullptr) && (node->GetData()->IsEmpty() == false) &&
        node->IsOn(boolPropertyKey, renderer) && node->IsOn(boolPropertyKey2, renderer))
    {
      const TimeGeometry *geometry = node->GetData()->GetUpdatedTimeGeometry();
      if (geometry != nullptr)
      {
        const TimeBounds &curTimeBounds = geometry->GetTimeBounds();
        cur = curTimeBounds[0];
        // is it after -infinity, but before everything else that we found until now?
        if ((cur > stmin) && (cur < timeBounds[0]))
          timeBounds[0] = cur;

        cur = curTimeBounds[1];
        // is it before infinity, but after everything else that we found until now?
        if ((cur < stmax) && (cur > timeBounds[1]))
          timeBounds[1] = cur;
      }
    }
  }
  if (!(timeBounds[0] < stmax))
  {
    timeBounds[0] = stmin;
    timeBounds[1] = stmax;
  }
  return timeBounds;
}

void mitk::DataStorage::BlockNodeModifiedEvents(bool block)
{
  m_BlockNodeModifiedEvents = block;
}

mitk::DataNode::Pointer mitk::FindTopmostVisibleNode(const DataStorage::SetOfObjects::ConstPointer nodes,
                                                     const Point3D worldPosition,
                                                     const TimePointType timePoint,
                                                     const BaseRenderer* baseRender)
{
   if (nodes.IsNull())
  {
    return nullptr;
  }

  mitk::DataNode::Pointer topLayerNode = nullptr;
  int maxLayer = std::numeric_limits<int>::min();

  for (auto node : *nodes)
  {
    if (node.IsNull())
    {
      continue;
    }

    bool isHelperObject = false;
    node->GetBoolProperty("helper object", isHelperObject);
    if (isHelperObject)
    {
      continue;
    }

    auto data = node->GetData();
    if (nullptr == data)
    {
      continue;
    }

    auto geometry = data->GetGeometry();
    if (nullptr == geometry || !geometry->IsInside(worldPosition))
    {
      continue;
    }

    auto timeGeometry = data->GetUpdatedTimeGeometry();
    if (nullptr == timeGeometry)
    {
      continue;
    }

    if (!timeGeometry->IsValidTimePoint(timePoint))
    {
      continue;
    }

    int layer = 0;
    if (!node->GetIntProperty("layer", layer, baseRender))
    {
      continue;
    }

    if (layer <= maxLayer)
    {
      continue;
    }

    if (!node->IsVisible(baseRender))
    {
      continue;
    }

    topLayerNode = node;
    maxLayer = layer;
  }

  return topLayerNode;
}
