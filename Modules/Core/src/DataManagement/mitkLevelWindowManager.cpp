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

#include "mitkLevelWindowManager.h"
#include "mitkDataStorage.h"
#include "mitkImage.h"
#include "mitkMessage.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProperties.h"
#include "mitkRenderingModeProperty.h"
#include <itkCommand.h>

mitk::LevelWindowManager::LevelWindowManager()
  : m_DataStorage(nullptr)
  , m_LevelWindowProperty(nullptr)
  , m_AutoTopMost(true)
  , m_SelectedImagesMode(false)
  , m_IsObserverTagSet(false)
  , m_CurrentImage(nullptr)
  , m_IsPropertyModifiedTagSet(false)
  , m_LevelWindowMutex(false)
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

  // clear both observer maps
  this->ClearPropObserverLists();
}

void mitk::LevelWindowManager::SetDataStorage(DataStorage *ds)
{
  if (ds == nullptr)
    return;

  /* remove listeners of old DataStorage */
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
      MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageAddedNode));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
      MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageRemovedNode));
  }

  /* register listener for new DataStorage */
  m_DataStorage = ds; // register
  m_DataStorage->AddNodeEvent.AddListener(
    MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageAddedNode));
  m_DataStorage->RemoveNodeEvent.AddListener(
    MessageDelegate1<LevelWindowManager, const DataNode *>(this, &LevelWindowManager::DataStorageRemovedNode));

  this->DataStorageAddedNode(); // update us with new DataStorage
}

mitk::DataStorage *mitk::LevelWindowManager::GetDataStorage()
{
  return m_DataStorage.GetPointer();
}

void mitk::LevelWindowManager::SetAutoTopMostImage(bool autoTopMost, const DataNode *removedNode/* = nullptr*/)
{
  m_AutoTopMost = autoTopMost;
  if (false == m_AutoTopMost)
  {
    return;
  }

  // deactivate other mode
  m_SelectedImagesMode = false;

  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  // find topmost image in the data storage
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "DataStorage not set";
  }

  DataNode::Pointer topLevelNode;
  int maxVisibleLayer = itk::NumericTraits<int>::min();
  m_LevelWindowProperty = nullptr;
  m_CurrentImage = nullptr;

  DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if (node.IsNull() || node == removedNode)
    {
      continue;
    }

    m_LevelWindowMutex = true;
    node->SetBoolProperty("imageForLevelWindow", false);
    m_LevelWindowMutex = false;

    if (false == node->IsVisible(nullptr))
    {
      continue;
    }

    int layer = -1;
    node->GetIntProperty("layer", layer);
    if (layer < maxVisibleLayer)
    {
      continue;
    }

    bool ignore = this->IgnoreNode(node);
    if (ignore)
    {
      continue;
    }

    m_LevelWindowProperty = dynamic_cast<LevelWindowProperty*>(node->GetProperty("levelwindow"));
    topLevelNode = node;
    maxVisibleLayer = layer;
  }

  // this will set the "imageForLevelWindow" property and the 'm_CurrentImage' and call 'Modified()'
  this->SetLevelWindowProperty(m_LevelWindowProperty);

  if (m_LevelWindowProperty.IsNull())
  {
    this->Modified();
  }
}

void mitk::LevelWindowManager::SetSelectedImages(bool selectedImagesMode, const DataNode *removedNode/* = nullptr*/)
{
  m_SelectedImagesMode = selectedImagesMode;
  if (false == m_SelectedImagesMode)
  {
    return;
  }

  // deactivate other mode
  m_AutoTopMost = false;

  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  // find selected images in the data storage
  if (m_DataStorage.IsNull())
  {
    mitkThrow() << "DataStorage not set";
  }

  DataNode::Pointer lastSelectedNode;
  m_LevelWindowProperty = nullptr;
  m_CurrentImage = nullptr;

  DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if (node.IsNull() || node == removedNode)
    {
      continue;
    }

    m_LevelWindowMutex = true;
    node->SetBoolProperty("imageForLevelWindow", false);
    m_LevelWindowMutex = false;

    if (false == node->IsSelected())
    {
      continue;
    }

    bool ignore = this->IgnoreNode(node);
    if (ignore)
    {
      continue;
    }

    m_LevelWindowProperty = dynamic_cast<LevelWindowProperty*>(node->GetProperty("levelwindow"));
    m_RelevantDataNodes.push_back(node);
    lastSelectedNode = node;
  }

  // this will set the "imageForLevelWindow" property and the 'm_CurrentImage' and call 'Modified()'
  this->SetLevelWindowProperty(m_LevelWindowProperty);

  if (m_LevelWindowProperty.IsNull())
  {
    this->Modified();
  }
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

    LevelWindow levelWindow;
    node->GetLevelWindow(levelWindow);
  }

  this->Update(event);
}

void mitk::LevelWindowManager::Update(const itk::EventObject &)
{
  if (m_LevelWindowMutex) // no mutex, should still help
  {
    return;
  }

  m_RelevantDataNodes.clear();
  if (m_AutoTopMost)
  {
    this->SetAutoTopMostImage(true);
    return;
  }

  if (m_SelectedImagesMode)
  {
    this->SetSelectedImages(true);
    return;
  }

  int maxVisibleLayer = itk::NumericTraits<int>::min();
  DataNode::Pointer topLevelNode = nullptr;
  std::vector<DataNode::Pointer> nodesForLevelWindow;

  DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it->Value();

    if (node.IsNull())
    {
      continue;
    }

    if (node->IsVisible(nullptr) == false)
    {
      continue;
    }

    bool prop = false;
    node->GetBoolProperty("imageForLevelWindow", prop);
    if (prop)
    {
      nodesForLevelWindow.push_back(node);
      continue;
    }

    int layer = -1;
    node->GetIntProperty("layer", layer);
    if (layer > maxVisibleLayer)
    {
      // top level node is backup node, if no node with
      // "imageForLevelWindow" property with value "true" is found
      topLevelNode = node;
      maxVisibleLayer = layer;
    }
  }

  int nodesForLevelWindowSize = nodesForLevelWindow.size();
  if (nodesForLevelWindowSize > 2)
  {
    MITK_ERROR << "Error: not more than two visible nodes are expected to have the imageForLevelWindow property set at "
                  "any point.";
  }

  if (nodesForLevelWindowSize > 0)
  {
    // 1 or 2 nodes for level window found
    for (const auto& node : nodesForLevelWindow)
    {
      LevelWindowProperty::Pointer newProp = dynamic_cast<LevelWindowProperty *>(node->GetProperty("levelwindow"));
      if (newProp != m_LevelWindowProperty)
      {
        this->SetLevelWindowProperty(newProp);
        return;
      }
    }
  }
  else if (topLevelNode)
  {
    // no nodes for level window found
    LevelWindowProperty::Pointer lvlProp = dynamic_cast<LevelWindowProperty *>(topLevelNode->GetProperty("levelwindow"));
    this->SetLevelWindowProperty(lvlProp);
  }
  else
  {
    // no nodes for level window found and no visible top level node found
    this->Modified();
  }
}

void mitk::LevelWindowManager::UpdateSelected(const itk::EventObject &)
{
  if (m_LevelWindowMutex) // no mutex, should still help
  {
    return;
  }

  m_RelevantDataNodes.clear();
  if (m_SelectedImagesMode)
  {
    this->SetSelectedImages(true);
  }
}

void mitk::LevelWindowManager::SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty)
{
  if (levelWindowProperty.IsNull())
  {
    return;
  }

  // find data node that belongs to the property
  DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  mitk::DataNode::Pointer propNode = nullptr;
  for (DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    DataNode::Pointer node = it.Value();
    LevelWindowProperty::Pointer property = dynamic_cast<LevelWindowProperty *>(node->GetProperty("levelwindow"));
    if (property == levelWindowProperty)
    {
      propNode = node;
    }
    else
    {
      m_LevelWindowMutex = true;
      node->SetBoolProperty("imageForLevelWindow", false);
      m_LevelWindowMutex = false;
    }
  }

  if (propNode.IsNull())
  {
    mitkThrow() << "No Image in the data storage that belongs to level-window property " << m_LevelWindowProperty;
  }

  if (m_IsPropertyModifiedTagSet) // remove listener for old property
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  m_LevelWindowProperty = levelWindowProperty;

  itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command =
    itk::ReceptorMemberCommand<LevelWindowManager>::New(); // register listener for new property
  command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
  m_PropertyModifiedTag = m_LevelWindowProperty->AddObserver(itk::ModifiedEvent(), command);
  m_IsPropertyModifiedTagSet = true;

  m_CurrentImage = dynamic_cast<Image *>(propNode->GetData());

  m_LevelWindowMutex = true;
  propNode->SetBoolProperty("imageForLevelWindow", true);
  m_LevelWindowMutex = false;

  this->Modified();
}

void mitk::LevelWindowManager::SetLevelWindow(const LevelWindow &levelWindow)
{
  if (m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->SetLevelWindow(levelWindow);
    for (const auto &dataNode : m_RelevantDataNodes)
    {
      auto levelWindowProperty = dynamic_cast<LevelWindowProperty *>(dataNode->GetProperty("levelwindow"));
      if (nullptr == levelWindowProperty)
      {
        continue;
      }

      levelWindowProperty->SetLevelWindow(levelWindow);
    }
  }
  this->Modified();
}

mitk::LevelWindowProperty::Pointer mitk::LevelWindowManager::GetLevelWindowProperty()
{
  return m_LevelWindowProperty;
}

const mitk::LevelWindow &mitk::LevelWindowManager::GetLevelWindow()
{
  if (m_LevelWindowProperty.IsNotNull())
  {
    return m_LevelWindowProperty->GetLevelWindow();
  }
  else
  {
    mitkThrow() << "No LevelWindow available!";
  }
}

bool mitk::LevelWindowManager::IsAutoTopMost()
{
  return m_AutoTopMost;
}

bool mitk::LevelWindowManager::IsSelectedImages()
{
  return m_SelectedImagesMode;
}

void mitk::LevelWindowManager::DataStorageAddedNode(const DataNode *)
{
  // update observers with new data storage
  this->UpdateObservers();

  // Initialize LevelWindowsManager to new image
  this->SetAutoTopMostImage(true);

  // check if everything is still ok
  if ((m_ObserverToVisibleProperty.size() != m_ObserverToLayerProperty.size()) ||
      (m_ObserverToLayerProperty.size() != this->GetRelevantNodes()->size()))
  {
    mitkThrow() << "Wrong number of observers in Level Window Manager!";
  }
}

void mitk::LevelWindowManager::DataStorageRemovedNode(const DataNode *removedNode)
{
  // First: check if deleted node is part of relevant nodes.
  // If not, abort method because there is no need change anything.
  bool removedNodeIsRelevant = false;
  DataStorage::SetOfObjects::ConstPointer relevantNodes = GetRelevantNodes();
  for (DataStorage::SetOfObjects::ConstIterator it = relevantNodes->Begin(); it != relevantNodes->End(); ++it)
  {
    if (it->Value() == removedNode)
    {
      removedNodeIsRelevant = true;
    }
  }

  if (false == removedNodeIsRelevant)
  {
    return;
  }

  // remember node which will be removed
  m_NodeMarkedToDelete = removedNode;

  // update observers
  this->UpdateObservers();

  // search image that belongs to the property
  if (m_LevelWindowProperty.IsNull())
  {
    this->SetAutoTopMostImage(true, removedNode);
  }
  else
  {
    NodePredicateProperty::Pointer property = NodePredicateProperty::New("levelwindow", m_LevelWindowProperty);
    DataNode *n = m_DataStorage->GetNode(property);
    if (n == nullptr || m_AutoTopMost) // if node was deleted, change our behavior to AutoTopMost, if AutoTopMost is
                                       // true change level window to topmost node
    {
      this->SetAutoTopMostImage(true, removedNode);
    }
  }

  // reset variable
  m_NodeMarkedToDelete = nullptr;

  // check if everything is still ok
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

mitk::Image *mitk::LevelWindowManager::GetCurrentImage()
{
  return m_CurrentImage;
}

int mitk::LevelWindowManager::GetNumberOfObservers()
{
  return m_ObserverToVisibleProperty.size();
}

mitk::DataStorage::SetOfObjects::ConstPointer mitk::LevelWindowManager::GetRelevantNodes()
{
  if (m_DataStorage.IsNull())
  {
    return DataStorage::SetOfObjects::ConstPointer(DataStorage::SetOfObjects::New());
  }

  NodePredicateProperty::Pointer notBinary = NodePredicateProperty::New("binary", BoolProperty::New(false));
  NodePredicateProperty::Pointer hasLevelWindow = NodePredicateProperty::New("levelwindow", nullptr);

  NodePredicateDataType::Pointer isImage = NodePredicateDataType::New("Image");
  NodePredicateDataType::Pointer isDImage = NodePredicateDataType::New("DiffusionImage");
  NodePredicateDataType::Pointer isTImage = NodePredicateDataType::New("TensorImage");
  NodePredicateDataType::Pointer isOdfImage = NodePredicateDataType::New("OdfImage");
  NodePredicateDataType::Pointer isShImage = NodePredicateDataType::New("ShImage");
  NodePredicateOr::Pointer predicateTypes = NodePredicateOr::New();
  predicateTypes->AddPredicate(isImage);
  predicateTypes->AddPredicate(isDImage);
  predicateTypes->AddPredicate(isTImage);
  predicateTypes->AddPredicate(isOdfImage);
  predicateTypes->AddPredicate(isShImage);

  NodePredicateAnd::Pointer predicate = NodePredicateAnd::New();
  predicate->AddPredicate(notBinary);
  predicate->AddPredicate(hasLevelWindow);
  predicate->AddPredicate(predicateTypes);

  DataStorage::SetOfObjects::ConstPointer relevantNodes = m_DataStorage->GetSubset(predicate);

  return relevantNodes;
}

void mitk::LevelWindowManager::UpdateObservers()
{
  this->ClearPropObserverLists();  // remove old observers
  this->CreatePropObserverLists(); // create new observer lists
}

void mitk::LevelWindowManager::ClearPropObserverLists()
{
  for (auto iter = m_ObserverToVisibleProperty.begin(); iter != m_ObserverToVisibleProperty.end(); ++iter)
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = nullptr;
  }
  m_ObserverToVisibleProperty.clear();

  for (auto iter = m_ObserverToLayerProperty.begin(); iter != m_ObserverToLayerProperty.end(); ++iter)
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = nullptr;
  }
  m_ObserverToLayerProperty.clear();

  for (auto iter = m_ObserverToRenderingModeProperty.begin(); iter != m_ObserverToRenderingModeProperty.end(); ++iter)
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = nullptr;
  }
  m_ObserverToRenderingModeProperty.clear();

  for (auto iter = m_ObserverToDisplayedComponentProperty.begin(); iter != m_ObserverToDisplayedComponentProperty.end(); ++iter)
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = nullptr;
  }
  m_ObserverToDisplayedComponentProperty.clear();

  for (auto iter = m_ObserverToLevelWindowImageProperty.begin(); iter != m_ObserverToLevelWindowImageProperty.end(); ++iter)
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = nullptr;
  }
  m_ObserverToLevelWindowImageProperty.clear();

  for (auto iter = m_ObserverToSelectedProperty.begin(); iter != m_ObserverToSelectedProperty.end(); ++iter)
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = nullptr;
  }
  m_ObserverToSelectedProperty.clear();
}

void mitk::LevelWindowManager::CreatePropObserverLists()
{
  if (m_DataStorage.IsNull()) // check if data storage is set
  {
    mitkThrow() << "DataStorage not set";
  }

  /* add observers for all relevant nodes */
  DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if ((it->Value().IsNull()) || (it->Value() == m_NodeMarkedToDelete))
    {
      continue;
    }

    /* register listener for changes in visible property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command =
      itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::Update);
    unsigned long visIdx = it->Value()->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command);
    m_ObserverToVisibleProperty[PropDataPair(visIdx, it->Value())] = it->Value()->GetProperty("visible");

    /* register listener for changes in layer property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command2 =
      itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command2->SetCallbackFunction(this, &LevelWindowManager::Update);
    unsigned long layerIdx = it->Value()->GetProperty("layer")->AddObserver(itk::ModifiedEvent(), command2);
    m_ObserverToLayerProperty[PropDataPair(layerIdx, it->Value())] = it->Value()->GetProperty("layer");

    /* register listener for changes in layer property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command3 =
      itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command3->SetCallbackFunction(this, &LevelWindowManager::Update);
    BaseProperty::Pointer imageRenderingMode = it->Value()->GetProperty("Image Rendering.Mode");
    if (imageRenderingMode.IsNotNull())
    {
      unsigned long rendIdx = imageRenderingMode->AddObserver(itk::ModifiedEvent(), command3);
      m_ObserverToRenderingModeProperty[PropDataPair(rendIdx, it->Value())] = imageRenderingMode.GetPointer();
    }

    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command4 =
      itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command4->SetCallbackFunction(this, &LevelWindowManager::RecalculateLevelWindowForSelectedComponent);
    BaseProperty::Pointer displayedImageComponent = it->Value()->GetProperty("Image.Displayed Component");
    if (displayedImageComponent.IsNotNull())
    {
      unsigned long dispIdx = displayedImageComponent->AddObserver(itk::ModifiedEvent(), command4);
      m_ObserverToDisplayedComponentProperty[PropDataPair(dispIdx, it->Value())] = displayedImageComponent.GetPointer();
    }

    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command5 =
      itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command5->SetCallbackFunction(this, &LevelWindowManager::Update);
    BaseProperty::Pointer imgForLvlWin = it->Value()->GetProperty("imageForLevelWindow");
    if (imgForLvlWin.IsNull())
    {
      it->Value()->SetBoolProperty("imageForLevelWindow", false);
      imgForLvlWin = it->Value()->GetProperty("imageForLevelWindow");
    }
    unsigned long lvlWinIdx = imgForLvlWin->AddObserver(itk::ModifiedEvent(), command5);
    m_ObserverToLevelWindowImageProperty[PropDataPair(lvlWinIdx, it->Value())] = it->Value()->GetProperty("imageForLevelWindow");

    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command6 =
      itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command6->SetCallbackFunction(this, &LevelWindowManager::UpdateSelected);
    BaseProperty::Pointer selectedDataNode = it->Value()->GetProperty("selected");
    if (selectedDataNode.IsNull())
    {
      it->Value()->SetBoolProperty("selected", false);
      selectedDataNode = it->Value()->GetProperty("selected");
    }
    unsigned long selectedIdx = selectedDataNode->AddObserver(itk::ModifiedEvent(), command5);
    m_ObserverToSelectedProperty[PropDataPair(selectedIdx, it->Value())] = it->Value()->GetProperty("selected");
  }
}

bool mitk::LevelWindowManager::IgnoreNode(const DataNode* dataNode)
{
  LevelWindowProperty::Pointer levelWindowProperty =
    dynamic_cast<LevelWindowProperty*>(dataNode->GetProperty("levelwindow"));
  if (levelWindowProperty.IsNull())
  {
    return true;
  }

  int nonLvlWinMode1 = RenderingModeProperty::LOOKUPTABLE_COLOR;
  int nonLvlWinMode2 = RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR;

  RenderingModeProperty::Pointer mode =
    dynamic_cast<RenderingModeProperty*>(dataNode->GetProperty("Image Rendering.Mode"));

  if (mode.IsNotNull())
  {
    int currMode = mode->GetRenderingMode();
    if (currMode == nonLvlWinMode1 || currMode == nonLvlWinMode2)
    {
      return true;
    }
  }
  else
  {
    return true;
  }

  return false;
}
