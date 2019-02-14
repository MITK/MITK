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

#include "mitkIGTLMessageSource.h"
#include "mitkUIDGenerator.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

const std::string mitk::IGTLMessageSource::US_INTERFACE_NAME =
    "org.mitk.services.IGTLMessageSource";
const std::string mitk::IGTLMessageSource::US_PROPKEY_DEVICENAME =
    US_INTERFACE_NAME + ".devicename";
const std::string mitk::IGTLMessageSource::US_PROPKEY_DEVICETYPE =
    US_INTERFACE_NAME + ".devicetype";
const std::string mitk::IGTLMessageSource::US_PROPKEY_ID =
    US_INTERFACE_NAME + ".id";
const std::string mitk::IGTLMessageSource::US_PROPKEY_ISACTIVE =
    US_INTERFACE_NAME + ".isActive";

mitk::IGTLMessageSource::IGTLMessageSource()
  : itk::ProcessObject(), m_Name("IGTLMessageSource (no defined type)"),
    m_Type("NONE"), m_StreamingFPS(0)
{
  m_StreamingFPSMutex = itk::FastMutexLock::New();
}

mitk::IGTLMessageSource::~IGTLMessageSource()
{
  //this->UnRegisterMicroservice();
}

mitk::IGTLMessage* mitk::IGTLMessageSource::GetOutput()
{
  if (this->GetNumberOfIndexedOutputs() < 1)
  {
    MITK_WARN << "IGTLMessageSource contained no outputs. Returning nullptr.";
    return nullptr;
  }

  return static_cast<IGTLMessage*>(this->ProcessObject::GetPrimaryOutput());
}

mitk::IGTLMessage* mitk::IGTLMessageSource::GetOutput(
    DataObjectPointerArraySizeType idx)
{
  IGTLMessage* out =
      dynamic_cast<IGTLMessage*>( this->ProcessObject::GetOutput(idx) );
  if ( out == nullptr && this->ProcessObject::GetOutput(idx) != nullptr )
  {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type "
                     <<  typeid( IGTLMessage ).name () );
  }
  return out;
}

mitk::IGTLMessage* mitk::IGTLMessageSource::GetOutput(
    const std::string& messageName)
{
  DataObjectPointerArray outputs = this->GetOutputs();
  for (DataObjectPointerArray::iterator it = outputs.begin();
       it != outputs.end();
       ++it)
  {
    if (messageName ==
        (static_cast<IGTLMessage*>(it->GetPointer()))->GetName())
    {
      return static_cast<IGTLMessage*>(it->GetPointer());
    }
  }
  return nullptr;
}

itk::ProcessObject::DataObjectPointerArraySizeType
mitk::IGTLMessageSource::GetOutputIndex( std::string messageName )
{
  DataObjectPointerArray outputs = this->GetOutputs();
  for (DataObjectPointerArray::size_type i = 0; i < outputs.size(); ++i)
  {
    if (messageName ==
        (static_cast<IGTLMessage*>(outputs.at(i).GetPointer()))->GetName())
    {
      return i;
    }
  }
  throw std::invalid_argument("output name does not exist");
}

void mitk::IGTLMessageSource::RegisterAsMicroservice()
{
  // Get Context
  us::ModuleContext* context = us::GetModuleContext();

  // Define ServiceProps
  us::ServiceProperties props;
  mitk::UIDGenerator uidGen =
      mitk::UIDGenerator ("org.mitk.services.IGTLMessageSource.id_", 16);
  props[ US_PROPKEY_ID ] = uidGen.GetUID();
  props[ US_PROPKEY_DEVICENAME ] = m_Name;
  props[ US_PROPKEY_DEVICETYPE ] = m_Type;
  m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::IGTLMessageSource::UnRegisterMicroservice()
{
  if (m_ServiceRegistration != nullptr)
  {
    m_ServiceRegistration.Unregister();
  }
  m_ServiceRegistration = 0;
}

std::string mitk::IGTLMessageSource::GetMicroserviceID()
{
  us::Any referenceProperty =
      this->m_ServiceRegistration.GetReference().GetProperty(US_PROPKEY_ID);
  return referenceProperty.ToString();
}

void mitk::IGTLMessageSource::GraftOutput(itk::DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::IGTLMessageSource::GraftNthOutput(unsigned int idx,
                                             itk::DataObject *graft)
{
  if ( idx >= this->GetNumberOfIndexedOutputs() )
  {
    itkExceptionMacro(<<"Requested to graft output " << idx << " but this filter"
                "only has " << this->GetNumberOfIndexedOutputs() << " Outputs.");
  }

  if ( !graft )
  {
    itkExceptionMacro(<<"Requested to graft output with a nullptr pointer object" );
  }

  itk::DataObject* output = this->GetOutput(idx);
  if ( !output )
  {
    itkExceptionMacro(<<"Requested to graft output that is a nullptr pointer" );
  }
  // Call Graft on IGTLMessage to copy member data
  output->Graft( graft );
}

itk::DataObject::Pointer mitk::IGTLMessageSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return IGTLMessage::New().GetPointer();
}

itk::DataObject::Pointer mitk::IGTLMessageSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
  {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
  }
  return static_cast<itk::DataObject *>(IGTLMessage::New().GetPointer());
}

mitk::PropertyList::ConstPointer mitk::IGTLMessageSource::GetParameters() const
{
  mitk::PropertyList::Pointer p = mitk::PropertyList::New();
  // add properties to p like this:
  //p->SetProperty("MyFilter_MyParameter", mitk::PropertyDataType::New(m_MyParameter));
  return mitk::PropertyList::ConstPointer(p);
}

void mitk::IGTLMessageSource::SetFPS(unsigned int fps)
{
  this->m_StreamingFPSMutex->Lock();
  this->m_StreamingFPS = fps;
  this->m_StreamingFPSMutex->Unlock();
}


unsigned int mitk::IGTLMessageSource::GetFPS()
{
  unsigned int fps = 0;
  this->m_StreamingFPSMutex->Lock();
  fps = this->m_StreamingFPS;
  this->m_StreamingFPSMutex->Unlock();
  return fps;
}
