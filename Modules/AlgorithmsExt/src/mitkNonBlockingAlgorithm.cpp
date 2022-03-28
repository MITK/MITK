/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNonBlockingAlgorithm.h"
#include "mitkCallbackFromGUIThread.h"
#include "mitkDataStorage.h"
#include <itkCommand.h>

namespace mitk
{
  NonBlockingAlgorithm::NonBlockingAlgorithm() : m_UpdateRequests(0), m_KillRequest(false)
  {
    m_Parameters = PropertyList::New();
  }

  NonBlockingAlgorithm::~NonBlockingAlgorithm()
  {
    if (m_Thread.joinable())
      m_Thread.detach();
  }

  void mitk::NonBlockingAlgorithm::SetDataStorage(DataStorage &storage) { m_DataStorage = &storage; }
  DataStorage *mitk::NonBlockingAlgorithm::GetDataStorage() { return m_DataStorage.Lock(); }
  void NonBlockingAlgorithm::Initialize(const NonBlockingAlgorithm *itkNotUsed(other))
  {
    // define one input, one output basedata object

    // some basedata input - image, surface, whatever
    BaseData::Pointer input;
    SetPointerParameter("Input", input);

    // some basedata output
    BaseData::Pointer output;
    SetPointerParameter("Output", output);
  }

  void NonBlockingAlgorithm::SetPointerParameter(const char *parameter, BaseData *value)
  {
    m_ParameterListMutex.lock();
    m_Parameters->SetProperty(parameter, SmartPointerProperty::New(value));
    m_ParameterListMutex.unlock();
  }

  void NonBlockingAlgorithm::DefineTriggerParameter(const char *parameter)
  {
    BaseProperty *value = m_Parameters->GetProperty(parameter);
    if (value && m_TriggerPropertyConnections.find(parameter) == m_TriggerPropertyConnections.end())
    {
      itk::ReceptorMemberCommand<NonBlockingAlgorithm>::Pointer command =
        itk::ReceptorMemberCommand<NonBlockingAlgorithm>::New();
      command->SetCallbackFunction(this, &NonBlockingAlgorithm::TriggerParameterModified);

      m_TriggerPropertyConnections[parameter] = value->AddObserver(itk::ModifiedEvent(), command);
    }
  }

  void NonBlockingAlgorithm::UnDefineTriggerParameter(const char *parameter)
  {
    auto iter = m_TriggerPropertyConnections.find(parameter);

    if (iter != m_TriggerPropertyConnections.end())
    {
      BaseProperty *value = m_Parameters->GetProperty(parameter);

      MITK_ERROR(!value) << "NonBlockingAlgorithm::UnDefineTriggerProperty() in bad state." << std::endl;
      ;

      value->RemoveObserver(m_TriggerPropertyConnections[parameter]);
      m_TriggerPropertyConnections.erase(iter);
    }
  }

  void NonBlockingAlgorithm::Reset() { Initialize(); }
  void NonBlockingAlgorithm::StartBlockingAlgorithm()
  {
    StartAlgorithm();
    StopAlgorithm();
  }

  void NonBlockingAlgorithm::StartAlgorithm()
  {
    if (!ReadyToRun())
      return; // let algorithm check if all input/parameters are ok
    if (m_KillRequest)
      return; // someone wants us to die

    m_ParameterListMutex.lock();
    ++m_UpdateRequests;
    m_ParameterListMutex.unlock();
    if (m_Thread.joinable()) // thread already running. But something obviously wants us to recalculate the output
    {
      return; // thread already running
    }

    // spawn a thread that calls ThreadedUpdateFunction(), and ThreadedUpdateFinished() on us
    this->Register();
    m_Thread = std::thread(StaticNonBlockingAlgorithmThread, this);
  }

  void NonBlockingAlgorithm::StopAlgorithm()
  {
    if (m_Thread.joinable())
      m_Thread.join(); // waits for the thread to terminate on its own
  }

  // a static function to call a member of NonBlockingAlgorithm from inside a thread
  void NonBlockingAlgorithm::StaticNonBlockingAlgorithmThread(NonBlockingAlgorithm* algorithm)
  {
    algorithm->m_ParameterListMutex.lock();
    while (algorithm->m_UpdateRequests > 0)
    {
      algorithm->m_UpdateRequests = 0;
      algorithm->m_ParameterListMutex.unlock();

      // actually call the methods that do the work
      if (algorithm->ThreadedUpdateFunction()) // returns a bool for success/failure
      {
        auto command = itk::ReceptorMemberCommand<NonBlockingAlgorithm>::New();
        command->SetCallbackFunction(algorithm, &NonBlockingAlgorithm::ThreadedUpdateSuccessful);
        CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command);

      }
      else
      {
        auto command = itk::ReceptorMemberCommand<NonBlockingAlgorithm>::New();
        command->SetCallbackFunction(algorithm, &NonBlockingAlgorithm::ThreadedUpdateFailed);
        CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command);
      }

      algorithm->m_ParameterListMutex.lock();
    }
    algorithm->m_ParameterListMutex.unlock();
  }

  void NonBlockingAlgorithm::TriggerParameterModified(const itk::EventObject &) { StartAlgorithm(); }
  bool NonBlockingAlgorithm::ReadyToRun()
  {
    return true; // default is always ready
  }

  bool NonBlockingAlgorithm::ThreadedUpdateFunction() { return true; }
  // called from gui thread
  void NonBlockingAlgorithm::ThreadedUpdateSuccessful(const itk::EventObject &)
  {
    ThreadedUpdateSuccessful();
  }

  void NonBlockingAlgorithm::ThreadedUpdateSuccessful()
  {
    // notify observers that a result is ready
    InvokeEvent(ResultAvailable(this));
    this->UnRegister();
  }

  // called from gui thread
  void NonBlockingAlgorithm::ThreadedUpdateFailed(const itk::EventObject &)
  {
    ThreadedUpdateFailed();
  }

  void NonBlockingAlgorithm::ThreadedUpdateFailed()
  {
    // notify observers that something went wrong
    InvokeEvent(ProcessingError(this));
    this->UnRegister();
  }

} // namespace
