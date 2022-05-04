/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkToolManager.h"
#include "mitkToolManagerProvider.h"
#include "mitkCoreObjectFactory.h"

#include <itkCommand.h>
#include <itkObjectFactoryBase.h>

#include <list>

#include "mitkInteractionEventObserver.h"
#include "mitkSegTool2D.h"
#include "mitkRenderingManager.h"
#include "mitkSliceNavigationController.h"

#include "usGetModuleContext.h"
#include "usModuleContext.h"

mitk::ToolManager::ToolManager(DataStorage *storage)
  : m_ActiveTool(nullptr), m_ActiveToolID(-1), m_RegisteredClients(0), m_DataStorage(storage)
{
  CoreObjectFactory::GetInstance(); // to make sure a CoreObjectFactory was instantiated (and in turn, possible tools
                                    // are registered) - bug 1029
  this->InitializeTools();
}

void mitk::ToolManager::EnsureTimeObservation()
{
  if (nullptr != mitk::RenderingManager::GetInstance() && nullptr != mitk::RenderingManager::GetInstance()->GetTimeNavigationController())
  {
    auto timeController = mitk::RenderingManager::GetInstance()->GetTimeNavigationController();

    m_LastTimePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

    auto currentTimeController = m_CurrentTimeNavigationController.Lock();

    if (timeController != currentTimeController)
    {
      if (currentTimeController.IsNotNull())
      {
        currentTimeController->RemoveObserver(m_TimePointObserverTag);
      }

      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction(this, &ToolManager::OnTimeChanged);
      command->SetCallbackFunction(this, &ToolManager::OnTimeChangedConst);
      m_CurrentTimeNavigationController = timeController;
      m_TimePointObserverTag = timeController->AddObserver(SliceNavigationController::GeometryTimeEvent(nullptr,0), command);
    }
  }
}

void mitk::ToolManager::StopTimeObservation()
{
  auto currentTimeController = m_CurrentTimeNavigationController.Lock();

  if (currentTimeController.IsNotNull())
  {
    currentTimeController->RemoveObserver(m_TimePointObserverTag);
    m_CurrentTimeNavigationController = nullptr;
    m_TimePointObserverTag = 0;
  }
}

mitk::ToolManager::~ToolManager()
{
  for (auto dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter)
    (*dataIter)->RemoveObserver(m_WorkingDataObserverTags[(*dataIter)]);

  if (this->GetDataStorage() != nullptr)
    this->GetDataStorage()->RemoveNodeEvent.RemoveListener(
      mitk::MessageDelegate1<ToolManager, const mitk::DataNode *>(this, &ToolManager::OnNodeRemoved));

  if (m_ActiveTool)
  {
    m_ActiveTool->Deactivated();
    m_ActiveToolRegistration.Unregister();

    m_ActiveTool = nullptr;
    m_ActiveToolID = -1; // no tool active

    ActiveToolChanged.Send();
  }
  for (auto observerTagMapIter = m_ReferenceDataObserverTags.begin();
       observerTagMapIter != m_ReferenceDataObserverTags.end();
       ++observerTagMapIter)
  {
    observerTagMapIter->first->RemoveObserver(observerTagMapIter->second);
  }
  this->StopTimeObservation();
}

void mitk::ToolManager::InitializeTools()
{
  // clear all previous tool pointers (tools may be still activated from another recently used plugin)
  if (m_ActiveTool)
  {
    m_ActiveTool->Deactivated();
    m_ActiveToolRegistration.Unregister();

    m_ActiveTool = nullptr;
    m_ActiveToolID = -1; // no tool active

    ActiveToolChanged.Send();
  }
  m_Tools.clear();
  // get a list of all known mitk::Tools
  std::list<itk::LightObject::Pointer> thingsThatClaimToBeATool = itk::ObjectFactoryBase::CreateAllInstance("mitkTool");

  // remember these tools
  for (auto iter = thingsThatClaimToBeATool.begin();
       iter != thingsThatClaimToBeATool.end();
       ++iter)
  {
    if (auto *tool = dynamic_cast<Tool *>(iter->GetPointer()))
    {
      tool->InitializeStateMachine();
      tool->SetToolManager(this); // important to call right after instantiation
      tool->ErrorMessage += MessageDelegate1<mitk::ToolManager, std::string>(this, &ToolManager::OnToolErrorMessage);
      tool->GeneralMessage +=
        MessageDelegate1<mitk::ToolManager, std::string>(this, &ToolManager::OnGeneralToolMessage);
      m_Tools.push_back(tool);
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

  for (auto iter = m_Tools.begin(); iter != m_Tools.end(); ++iter)
  {
    resultList.push_back(iter->GetPointer());
  }

  return resultList;
}

mitk::Tool *mitk::ToolManager::GetToolById(int id)
{
  try
  {
    return m_Tools.at(id);
  }
  catch (const std::exception &)
  {
    return nullptr;
  }
}

bool mitk::ToolManager::ActivateTool(int id)
{
  const auto workingDataNode = this->GetWorkingData(0);
  const mitk::BaseData* workingData = nullptr;
  if (nullptr != workingDataNode)
  {
    workingData = workingDataNode->GetData();
  }

  const auto referenceDataNode = this->GetReferenceData(0);
  const mitk::BaseData* referenceData = nullptr;
  if (nullptr != referenceDataNode)
  {
    referenceData = referenceDataNode->GetData();
  }

  if (id != -1 && !this->GetToolById(id)->CanHandle(referenceData, workingData))
    return false;

  if (this->GetDataStorage())
  {
    this->GetDataStorage()->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<ToolManager, const mitk::DataNode *>(this, &ToolManager::OnNodeRemoved));
  }

  if (GetToolById(id) == m_ActiveTool)
    return true; // no change needed

  static int nextTool = -1;
  nextTool = id;

  static bool inActivateTool = false;
  if (inActivateTool)
  {
    return true;
  }
  inActivateTool = true;

  while (nextTool != m_ActiveToolID)
  {
    // Deactivate all other active tools to ensure a globally single active tool
    for (const auto& toolManager : ToolManagerProvider::GetInstance()->GetToolManagers())
    {
      if (nullptr != toolManager.second->m_ActiveTool)
      {
        toolManager.second->m_ActiveTool->Deactivated();
        toolManager.second->m_ActiveToolRegistration.Unregister();

        // The active tool of *this* ToolManager is handled below this loop
        if (this != toolManager.second)
        {
          toolManager.second->m_ActiveTool = nullptr;
          toolManager.second->m_ActiveToolID = -1;
          toolManager.second->ActiveToolChanged.Send();
        }
      }
    }

    m_ActiveTool = GetToolById(nextTool);
    m_ActiveToolID = m_ActiveTool ? nextTool : -1; // current ID if tool is valid, otherwise -1

    ActiveToolChanged.Send();

    if (m_ActiveTool)
    {
      this->EnsureTimeObservation();
      if (m_RegisteredClients > 0)
      {
        m_ActiveTool->Activated();
        m_ActiveToolRegistration =
          us::GetModuleContext()->RegisterService<InteractionEventObserver>(m_ActiveTool, us::ServiceProperties());
      }
    }
  }

  inActivateTool = false;
  return (m_ActiveTool != nullptr);
}

void mitk::ToolManager::SetReferenceData(DataVectorType data)
{
  if (data != m_ReferenceData)
  {
    // remove observers from old nodes
    for (auto dataIter = m_ReferenceData.begin(); dataIter != m_ReferenceData.end(); ++dataIter)
    {
      auto searchIter = m_ReferenceDataObserverTags.find(*dataIter);
      if (searchIter != m_ReferenceDataObserverTags.end())
      {
        (*dataIter)->RemoveObserver(searchIter->second);
      }
    }

    m_ReferenceData = data;
    // TODO tell active tool?

    // attach new observers
    m_ReferenceDataObserverTags.clear();
    for (auto dataIter = m_ReferenceData.begin(); dataIter != m_ReferenceData.end(); ++dataIter)
    {
      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction(this, &ToolManager::OnOneOfTheReferenceDataDeleted);
      command->SetCallbackFunction(this, &ToolManager::OnOneOfTheReferenceDataDeletedConst);
      m_ReferenceDataObserverTags.insert(
        std::pair<DataNode *, unsigned long>((*dataIter), (*dataIter)->AddObserver(itk::DeleteEvent(), command)));
    }

    ReferenceDataChanged.Send();
  }
}

void mitk::ToolManager::OnOneOfTheReferenceDataDeletedConst(const itk::Object *caller, const itk::EventObject &e)
{
  OnOneOfTheReferenceDataDeleted(const_cast<itk::Object *>(caller), e);
}

void mitk::ToolManager::OnOneOfTheReferenceDataDeleted(itk::Object *caller, const itk::EventObject &itkNotUsed(e))
{
  DataVectorType v;

  for (auto dataIter = m_ReferenceData.begin(); dataIter != m_ReferenceData.end(); ++dataIter)
  {
    if ((void *)(*dataIter) != (void *)caller)
    {
      v.push_back(*dataIter);
    }
    else
    {
      m_ReferenceDataObserverTags.erase(*dataIter); // no tag to remove anymore
    }
  }
  this->SetReferenceData(v);
}

void mitk::ToolManager::SetReferenceData(DataNode *data)
{
  DataVectorType v;
  if (data)
  {
    v.push_back(data);
  }
  SetReferenceData(v);
}

void mitk::ToolManager::SetWorkingData(DataVectorType data)
{
  if (data != m_WorkingData)
  {
    // remove observers from old nodes
    for (auto dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter)
    {
      auto searchIter = m_WorkingDataObserverTags.find(*dataIter);
      if (searchIter != m_WorkingDataObserverTags.end())
      {
        (*dataIter)->RemoveObserver(searchIter->second);
      }
    }

    m_WorkingData = data;
    // TODO tell active tool?

    // Quick workaround for bug #16598
    if (m_WorkingData.empty())
      this->ActivateTool(-1);
    // workaround end

    // attach new observers
    m_WorkingDataObserverTags.clear();
    for (auto dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter)
    {
      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction(this, &ToolManager::OnOneOfTheWorkingDataDeleted);
      command->SetCallbackFunction(this, &ToolManager::OnOneOfTheWorkingDataDeletedConst);
      m_WorkingDataObserverTags.insert(
        std::pair<DataNode *, unsigned long>((*dataIter), (*dataIter)->AddObserver(itk::DeleteEvent(), command)));
    }

    WorkingDataChanged.Send();
  }
}

void mitk::ToolManager::OnOneOfTheWorkingDataDeletedConst(const itk::Object *caller, const itk::EventObject &e)
{
  OnOneOfTheWorkingDataDeleted(const_cast<itk::Object *>(caller), e);
}

void mitk::ToolManager::OnOneOfTheWorkingDataDeleted(itk::Object *caller, const itk::EventObject &itkNotUsed(e))
{
  DataVectorType v;

  for (auto dataIter = m_WorkingData.begin(); dataIter != m_WorkingData.end(); ++dataIter)
  {
    if ((void *)(*dataIter) != (void *)caller)
    {
      v.push_back(*dataIter);
    }
    else
    {
      m_WorkingDataObserverTags.erase(*dataIter); // no tag to remove anymore
    }
  }
  this->SetWorkingData(v);
}

void mitk::ToolManager::SetWorkingData(DataNode *data)
{
  DataVectorType v;

  if (data) // don't allow for nullptr nodes
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
    for (auto dataIter = m_RoiData.begin(); dataIter != m_RoiData.end(); ++dataIter)
    {
      auto searchIter = m_RoiDataObserverTags.find(*dataIter);
      if (searchIter != m_RoiDataObserverTags.end())
      {
        (*dataIter)->RemoveObserver(searchIter->second);
      }
    }

    m_RoiData = data;
    // TODO tell active tool?

    // attach new observers
    m_RoiDataObserverTags.clear();
    for (auto dataIter = m_RoiData.begin(); dataIter != m_RoiData.end(); ++dataIter)
    {
      itk::MemberCommand<ToolManager>::Pointer command = itk::MemberCommand<ToolManager>::New();
      command->SetCallbackFunction(this, &ToolManager::OnOneOfTheRoiDataDeleted);
      command->SetCallbackFunction(this, &ToolManager::OnOneOfTheRoiDataDeletedConst);
      m_RoiDataObserverTags.insert(
        std::pair<DataNode *, unsigned long>((*dataIter), (*dataIter)->AddObserver(itk::DeleteEvent(), command)));
    }
    RoiDataChanged.Send();
  }
}

void mitk::ToolManager::SetRoiData(DataNode *data)
{
  DataVectorType v;

  if (data)
  {
    v.push_back(data);
  }
  this->SetRoiData(v);
}

void mitk::ToolManager::OnOneOfTheRoiDataDeletedConst(const itk::Object *caller, const itk::EventObject &e)
{
  OnOneOfTheRoiDataDeleted(const_cast<itk::Object *>(caller), e);
}

void mitk::ToolManager::OnOneOfTheRoiDataDeleted(itk::Object *caller, const itk::EventObject &itkNotUsed(e))
{
  DataVectorType v;

  for (auto dataIter = m_RoiData.begin(); dataIter != m_RoiData.end(); ++dataIter)
  {
    if ((void *)(*dataIter) != (void *)caller)
    {
      v.push_back(*dataIter);
    }
    else
    {
      m_RoiDataObserverTags.erase(*dataIter); // no tag to remove anymore
    }
  }
  this->SetRoiData(v);
}

mitk::ToolManager::DataVectorType mitk::ToolManager::GetReferenceData()
{
  return m_ReferenceData;
}

mitk::DataNode *mitk::ToolManager::GetReferenceData(int idx)
{
  try
  {
    return m_ReferenceData.at(idx);
  }
  catch (const std::exception &)
  {
    return nullptr;
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

mitk::DataNode *mitk::ToolManager::GetRoiData(int idx)
{
  try
  {
    return m_RoiData.at(idx);
  }
  catch (const std::exception &)
  {
    return nullptr;
  }
}

mitk::DataStorage::Pointer mitk::ToolManager::GetDataStorage() const
{
  return m_DataStorage.Lock();
}

void mitk::ToolManager::SetDataStorage(DataStorage &storage)
{
  m_DataStorage = &storage;
}

mitk::DataNode *mitk::ToolManager::GetWorkingData(unsigned int idx)
{
  if (m_WorkingData.empty())
    return nullptr;

  if (m_WorkingData.size() > idx)
    return m_WorkingData[idx];

  return nullptr;
}

int mitk::ToolManager::GetActiveToolID()
{
  return m_ActiveToolID;
}

mitk::Tool *mitk::ToolManager::GetActiveTool()
{
  return m_ActiveTool;
}

void mitk::ToolManager::RegisterClient()
{
  if (m_RegisteredClients < 1)
  {
    if (m_ActiveTool)
    {
      m_ActiveTool->Activated();
      m_ActiveToolRegistration =
        us::GetModuleContext()->RegisterService<InteractionEventObserver>(m_ActiveTool, us::ServiceProperties());
    }
  }
  ++m_RegisteredClients;
}

void mitk::ToolManager::UnregisterClient()
{
  if (m_RegisteredClients < 1)
    return;

  --m_RegisteredClients;
  if (m_RegisteredClients < 1)
  {
    if (m_ActiveTool)
    {
      m_ActiveTool->Deactivated();
      m_ActiveToolRegistration.Unregister();
    }
  }
}

int mitk::ToolManager::GetToolID(const Tool *tool)
{
  int id(0);
  for (auto iter = m_Tools.begin(); iter != m_Tools.end(); ++iter, ++id)
  {
    if (tool == iter->GetPointer())
    {
      return id;
    }
  }
  return -1;
}

void mitk::ToolManager::OnNodeRemoved(const mitk::DataNode *node)
{
  // check all storage vectors
  OnOneOfTheReferenceDataDeleted(const_cast<mitk::DataNode *>(node), itk::DeleteEvent());
  OnOneOfTheRoiDataDeleted(const_cast<mitk::DataNode *>(node), itk::DeleteEvent());
  OnOneOfTheWorkingDataDeleted(const_cast<mitk::DataNode *>(node), itk::DeleteEvent());
}

void mitk::ToolManager::OnTimeChanged(itk::Object* caller, const itk::EventObject& e)
{
  this->OnTimeChangedConst(caller, e);
}

void mitk::ToolManager::OnTimeChangedConst(const itk::Object* caller, const itk::EventObject& /*e*/)
{
  auto currentController = m_CurrentTimeNavigationController.Lock();
  if (caller == currentController)
  {
    const auto currentTimePoint = currentController->GetSelectedTimePoint();
    if (currentTimePoint != m_LastTimePoint)
    {
      m_LastTimePoint = currentTimePoint;
      SelectedTimePointChanged.Send();
    }
  }
}

mitk::TimePointType mitk::ToolManager::GetCurrentTimePoint() const
{
  return m_LastTimePoint;
}
