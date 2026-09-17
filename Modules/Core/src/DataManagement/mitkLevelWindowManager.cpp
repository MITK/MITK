/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLevelWindowManager.h>

#include <mitkImage.h>
#include <mitkMessage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingModeProperty.h>
#include <itkCommand.h>

namespace
{
  bool HasLevelWindowRenderingMode(mitk::DataNode *node)
  {
    auto *mode = dynamic_cast<mitk::RenderingModeProperty *>(node->GetProperty("Image Rendering.Mode"));

    if (nullptr == mode)
      return false;

    const int renderingMode = mode->GetRenderingMode();

    return renderingMode == mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR ||
           renderingMode == mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_LEVELWINDOW_COLOR;
  }

  // Only visible nodes that are rendered with a level window can be the current node.
  bool IsCandidate(mitk::DataNode *node)
  {
    return node->IsVisible(nullptr) && HasLevelWindowRenderingMode(node);
  }

  mitk::LevelWindowProperty *GetNodeLevelWindowProperty(const mitk::DataNode *node)
  {
    return dynamic_cast<mitk::LevelWindowProperty *>(node->GetProperty("levelwindow"));
  }

  template <typename TObserverMap>
  void ClearObserverMap(TObserverMap &observerMap)
  {
    for (auto &[tagAndNode, property] : observerMap)
      property->RemoveObserver(tagAndNode.first);

    observerMap.clear();
  }
}

mitk::LevelWindowManager::LevelWindowManager()
{
}

mitk::LevelWindowManager::~LevelWindowManager()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
      MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageAddedNode));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageRemovedNode));
    m_DataStorage = nullptr;
  }

  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  this->ClearPropertyObserverMaps();
}

void mitk::LevelWindowManager::SetDataStorage(DataStorage *dataStorage)
{
  if (nullptr == dataStorage)
    return;

  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
      MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageAddedNode));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageRemovedNode));
  }

  m_DataStorage = dataStorage;
  m_DataStorage->AddNodeEvent.AddListener(
    MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageAddedNode));
  m_DataStorage->RemoveNodeEvent.AddListener(
    MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageRemovedNode));

  this->DataStorageAddedNode();
}

mitk::DataStorage *mitk::LevelWindowManager::GetDataStorage()
{
  return m_DataStorage.GetPointer();
}

void mitk::LevelWindowManager::SetMode(Mode mode)
{
  if (mode == m_Mode)
    return;

  if (Mode::ExplicitImage == mode)
  {
    m_ModeBeforeExplicitImage = m_Mode;
    m_ExplicitNode = m_CurrentNode;
  }
  else
  {
    m_ExplicitNode = nullptr;
  }

  m_Mode = mode;

  this->Recompute();
}

mitk::LevelWindowManager::Mode mitk::LevelWindowManager::GetMode() const
{
  return m_Mode;
}

void mitk::LevelWindowManager::SetApplyToAllSelectedImages(bool apply)
{
  if (apply == m_ApplyToAllSelectedImages)
    return;

  m_ApplyToAllSelectedImages = apply;

  this->Recompute();
}

bool mitk::LevelWindowManager::GetApplyToAllSelectedImages() const
{
  return m_ApplyToAllSelectedImages;
}

void mitk::LevelWindowManager::RecalculateLevelWindowForSelectedComponent(const itk::EventObject &event)
{
  DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if (node.IsNull())
      continue;

    bool isSelected = false;
    node->GetBoolProperty("selected", isSelected);
    if (isSelected)
    {
      LevelWindow selectedLevelWindow;
      node->GetLevelWindow(selectedLevelWindow); // node is an image node because of predicates

      auto *image = dynamic_cast<Image *>(node->GetData());
      int displayedComponent = 0;
      if (image && (node->GetIntProperty("Image.Displayed Component", displayedComponent)))
      { // we found a selected image with a displayed component
        // let's recalculate the levelwindow for this.
        selectedLevelWindow.SetAuto(image, true, true, static_cast<unsigned>(displayedComponent));
        node->SetLevelWindow(selectedLevelWindow);
      }
    }
  }

  this->Update(event);
}

void mitk::LevelWindowManager::Update(const itk::EventObject &)
{
  this->Recompute();
}

void mitk::LevelWindowManager::Recompute(const DataNode *removedNode)
{
  if (m_DataStorage.IsNull())
    return;

  m_DataNodesForLevelWindow.clear();

  if (Mode::ExplicitImage == m_Mode &&
      (m_ExplicitNode.IsNull() || m_ExplicitNode.GetPointer() == removedNode || !m_DataStorage->Exists(m_ExplicitNode)))
  {
    m_ExplicitNode = nullptr;
    m_Mode = m_ModeBeforeExplicitImage;
  }

  DataNode *node = nullptr;

  switch (m_Mode)
  {
    case Mode::TopMostImage:
      node = this->FindTopMostVisibleNode(false, removedNode);
      break;

    case Mode::SelectedImage:
      node = this->FindTopMostVisibleNode(true, removedNode);

      if (nullptr != node && m_ApplyToAllSelectedImages)
      {
        auto relevantNodes = this->GetRelevantNodes();
        for (auto it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
        {
          DataNode *otherNode = it->Value();

          if (nullptr == otherNode || otherNode == node || otherNode == removedNode)
            continue;

          if (otherNode->IsSelected() && IsCandidate(otherNode))
            m_DataNodesForLevelWindow.push_back(otherNode);
        }
      }
      break;

    case Mode::ExplicitImage:
      // The pin survives a hidden node; a hidden widget would leave no way to change the mode via its context menu.
      node = IsCandidate(m_ExplicitNode) ? m_ExplicitNode.GetPointer() : this->FindTopMostVisibleNode(false, removedNode);
      break;
  }

  this->SetCurrentNode(node);
}

mitk::DataNode *mitk::LevelWindowManager::FindTopMostVisibleNode(bool selectedOnly, const DataNode *excludedNode) const
{
  DataNode *topMostNode = nullptr;
  int maxLayer = itk::NumericTraits<int>::min();

  auto relevantNodes = this->GetRelevantNodes();
  for (auto it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
  {
    DataNode *node = it->Value();

    if (nullptr == node || node == excludedNode || !IsCandidate(node))
      continue;

    if (selectedOnly && !node->IsSelected())
      continue;

    int layer = -1;
    node->GetIntProperty("layer", layer);

    if (layer <= maxLayer)
      continue;

    topMostNode = node;
    maxLayer = layer;
  }

  return topMostNode;
}

void mitk::LevelWindowManager::SetCurrentNode(DataNode *node)
{
  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  m_LevelWindowProperty = nullptr != node ? GetNodeLevelWindowProperty(node) : nullptr;
  m_CurrentNode = m_LevelWindowProperty.IsNotNull() ? node : nullptr;

  if (m_LevelWindowProperty.IsNotNull())
  {
    auto command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
    m_PropertyModifiedTag = m_LevelWindowProperty->AddObserver(itk::ModifiedEvent(), command);
    m_IsPropertyModifiedTagSet = true;
  }

  auto relevantNodes = this->GetRelevantNodes();
  for (auto it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
  {
    DataNode *relevantNode = it->Value();

    if (nullptr != relevantNode)
      relevantNode->SetBoolProperty("imageForLevelWindow", relevantNode == m_CurrentNode.GetPointer());
  }

  this->Modified();
}

void mitk::LevelWindowManager::SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty)
{
  if (levelWindowProperty.IsNull())
    return;

  if (m_DataStorage.IsNull())
    mitkThrow() << "DataStorage not set";

  DataNode *propertyNode = nullptr;

  auto relevantNodes = this->GetRelevantNodes();
  for (auto it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
  {
    DataNode *node = it->Value();

    if (nullptr != node && GetNodeLevelWindowProperty(node) == levelWindowProperty.GetPointer())
    {
      propertyNode = node;
      break;
    }
  }

  if (nullptr == propertyNode)
    mitkThrow() << "No image in the data storage that belongs to level-window property " << levelWindowProperty.GetPointer();

  if (Mode::ExplicitImage != m_Mode)
    m_ModeBeforeExplicitImage = m_Mode;

  m_ExplicitNode = propertyNode;
  m_Mode = Mode::ExplicitImage;

  this->Recompute();
}

void mitk::LevelWindowManager::SetLevelWindow(const LevelWindow &levelWindow)
{
  if (m_LevelWindowProperty.IsNull())
    return;

  m_LevelWindowProperty->SetLevelWindow(levelWindow);

  for (const auto &node : m_DataNodesForLevelWindow)
  {
    auto *property = GetNodeLevelWindowProperty(node);

    if (nullptr != property)
      property->SetLevelWindow(levelWindow);
  }

  this->Modified();
}

mitk::LevelWindowProperty::Pointer mitk::LevelWindowManager::GetLevelWindowProperty() const
{
  return m_LevelWindowProperty;
}

const mitk::LevelWindow &mitk::LevelWindowManager::GetLevelWindow() const
{
  if (m_LevelWindowProperty.IsNull())
    mitkThrow() << "No LevelWindow available!";

  return m_LevelWindowProperty->GetLevelWindow();
}

void mitk::LevelWindowManager::DataStorageAddedNode(const DataNode *)
{
  this->UpdateObservers();
  this->Recompute();

  if ((m_ObserverToVisibleProperty.size() != m_ObserverToLayerProperty.size()) ||
      (m_ObserverToLayerProperty.size() != this->GetRelevantNodes()->size()))
  {
    mitkThrow() << "Wrong number of observers in Level Window Manager!";
  }
}

void mitk::LevelWindowManager::DataStorageRemovedNode(const DataNode *removedNode)
{
  // The node is still part of the data storage while this listener runs.
  auto relevantNodes = this->GetRelevantNodes();

  bool removedNodeIsRelevant = false;
  for (auto it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
  {
    if (it->Value().GetPointer() == removedNode)
    {
      removedNodeIsRelevant = true;
      break;
    }
  }

  if (!removedNodeIsRelevant)
    return;

  this->UpdateObservers(removedNode);
  this->Recompute(removedNode);

  if ((m_ObserverToVisibleProperty.size() != m_ObserverToLayerProperty.size()) ||
      (m_ObserverToLayerProperty.size() != (relevantNodes->size() - 1)))
  {
    mitkThrow() << "Wrong number of observers in Level Window Manager!";
  }
}

void mitk::LevelWindowManager::OnPropertyModified(const itk::EventObject &)
{
  this->Modified();
}

void mitk::LevelWindowManager::OnSelectedPropertyModified(const itk::EventObject &)
{
  if (Mode::SelectedImage == m_Mode)
    this->Recompute();
}

void mitk::LevelWindowManager::OnNodeNameModified(const itk::EventObject &)
{
  this->Modified();
}

mitk::DataNode *mitk::LevelWindowManager::GetCurrentNode() const
{
  return m_CurrentNode.GetPointer();
}

mitk::Image *mitk::LevelWindowManager::GetCurrentImage() const
{
  return m_CurrentNode.IsNotNull() ? dynamic_cast<Image *>(m_CurrentNode->GetData()) : nullptr;
}

int mitk::LevelWindowManager::GetNumberOfObservers() const
{
  return static_cast<int>(m_ObserverToVisibleProperty.size());
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::LevelWindowManager::GetRelevantNodes() const
{
  if (m_DataStorage.IsNull())
    return DataStorage::SetOfObjects::ConstPointer(DataStorage::SetOfObjects::New());

  auto notBinary = NodePredicateProperty::New("binary", BoolProperty::New(false));
  auto hasLevelWindow = NodePredicateProperty::New("levelwindow", nullptr);

  auto isImage = NodePredicateDataType::New("Image");
  auto isDImage = NodePredicateDataType::New("DiffusionImage");
  auto isTImage = NodePredicateDataType::New("TensorImage");
  auto isOdfImage = NodePredicateDataType::New("OdfImage");
  auto isShImage = NodePredicateDataType::New("ShImage");
  auto predicateTypes = NodePredicateOr::New();
  predicateTypes->AddPredicate(isImage);
  predicateTypes->AddPredicate(isDImage);
  predicateTypes->AddPredicate(isTImage);
  predicateTypes->AddPredicate(isOdfImage);
  predicateTypes->AddPredicate(isShImage);

  NodePredicateAnd::Pointer predicate = NodePredicateAnd::New();
  predicate->AddPredicate(notBinary);
  predicate->AddPredicate(hasLevelWindow);
  predicate->AddPredicate(predicateTypes);

  return m_DataStorage->GetSubset(predicate);
}

void mitk::LevelWindowManager::UpdateObservers(const DataNode *excludedNode)
{
  this->ClearPropertyObserverMaps();
  this->CreatePropertyObserverMaps(excludedNode);
}

void mitk::LevelWindowManager::ClearPropertyObserverMaps()
{
  ClearObserverMap(m_ObserverToVisibleProperty);
  ClearObserverMap(m_ObserverToLayerProperty);
  ClearObserverMap(m_ObserverToRenderingModeProperty);
  ClearObserverMap(m_ObserverToDisplayedComponentProperty);
  ClearObserverMap(m_ObserverToSelectedProperty);
  ClearObserverMap(m_ObserverToNameProperty);
}

void mitk::LevelWindowManager::CreatePropertyObserverMaps(const DataNode *excludedNode)
{
  if (m_DataStorage.IsNull())
    mitkThrow() << "DataStorage not set";

  auto relevantNodes = this->GetRelevantNodes();
  for (auto it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
  {
    DataNode *node = it->Value();

    if (nullptr == node || node == excludedNode)
      continue;

    if (nullptr == node->GetProperty("selected"))
      node->SetBoolProperty("selected", false);

    this->ObserveProperty(node, "visible", &LevelWindowManager::Update, m_ObserverToVisibleProperty);
    this->ObserveProperty(node, "layer", &LevelWindowManager::Update, m_ObserverToLayerProperty);
    this->ObserveProperty(node, "Image Rendering.Mode", &LevelWindowManager::Update, m_ObserverToRenderingModeProperty);
    this->ObserveProperty(node, "Image.Displayed Component", &LevelWindowManager::RecalculateLevelWindowForSelectedComponent, m_ObserverToDisplayedComponentProperty);
    this->ObserveProperty(node, "selected", &LevelWindowManager::OnSelectedPropertyModified, m_ObserverToSelectedProperty);
    this->ObserveProperty(node, "name", &LevelWindowManager::OnNodeNameModified, m_ObserverToNameProperty);
  }
}

void mitk::LevelWindowManager::ObserveProperty(DataNode *node,
                                               const char *propertyKey,
                                               void (LevelWindowManager::*callback)(const itk::EventObject &),
                                               ObserverToPropertyValueMap &observerMap)
{
  BaseProperty *property = node->GetProperty(propertyKey);

  if (nullptr == property)
    return;

  auto command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
  command->SetCallbackFunction(this, callback);

  const unsigned long tag = property->AddObserver(itk::ModifiedEvent(), command);
  observerMap[PropDataPair(tag, node)] = property;
}
