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

#include "mitkNonBlockingAlgorithm.h"
#include "mitkCallbackFromGUIThread.h"
#include "mitkDataStorage.h"
#include <itkCommand.h>

namespace mitk {

NonBlockingAlgorithm::NonBlockingAlgorithm()
: m_ThreadID(-1),
  m_UpdateRequests(0),
  m_KillRequest(false)
{
  m_ParameterListMutex = itk::FastMutexLock::New();
  m_Parameters = PropertyList::New();
  m_MultiThreader = itk::MultiThreader::New();
}

NonBlockingAlgorithm::~NonBlockingAlgorithm()
{
}

void mitk::NonBlockingAlgorithm::SetDataStorage(DataStorage& storage)
{
  m_DataStorage = &storage;
}

DataStorage* mitk::NonBlockingAlgorithm::GetDataStorage()
{
  return m_DataStorage;
}


void NonBlockingAlgorithm::Initialize(const NonBlockingAlgorithm* itkNotUsed(other))
{ 
  // define one input, one output basedata object

  // some basedata input - image, surface, whatever
  BaseData::Pointer input;
  SetPointerParameter("Input", input );

  // some basedata output
  BaseData::Pointer output;
  SetPointerParameter("Output", output );
}

void NonBlockingAlgorithm::SetPointerParameter(const char* parameter, BaseData* value)
{
  m_ParameterListMutex->Lock();
  m_Parameters->SetProperty(parameter, SmartPointerProperty::New(value) );
  m_ParameterListMutex->Unlock();
}

void NonBlockingAlgorithm::DefineTriggerParameter(const char* parameter)
{
  BaseProperty* value = m_Parameters->GetProperty( parameter );
  if (   value
      && m_TriggerPropertyConnections.find(parameter) == m_TriggerPropertyConnections.end() )
  {
    itk::ReceptorMemberCommand<NonBlockingAlgorithm>::Pointer command = itk::ReceptorMemberCommand<NonBlockingAlgorithm>::New();
    command->SetCallbackFunction( this, &NonBlockingAlgorithm::TriggerParameterModified);

    m_TriggerPropertyConnections[ parameter ] = value->AddObserver( itk::ModifiedEvent(), command );
  }
}

void NonBlockingAlgorithm::UnDefineTriggerParameter(const char* parameter)
{
  MapTypeStringUInt::iterator iter = m_TriggerPropertyConnections.find( parameter );

  if ( iter != m_TriggerPropertyConnections.end() )
  {
    BaseProperty* value = m_Parameters->GetProperty( parameter );
    
    LOG_ERROR(!value) << "NonBlockingAlgorithm::UnDefineTriggerProperty() in bad state." << std::endl; ;

    value->RemoveObserver( m_TriggerPropertyConnections[parameter] );
    m_TriggerPropertyConnections.erase(iter);
  }
}


void NonBlockingAlgorithm::Reset() 
{ 
  Initialize(); 
} 


void NonBlockingAlgorithm::StartBlockingAlgorithm()
{
   StartAlgorithm();
   StopAlgorithm();
}

void NonBlockingAlgorithm::StartAlgorithm()
{
  if ( !ReadyToRun() ) return; // let algorithm check if all input/parameters are ok
  if (m_KillRequest) return; // someone wants us to die

   m_ParameterListMutex->Lock();
     m_ThreadParameters.m_Algorithm = this;
     ++m_UpdateRequests;
   m_ParameterListMutex->Unlock();
  if (m_ThreadID != -1) // thread already running. But something obviously wants us to recalculate the output
  {
    return; // thread already running
  }

   // spawn a thread that calls ThreadedUpdateFunction(), and ThreadedUpdateFinished() on us
   itk::ThreadFunctionType fpointer = &StaticNonBlockingAlgorithmThread;
   m_ThreadID = m_MultiThreader->SpawnThread( fpointer, &m_ThreadParameters);
}

void NonBlockingAlgorithm::StopAlgorithm()
{
  if (m_ThreadID == -1) return; // thread not running
 
  m_MultiThreader->TerminateThread( m_ThreadID ); // waits for the thread to terminate on its own
}


// a static function to call a member of NonBlockingAlgorithm from inside an ITK thread
ITK_THREAD_RETURN_TYPE NonBlockingAlgorithm::StaticNonBlockingAlgorithmThread(void* param)
{
  // itk::MultiThreader provides an itk::MultiThreader::ThreadInfoStruct as parameter
  itk::MultiThreader::ThreadInfoStruct* itkmttis = static_cast<itk::MultiThreader::ThreadInfoStruct*>(param);
  
  // we need the UserData part of that structure
  ThreadParameters* flsp = static_cast<ThreadParameters*>(itkmttis->UserData);

  NonBlockingAlgorithm::Pointer algorithm = flsp->m_Algorithm;
  // this UserData tells us, which BubbleTool's method to call
  if (!algorithm) 
  {
    return ITK_THREAD_RETURN_VALUE;
  }

  algorithm->m_ParameterListMutex->Lock();
  while ( algorithm->m_UpdateRequests > 0 )
  {
    algorithm->m_UpdateRequests = 0;
    algorithm->m_ParameterListMutex->Unlock();

    // actually call the methods that do the work
    if ( algorithm->ThreadedUpdateFunction() ) // returns a bool for success/failure
    {
      itk::ReceptorMemberCommand<NonBlockingAlgorithm>::Pointer command = itk::ReceptorMemberCommand<NonBlockingAlgorithm>::New();
      command->SetCallbackFunction(algorithm, &NonBlockingAlgorithm::ThreadedUpdateSuccessful);
      CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command);
      //algorithm->ThreadedUpdateSuccessful();
    }
    else
    {
      itk::ReceptorMemberCommand<NonBlockingAlgorithm>::Pointer command = itk::ReceptorMemberCommand<NonBlockingAlgorithm>::New();
      command->SetCallbackFunction(algorithm, &NonBlockingAlgorithm::ThreadedUpdateFailed);
      CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command);
      //algorithm->ThreadedUpdateFailed();
    }
    
    algorithm->m_ParameterListMutex->Lock();
  }
  algorithm->m_ParameterListMutex->Unlock();

  return ITK_THREAD_RETURN_VALUE;
}


void NonBlockingAlgorithm::TriggerParameterModified(const itk::EventObject&)
{
  StartAlgorithm();
}

bool NonBlockingAlgorithm::ReadyToRun()
{
  return true; // default is always ready
}
    
bool NonBlockingAlgorithm::ThreadedUpdateFunction()
{
  return true;
}

// called from gui thread
void NonBlockingAlgorithm::ThreadedUpdateSuccessful(const itk::EventObject&)
{
  ThreadedUpdateSuccessful();
  
  m_ParameterListMutex->Lock();
  m_ThreadID = -1; // tested before starting
  m_ParameterListMutex->Unlock();
  m_ThreadParameters.m_Algorithm = NULL;
}
    
void NonBlockingAlgorithm::ThreadedUpdateSuccessful()
{
  // notify observers that a result is ready
  InvokeEvent( ResultAvailable(this) );
}

// called from gui thread
void NonBlockingAlgorithm::ThreadedUpdateFailed(const itk::EventObject&)
{
  ThreadedUpdateFailed();
  
  m_ParameterListMutex->Lock();
  m_ThreadID = -1; // tested before starting
  m_ParameterListMutex->Unlock();
  m_ThreadParameters.m_Algorithm = NULL; //delete
}
 
void NonBlockingAlgorithm::ThreadedUpdateFailed()
{
  // notify observers that something went wrong
  InvokeEvent( ProcessingError(this) );
}

} // namespace

