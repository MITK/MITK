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

#include "mitkToolManager.h"
#include "mitkGlobalInteraction.h"
#include "mitkCoreObjectFactory.h"

#include <itkObjectFactoryBase.h>
#include <itkCommand.h>

#include <list>

#include "mitkInteractionEventObserver.h"
#include "mitkDisplayInteractor.h"
#include "mitkSegTool2D.h"



mitk::ToolManager::ToolManager(DataStorage* storage)
:m_ActiveTool(NULL),
 m_ActiveToolID(-1),
 m_RegisteredClients(0),
 m_DataStorage(storage)
{
  CoreObjectFactory::GetInstance(); // to make sure a CoreObjectFactory was instantiated (and in turn, possible tools are registered) - bug 1029
  this->InitializeTools();
  //ActivateTool(0); // first one is default
}

mitk::ToolManager::~ToolManager()
{
  for (DataVectorType::iterator dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter)
    (*dataIter)->RemoveObserver(m_WorkingDataObserverTags[(*dataIter)]);

  if(this->GetDataStorage() != NULL)
    this->GetDataStorage()->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<ToolManager, const mitk::DataNode*>
        ( this, &ToolManager::OnNodeRemoved ));

  if (m_ActiveTool)
  {
    m_ActiveTool->Deactivated();
    GlobalInteraction::GetInstance()->RemoveListener( m_ActiveTool );

    m_ActiveTool = NULL;
    m_ActiveToolID = -1; // no tool active

    ActiveToolChanged.Send();

  }
  for ( NodeTagMapType::iterator observerTagMapIter = m_ReferenceDataObserverTags.begin(); observerTagMapIter != m_ReferenceDataObserverTags.end(); ++observerTagMapIter )
  {
    observerTagMapIter->first->RemoveObserver( observerTagMapIter->second );
  }
}


void mitk::ToolManager::InitializeTools()
{
  if(mitk::GlobalInteraction::GetInstance()->IsInitialized())
  {
    m_Tools.resize(0);
    // get a list of all known mitk::Tools
    std::list<itk::LightObject::Pointer> thingsThatClaimToBeATool = itk::ObjectFactoryBase::CreateAllInstance("mitkTool");

    // remember these tools
    for ( std::list<itk::LightObject::Pointer>::iterator iter = thingsThatClaimToBeATool.begin();
      iter != thingsThatClaimToBeATool.end();
      ++iter )
    {
      if ( Tool* tool = dynamic_cast<Tool*>( iter->GetPointer() ) )
      {
        tool->SetToolManager(this); // important to call right after instantiation
        tool->ErrorMessage += MessageDelegate1<mitk::ToolManager, std::string>( this, &ToolManager::OnToolErrorMessage );
        tool->GeneralMessage += MessageDelegate1<mitk::ToolManager, std::string>( this, &ToolManager::OnGeneralToolMessage );
        m_Tools.push_back( tool );
      }
    }
  }
}


void mitk::ToolManager::OnToolErrorMessage(std::string s)
{
  this->ToolErrorMessage(s);
}

void mitk::ToolManager::OnGeneralToolMessage(std::string s)
{
  this->GeneralToolMessage(s);
}


const mitk::ToolManager::ToolVectorTypeConst mitk::ToolManager::GetTools()
{
  ToolVectorTypeConst resultList;

  for ( ToolVectorType::iterator iter = m_Tools.begin();
    iter != m_Tools.end();
    ++iter )
  {
    resultList.push_back( iter->GetPointer() );
  }

  return resultList;
}

mitk::Tool* mitk::ToolManager::GetToolById(int id)
{
  try
  {
    return m_Tools.at(id);
  }
  catch(std::exception&)
  {
    return NULL;
  }
}

bool mitk::ToolManager::ActivateTool(int id)
{

  if(this->GetDataStorage())
    {
      this->GetDataStorage()->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<ToolManager, const mitk::DataNode*>
        ( this, &ToolManager::OnNodeRemoved ) );
    }

  //MITK_INFO << "ToolManager::ActivateTool("<<id<<")"<<std::endl;
  //if( GetToolById(id) == NULL ) return false; // NO, invalid IDs are actually used here. Parameter -1 or anything that does not exists will deactivate all tools!

  //If a tool is deactivated set the event notification policy of the global interaction to multiple again
  if (id == -1)
  {
    GlobalInteraction::GetInstance()->SetEventNotificationPolicy(GlobalInteraction::INFORM_MULTIPLE);
  }

  if ( GetToolById( id ) == m_ActiveTool ) return true; // no change needed

  static int nextTool = -1;
  nextTool = id;
  //MITK_INFO << "ToolManager::ActivateTool("<<id<<"): nextTool = "<<nextTool<<std::endl;

  static bool inActivateTool = false;
  if (inActivateTool)
  {
    //MITK_INFO << "ToolManager::ActivateTool("<<id<<"): already inside ActivateTool somehow, returning now "<<std::endl;
    return true;
  }
  inActivateTool = true;

  while ( nextTool != m_ActiveToolID )
  {
    //MITK_INFO <<"ToolManager::ActivateTool: nextTool = " << nextTool << " (active tool = " << m_ActiveToolID<<")"<<std::endl;
    if (m_ActiveTool)
    {
      m_ActiveTool->Deactivated();
      GlobalInteraction::GetInstance()->RemoveListener( m_ActiveTool );
    }

    m_ActiveTool = GetToolById( nextTool );
    m_ActiveToolID = m_ActiveTool ? nextTool : -1; // current ID if tool is valid, otherwise -1

    ActiveToolChanged.Send();

    if (m_ActiveTool)
    {
      if (m_RegisteredClients > 0)
      {
        m_ActiveTool->Activated();
        GlobalInteraction::GetInstance()->AddListener( m_ActiveTool );
        //If a tool is activated set event notification policy to one
        if (dynamic_cast<mitk::SegTool2D*>(m_ActiveTool))
          GlobalInteraction::GetInstance()->SetEventNotificationPolicy(GlobalInteraction::INFORM_ONE);
      }
    }
  }

  inActivateTool = false;
  return (m_ActiveTool != NULL);
}

void mitk::ToolManager::SetReferenceData(DataVectorType data)
{
  if (data != m_ReferenceData)
  {
    // remove observers from old nodes
    for ( DataVectorType::iterator dataIter = m_ReferenceData.begin(); dataIter != m_ReferenceData.end(); ++dataIter )
    {
      NodeTagMapType::iterator searchIter = m_ReferenceDataObserverTags.find( *dataIter );
      if ( searchIter != m_ReferenceDataObserverTags.end() )
      {
        //MITK_INFO << "Stopping observation of " << (void*)(*dataIter) << std::endl;
        (*dataIter)->RemoveObserver( searchIter->second );
      }
    }

    m_ReferenceData = data;
    // TODO tell active tool?

    // attach new observers
    m_ReferenceDataObserverTags.clear();
    for ( DataVectorType::iterator dataIter = m_ReferenceData.begin(); dataIter != m_ReferenceData.end(); ++dataIter )
    {


      //MITK_INFO << "Observing " << (void*)(*dataIter) << std::endl;
      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction( this, &ToolManager::OnOneOfTheReferenceDataDeleted );
      command->SetCallbackFunction( this, &ToolManager::OnOneOfTheReferenceDataDeletedConst );
      m_ReferenceDataObserverTags.insert( std::pair<DataNode*, unsigned long>( (*dataIter), (*dataIter)->AddObserver( itk::DeleteEvent(), command ) ) );
    }

    ReferenceDataChanged.Send();
  }
}

void mitk::ToolManager::OnOneOfTheReferenceDataDeletedConst(const itk::Object* caller, const itk::EventObject& e)
{
  OnOneOfTheReferenceDataDeleted( const_cast<itk::Object*>(caller), e );
}

void mitk::ToolManager::OnOneOfTheReferenceDataDeleted(itk::Object* caller, const itk::EventObject& itkNotUsed(e))
{
  //MITK_INFO << "Deleted: " << (void*)caller << " Removing from reference data list." << std::endl;
  DataVectorType v;

  for (DataVectorType::iterator dataIter = m_ReferenceData.begin(); dataIter != m_ReferenceData.end(); ++dataIter )
  {
    //MITK_INFO << " In list: " << (void*)(*dataIter);
    if ( (void*)(*dataIter) != (void*)caller )
    {
      v.push_back( *dataIter );
      //MITK_INFO << " kept" << std::endl;
    }
    else
    {
      //MITK_INFO << " removed" << std::endl;
      m_ReferenceDataObserverTags.erase( *dataIter ); // no tag to remove anymore
    }
  }
  this->SetReferenceData( v );
}

void mitk::ToolManager::SetReferenceData(DataNode* data)
{
  //MITK_INFO << "ToolManager::SetReferenceData(" << (void*)data << ")" << std::endl;
  DataVectorType v;
  if (data)
  {
    v.push_back(data);
  }
  SetReferenceData(v);
}

void mitk::ToolManager::SetWorkingData(DataVectorType data)
{
  if ( data != m_WorkingData )
  {
    // remove observers from old nodes
    for ( DataVectorType::iterator dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter )
    {
      NodeTagMapType::iterator searchIter = m_WorkingDataObserverTags.find( *dataIter );
      if ( searchIter != m_WorkingDataObserverTags.end() )
      {
        //MITK_INFO << "Stopping observation of " << (void*)(*dataIter) << std::endl;
        (*dataIter)->RemoveObserver( searchIter->second );
      }
    }

    m_WorkingData = data;
    // TODO tell active tool?

    // attach new observers
    m_WorkingDataObserverTags.clear();
    for ( DataVectorType::iterator dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter )
    {
      //MITK_INFO << "Observing " << (void*)(*dataIter) << std::endl;
      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction( this, &ToolManager::OnOneOfTheWorkingDataDeleted );
      command->SetCallbackFunction( this, &ToolManager::OnOneOfTheWorkingDataDeletedConst );
      m_WorkingDataObserverTags.insert( std::pair<DataNode*, unsigned long>( (*dataIter), (*dataIter)->AddObserver( itk::DeleteEvent(), command ) ) );
    }

    WorkingDataChanged.Send();
  }
}

void mitk::ToolManager::OnOneOfTheWorkingDataDeletedConst(const itk::Object* caller, const itk::EventObject& e)
{
  OnOneOfTheWorkingDataDeleted( const_cast<itk::Object*>(caller), e );
}

void mitk::ToolManager::OnOneOfTheWorkingDataDeleted(itk::Object* caller, const itk::EventObject& itkNotUsed(e))
{
  //MITK_INFO << "Deleted: " << (void*)caller << " Removing from reference data list." << std::endl;
  DataVectorType v;

  for (DataVectorType::iterator dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter )
  {
    //MITK_INFO << " In list: " << (void*)(*dataIter);
    if ( (void*)(*dataIter) != (void*)caller )
    {
      v.push_back( *dataIter );
      //MITK_INFO << " kept" << std::endl;
    }
    else
    {
      //MITK_INFO << " removed" << std::endl;
      m_WorkingDataObserverTags.erase( *dataIter ); // no tag to remove anymore
    }
  }
  this->SetWorkingData( v );
}


void mitk::ToolManager::SetWorkingData(DataNode* data)
{
  DataVectorType v;

  if (data) // don't allow for NULL nodes
  {
    v.push_back(data);
  }

  SetWorkingData(v);
}

void mitk::ToolManager::SetRoiData(DataVectorType data)
{
  if (data != m_RoiData)
  {
    // remove observers from old nodes
    for ( DataVectorType::iterator dataIter = m_RoiData.begin(); dataIter != m_RoiData.end(); ++dataIter )
    {
      NodeTagMapType::iterator searchIter = m_RoiDataObserverTags.find( *dataIter );
      if ( searchIter != m_RoiDataObserverTags.end() )
      {
        //MITK_INFO << "Stopping observation of " << (void*)(*dataIter) << std::endl;
        (*dataIter)->RemoveObserver( searchIter->second );
      }
    }

    m_RoiData = data;
    // TODO tell active tool?

    // attach new observers
    m_RoiDataObserverTags.clear();
    for ( DataVectorType::iterator dataIter = m_RoiData.begin(); dataIter != m_RoiData.end(); ++dataIter )
    {
      //MITK_INFO << "Observing " << (void*)(*dataIter) << std::endl;
      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction( this, &ToolManager::OnOneOfTheRoiDataDeleted );
      command->SetCallbackFunction( this, &ToolManager::OnOneOfTheRoiDataDeletedConst );
      m_RoiDataObserverTags.insert( std::pair<DataNode*, unsigned long>( (*dataIter), (*dataIter)->AddObserver( itk::DeleteEvent(), command ) ) );
    }
    RoiDataChanged.Send();
  }
}

void mitk::ToolManager::SetRoiData(DataNode* data)
{
  DataVectorType v;

  if(data)
  {
    v.push_back(data);
  }
  this->SetRoiData(v);
}

void mitk::ToolManager::OnOneOfTheRoiDataDeletedConst(const itk::Object* caller, const itk::EventObject& e)
{
  OnOneOfTheRoiDataDeleted( const_cast<itk::Object*>(caller), e );
}

void mitk::ToolManager::OnOneOfTheRoiDataDeleted(itk::Object* caller, const itk::EventObject& itkNotUsed(e))
{
  //MITK_INFO << "Deleted: " << (void*)caller << " Removing from roi data list." << std::endl;
  DataVectorType v;

  for (DataVectorType::iterator dataIter = m_RoiData.begin(); dataIter != m_RoiData.end(); ++dataIter )
  {
    //MITK_INFO << " In list: " << (void*)(*dataIter);
    if ( (void*)(*dataIter) != (void*)caller )
    {
      v.push_back( *dataIter );
      //MITK_INFO << " kept" << std::endl;
    }
    else
    {
      //MITK_INFO << " removed" << std::endl;
      m_RoiDataObserverTags.erase( *dataIter ); // no tag to remove anymore
    }
  }
  this->SetRoiData( v );
}

mitk::ToolManager::DataVectorType mitk::ToolManager::GetReferenceData()
{
  return m_ReferenceData;
}

mitk::DataNode* mitk::ToolManager::GetReferenceData(int idx)
{
  try
  {
    return m_ReferenceData.at(idx);
  }
  catch(std::exception&)
  {
    return NULL;
  }
}

mitk::ToolManager::DataVectorType mitk::ToolManager::GetWorkingData()
{
  return m_WorkingData;
}

mitk::ToolManager::DataVectorType mitk::ToolManager::GetRoiData()
{
  return m_RoiData;
}

mitk::DataNode* mitk::ToolManager::GetRoiData(int idx)
{
  try
  {
    return m_RoiData.at(idx);
  }
  catch(std::exception&)
  {
    return NULL;
  }
}

mitk::DataStorage* mitk::ToolManager::GetDataStorage()
{
  if ( m_DataStorage.IsNotNull() )
  {
    return m_DataStorage;
  }
  else
  {
    return NULL;
  }
}

void mitk::ToolManager::SetDataStorage(DataStorage& storage)
{
  m_DataStorage = &storage;
}



mitk::DataNode* mitk::ToolManager::GetWorkingData(int idx)
{
  try
  {
    return m_WorkingData.at(idx);
  }
  catch(std::exception&)
  {
    return NULL;
  }
}

int mitk::ToolManager::GetActiveToolID()
{
  return m_ActiveToolID;
}

mitk::Tool* mitk::ToolManager::GetActiveTool()
{
  return m_ActiveTool;
}

void mitk::ToolManager::RegisterClient()
{
  if ( m_RegisteredClients < 1 )
  {
    if ( m_ActiveTool )
    {
      m_ActiveTool->Activated();
      GlobalInteraction::GetInstance()->AddListener( m_ActiveTool );
    }
  }
  ++m_RegisteredClients;
}

void mitk::ToolManager::UnregisterClient()
{
  if ( m_RegisteredClients < 1) return;

  --m_RegisteredClients;
  if ( m_RegisteredClients < 1 )
  {
    if ( m_ActiveTool )
    {
      m_ActiveTool->Deactivated();
      GlobalInteraction::GetInstance()->RemoveListener( m_ActiveTool );
    }
  }
}

int mitk::ToolManager::GetToolID( const Tool* tool )
{
  int id(0);
  for ( ToolVectorType::iterator iter = m_Tools.begin();
    iter != m_Tools.end();
    ++iter, ++id )
  {
    if ( tool == iter->GetPointer() )
    {
      return id;
    }
  }
  return -1;
}


void mitk::ToolManager::OnNodeRemoved(const mitk::DataNode* node)
{
  //check if the data of the node is typeof Image
  /*if(dynamic_cast<mitk::Image*>(node->GetData()))
  {*/
    //check all storage vectors
    OnOneOfTheReferenceDataDeleted(const_cast<mitk::DataNode*>(node), itk::DeleteEvent());
    OnOneOfTheRoiDataDeleted(const_cast<mitk::DataNode*>(node),itk::DeleteEvent());
    OnOneOfTheWorkingDataDeleted(const_cast<mitk::DataNode*>(node),itk::DeleteEvent());
  //}
}

