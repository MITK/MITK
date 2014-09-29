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
#include <itkCommand.h>
#include "mitkDataStorage.h"
#include "mitkNodePredicateBase.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateNot.h"
#include "mitkProperties.h"
#include "mitkMessage.h"
#include "mitkRenderingModeProperty.h"
#include "mitkImage.h"

mitk::LevelWindowManager::LevelWindowManager()
: m_DataStorage(NULL)
, m_LevelWindowProperty(NULL)
, m_AutoTopMost(true)
, m_IsObserverTagSet(false)
, m_CurrentImage(NULL)
, m_IsPropertyModifiedTagSet(false)
{
}


mitk::LevelWindowManager::~LevelWindowManager()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
        MessageDelegate1<LevelWindowManager, const mitk::DataNode*>( this, &LevelWindowManager::DataStorageAddedNode ));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
        MessageDelegate1<LevelWindowManager, const mitk::DataNode*>( this, &LevelWindowManager::DataStorageRemovedNode ));
    m_DataStorage = NULL;
  }

  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  //clear both observer maps
  this->ClearPropObserverLists();
}


void mitk::LevelWindowManager::SetDataStorage( mitk::DataStorage* ds )
{
  if (ds == NULL) return;

  /* remove listeners of old DataStorage */
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
        MessageDelegate1<LevelWindowManager, const mitk::DataNode*>( this, &LevelWindowManager::DataStorageAddedNode ));
    m_DataStorage->RemoveNodeEvent.RemoveListener(
        MessageDelegate1<LevelWindowManager, const mitk::DataNode*>( this, &LevelWindowManager::DataStorageRemovedNode ));
  }

  /* register listener for new DataStorage */
  m_DataStorage = ds;  // register
  m_DataStorage->AddNodeEvent.AddListener(
      MessageDelegate1<LevelWindowManager, const mitk::DataNode*>( this, &LevelWindowManager::DataStorageAddedNode ));
  m_DataStorage->RemoveNodeEvent.AddListener(
      MessageDelegate1<LevelWindowManager, const mitk::DataNode*>( this, &LevelWindowManager::DataStorageRemovedNode ));

  this->DataStorageAddedNode(); // update us with new DataStorage
}


void mitk::LevelWindowManager::OnPropertyModified(const itk::EventObject& )
{
  Modified();
}


void mitk::LevelWindowManager::SetAutoTopMostImage(bool autoTopMost, const mitk::DataNode* removedNode)
{
  m_AutoTopMost = autoTopMost;
  if (m_AutoTopMost == false)
    return;

  if (m_IsPropertyModifiedTagSet && m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  /* search topmost image */
  if (m_DataStorage.IsNull())
  {
    itkExceptionMacro("DataStorage not set");
  }

  int maxLayer = itk::NumericTraits<int>::min();
  m_LevelWindowProperty = NULL;

  mitk::DataNode::Pointer topLevelNode;

  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    mitk::DataNode::Pointer node = it->Value();
    if (node.IsNull() || (removedNode != NULL && node == removedNode))
      continue;

    node->SetBoolProperty( "imageForLevelWindow", false );

    if (node->IsVisible(NULL) == false)
      continue;

    int layer = 0;
    node->GetIntProperty("layer", layer);
    if ( layer < maxLayer )
      continue;

    mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"));
    if (levelWindowProperty.IsNull())
      continue;

    int nonLvlWinMode1 = mitk::RenderingModeProperty::LOOKUPTABLE_COLOR;
    int nonLvlWinMode2 = mitk::RenderingModeProperty::COLORTRANSFERFUNCTION_COLOR;

    mitk::RenderingModeProperty::Pointer mode = dynamic_cast<mitk::RenderingModeProperty*>(node->GetProperty( "Image Rendering.Mode" ));

    if( mode.IsNotNull() )
    {
      int currMode = mode->GetRenderingMode();
      if ( currMode == nonLvlWinMode1 || currMode == nonLvlWinMode2 )
      {
        continue;
      }
    }
    else continue;

    m_LevelWindowProperty = levelWindowProperty;
    m_CurrentImage = dynamic_cast<mitk::Image*>(node->GetData());
    topLevelNode = node;

    maxLayer = layer;
  }

  if (topLevelNode.IsNotNull())
  {
    topLevelNode->SetBoolProperty( "imageForLevelWindow", true );
  }

  this->SetLevelWindowProperty( m_LevelWindowProperty );

  if ( m_LevelWindowProperty.IsNull() )
  {
    Modified();
  }
  // else SetLevelWindowProperty will call Modified();
}

// sets an specific LevelWindowProperty, all changes will affect the image belonging to this property.
void mitk::LevelWindowManager::SetLevelWindowProperty(LevelWindowProperty::Pointer levelWindowProperty)
{
  if (levelWindowProperty.IsNull())
    return;

  if (m_IsPropertyModifiedTagSet)  // remove listener for old property
  {
    m_LevelWindowProperty->RemoveObserver(m_PropertyModifiedTag);
    m_IsPropertyModifiedTagSet = false;
  }

  m_LevelWindowProperty = levelWindowProperty;

  itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();  // register listener for new property
  command->SetCallbackFunction(this, &LevelWindowManager::OnPropertyModified);
  m_PropertyModifiedTag = m_LevelWindowProperty->AddObserver( itk::ModifiedEvent(), command );
  m_IsPropertyModifiedTagSet = true;

  /* search image than belongs to the property */
  mitk::NodePredicateProperty::Pointer p = mitk::NodePredicateProperty::New("levelwindow", m_LevelWindowProperty);
  mitk::DataNode* n = m_DataStorage->GetNode(p);
  if (n == NULL)
  {
    mitkThrow() << "No Image in DataStorage that belongs to LevelWindow property" << m_LevelWindowProperty;
  }

  m_CurrentImage = dynamic_cast<mitk::Image*>(n->GetData());
  n->SetBoolProperty( "imageForLevelWindow", true );
  this->Modified();
}


// returns the current mitkLevelWindowProperty object from the image that is affected by changes
mitk::LevelWindowProperty::Pointer mitk::LevelWindowManager::GetLevelWindowProperty()
{
  return m_LevelWindowProperty;
}


// returns Level/Window values for the current image
const mitk::LevelWindow& mitk::LevelWindowManager::GetLevelWindow()
{
  if (m_LevelWindowProperty.IsNotNull())
  {
    return m_LevelWindowProperty->GetLevelWindow();
  }
  else
  {
    itkExceptionMacro("No LevelWindow available!");
  }
}

// sets new Level/Window values and informs all listeners about changes
void mitk::LevelWindowManager::SetLevelWindow(const mitk::LevelWindow& levelWindow)
{
  if (m_LevelWindowProperty.IsNotNull())
  {
    m_LevelWindowProperty->SetLevelWindow(levelWindow);
  }
  this->Modified();
}

void mitk::LevelWindowManager::DataStorageAddedNode( const mitk::DataNode* )
{

  //update observers with new data storage
  UpdateObservers();

  //Initialize LevelWindowsManager to new image
  SetAutoTopMostImage(true);

  //check if everything is still ok
  if ((m_PropObserverToNode.size() != m_PropObserverToNode2.size()) || (m_PropObserverToNode2.size() != this->GetRelevantNodes()->size()))
     {mitkThrow() << "Wrong number of observers in Level Window Manager!";}

}

void mitk::LevelWindowManager::DataStorageRemovedNode( const mitk::DataNode* removedNode )
{
  //first: check if deleted node is part of relevant nodes. If not, abort method because there is no need change anything.
  if ((this->GetRelevantNodes()->size() == 0)) return;
  bool removedNodeIsRelevant = false;
  /* Iterator code: is crashing, don't know why... so using for loop
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = this->GetRelevantNodes()->Begin();
       it != this->GetRelevantNodes()->End();
       ++it)
  {if (it->Value() == removedNode) {removedNodeIsRelevant=true;}}*/
  for (unsigned int i=0; i<this->GetRelevantNodes()->size(); i++)
    {
    if (this->GetRelevantNodes()->at(i) == removedNode) {removedNodeIsRelevant=true;}
    }
  if (!removedNodeIsRelevant) return;

  //remember node which will be removed
  m_NodeMarkedToDelete = removedNode;

  //update observers
  UpdateObservers();

  /* search image than belongs to the property */
  if (m_LevelWindowProperty.IsNull())
  {
    SetAutoTopMostImage(true, removedNode);
  }
  else
  {
    mitk::NodePredicateProperty::Pointer p2 = mitk::NodePredicateProperty::New("levelwindow", m_LevelWindowProperty);
    mitk::DataNode* n = m_DataStorage->GetNode(p2);
    if (n == NULL || m_AutoTopMost) // if node was deleted, change our behaviour to AutoTopMost, if AutoTopMost is true change level window to topmost node
    {
      SetAutoTopMostImage(true, removedNode);
    }
  }

  //reset variable
  m_NodeMarkedToDelete = NULL;

  //check if everything is still ok
  if ((m_PropObserverToNode.size() != m_PropObserverToNode2.size()) || (m_PropObserverToNode2.size() != (this->GetRelevantNodes()->size()-1)))
     {mitkThrow() << "Wrong number of observers in Level Window Manager!";}
}

void mitk::LevelWindowManager::UpdateObservers()
{
  this->ClearPropObserverLists(); //remove old observers
  CreatePropObserverLists(); //create new observer lists




}


int mitk::LevelWindowManager::GetNumberOfObservers()
{
  return m_PropObserverToNode.size();
}

mitk::DataStorage* mitk::LevelWindowManager::GetDataStorage()
{
  return m_DataStorage.GetPointer();
}


// true if changes on slider or line-edits will affect always the topmost layer image
bool mitk::LevelWindowManager::isAutoTopMost()
{
  return m_AutoTopMost;
}


void mitk::LevelWindowManager::Update(const itk::EventObject&)  // visible property of a image has changed
{
  if (m_AutoTopMost)
  {
    SetAutoTopMostImage(true);
    return;
  }
  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin();
       it != all->End();
       ++it)
  {
    mitk::DataNode::Pointer node = it->Value();
    if (node.IsNull())
      continue;

    node->SetBoolProperty( "imageForLevelWindow", false );

    if (node->IsVisible(NULL) == false)
      continue;

    mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>(node->GetProperty("levelwindow"));
    if (levelWindowProperty.IsNull())
      continue;

    m_LevelWindowProperty = levelWindowProperty;
    m_CurrentImage = dynamic_cast<mitk::Image*>(node->GetData());
    node->SetBoolProperty( "imageForLevelWindow", true );
    if (m_LevelWindowProperty.IsNull() && m_LevelWindowProperty.GetPointer() == levelWindowProperty) // we found our m_LevelWindowProperty
    {
      return;
    }
  }
  Modified();
}


mitk::DataStorage::SetOfObjects::ConstPointer mitk::LevelWindowManager::GetRelevantNodes()
{
  if (m_DataStorage.IsNull())
    return mitk::DataStorage::SetOfObjects::ConstPointer(mitk::DataStorage::SetOfObjects::New());  // return empty set

  mitk::BoolProperty::Pointer trueProp = mitk::BoolProperty::New(true);
  mitk::NodePredicateProperty::Pointer notBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(false));
  mitk::NodePredicateProperty::Pointer hasLevelWindow = mitk::NodePredicateProperty::New("levelwindow", NULL);

  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateDataType::Pointer isDImage = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isTImage = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQImage = mitk::NodePredicateDataType::New("QBallImage");
  mitk::NodePredicateOr::Pointer predicateTypes = mitk::NodePredicateOr::New();
  predicateTypes->AddPredicate(isImage);
  predicateTypes->AddPredicate(isDImage);
  predicateTypes->AddPredicate(isTImage);
  predicateTypes->AddPredicate(isQImage);

  mitk::NodePredicateAnd::Pointer predicate = mitk::NodePredicateAnd::New();
  predicate->AddPredicate(notBinary);
  predicate->AddPredicate(hasLevelWindow);
  predicate->AddPredicate(predicateTypes);

  mitk::DataStorage::SetOfObjects::ConstPointer relevantNodes = m_DataStorage->GetSubset( predicate );

  return relevantNodes;
}


mitk::Image* mitk::LevelWindowManager::GetCurrentImage()
{
  return m_CurrentImage;
}

void mitk::LevelWindowManager::ClearPropObserverLists()
{
for( ObserverToPropertyMap::iterator iter = m_PropObserverToNode.begin();
       iter != m_PropObserverToNode.end();
       ++iter )
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = 0;
  }
  m_PropObserverToNode.clear();

  for( ObserverToPropertyMap::iterator iter = m_PropObserverToNode2.begin();
       iter != m_PropObserverToNode2.end();
       ++iter )
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = 0;
  }
  m_PropObserverToNode2.clear();

  for( ObserverToPropertyMap::iterator iter = m_PropObserverToNode3.begin();
       iter != m_PropObserverToNode3.end();
       ++iter )
  {
    (*iter).second->RemoveObserver((*iter).first.first);
    (*iter).second = 0;
  }
  m_PropObserverToNode3.clear();
}

void mitk::LevelWindowManager::CreatePropObserverLists()
{
  if (m_DataStorage.IsNull())     //check if data storage is set
    {itkExceptionMacro("DataStorage not set");}

  /* add observers for all relevant nodes */
  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetRelevantNodes();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin();
       it != all->End();
       ++it)
  {
    if ((it->Value().IsNull()) || (it->Value() == m_NodeMarkedToDelete))
      {continue;}

    /* register listener for changes in visible property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command->SetCallbackFunction(this, &LevelWindowManager::Update);
    unsigned long idx = it->Value()->GetProperty("visible")->AddObserver( itk::ModifiedEvent(), command );
    m_PropObserverToNode[PropDataPair(idx, it->Value())] = it->Value()->GetProperty("visible");
  }

  /* add observers for all layer properties*/
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin();
       it != all->End();
       ++it)
  {
    if ((it->Value().IsNull()) || (it->Value() == m_NodeMarkedToDelete))
      {continue;}
    /* register listener for changes in layer property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command2 = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command2->SetCallbackFunction(this, &LevelWindowManager::Update);
    unsigned long idx = it->Value()->GetProperty("layer")->AddObserver( itk::ModifiedEvent(), command2 );
    m_PropObserverToNode2[PropDataPair(idx, it->Value())] = it->Value()->GetProperty("layer");
  }

  /* add observers for all Image rendering.mode properties*/
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin();
       it != all->End();
       ++it)
  {
    if ((it->Value().IsNull()) || (it->Value() == m_NodeMarkedToDelete))
      {continue;}
    /* register listener for changes in layer property */
    itk::ReceptorMemberCommand<LevelWindowManager>::Pointer command3 = itk::ReceptorMemberCommand<LevelWindowManager>::New();
    command3->SetCallbackFunction(this, &LevelWindowManager::Update);
    mitk::BaseProperty::Pointer imageRenderingMode = it->Value()->GetProperty("Image Rendering.Mode");
    if( imageRenderingMode.IsNotNull() )
    {
      unsigned long idx = imageRenderingMode->AddObserver( itk::ModifiedEvent(), command3 );
      m_PropObserverToNode3[PropDataPair(idx, it->Value())] = imageRenderingMode.GetPointer();
    }
  }

}
