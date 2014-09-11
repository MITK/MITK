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

#include "mitkNavigationDataSource.h"
#include "mitkUIDGenerator.h"

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

const std::string mitk::NavigationDataSource::US_INTERFACE_NAME = "org.mitk.services.NavigationDataSource";
const std::string mitk::NavigationDataSource::US_PROPKEY_DEVICENAME = US_INTERFACE_NAME + ".devicename";
const std::string mitk::NavigationDataSource::US_PROPKEY_ID = US_INTERFACE_NAME + ".id";
const std::string mitk::NavigationDataSource::US_PROPKEY_ISACTIVE = US_INTERFACE_NAME + ".isActive";

mitk::NavigationDataSource::NavigationDataSource()
  : itk::ProcessObject(), m_Name("NavigationDataSource (no defined type)")
{
}

mitk::NavigationDataSource::~NavigationDataSource()
{
}

mitk::NavigationData* mitk::NavigationDataSource::GetOutput()
{
  if (this->GetNumberOfIndexedOutputs() < 1)
    return NULL;

  return static_cast<NavigationData*>(this->ProcessObject::GetPrimaryOutput());
}

mitk::NavigationData* mitk::NavigationDataSource::GetOutput(DataObjectPointerArraySizeType idx)
{
  NavigationData* out = dynamic_cast<NavigationData*>( this->ProcessObject::GetOutput(idx) );
  if ( out == NULL && this->ProcessObject::GetOutput(idx) != NULL )
  {
    itkWarningMacro (<< "Unable to convert output number " << idx << " to type " <<  typeid( NavigationData ).name () );
  }
  return out;
}

mitk::NavigationData* mitk::NavigationDataSource::GetOutput(const std::string& navDataName)
{
  DataObjectPointerArray outputs = this->GetOutputs();
  for (DataObjectPointerArray::iterator it = outputs.begin(); it != outputs.end(); ++it)
    if (navDataName == (static_cast<NavigationData*>(it->GetPointer()))->GetName())
      return static_cast<NavigationData*>(it->GetPointer());
  return NULL;
}

itk::ProcessObject::DataObjectPointerArraySizeType mitk::NavigationDataSource::GetOutputIndex( std::string navDataName )
{
  DataObjectPointerArray outputs = this->GetOutputs();
  for (DataObjectPointerArray::size_type i = 0; i < outputs.size(); ++i)
    if (navDataName == (static_cast<NavigationData*>(outputs.at(i).GetPointer()))->GetName())
      return i;
  throw std::invalid_argument("output name does not exist");
}

void mitk::NavigationDataSource::RegisterAsMicroservice(){
  // Get Context
  us::ModuleContext* context = us::GetModuleContext();

  // Define ServiceProps
  us::ServiceProperties props;
  mitk::UIDGenerator uidGen = mitk::UIDGenerator ("org.mitk.services.NavigationDataSource.id_", 16);
  props[ US_PROPKEY_ID ] = uidGen.GetUID();
  props[ US_PROPKEY_DEVICENAME ] = m_Name;
  m_ServiceRegistration = context->RegisterService(this, props);
}

void mitk::NavigationDataSource::UnRegisterMicroservice(){
  if (m_ServiceRegistration != NULL) m_ServiceRegistration.Unregister();
  m_ServiceRegistration = 0;
}

std::string mitk::NavigationDataSource::GetMicroserviceID(){
  return this->m_ServiceRegistration.GetReference().GetProperty(US_PROPKEY_ID).ToString();
}

void mitk::NavigationDataSource::GraftOutput(itk::DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}

void mitk::NavigationDataSource::GraftNthOutput(unsigned int idx, itk::DataObject *graft)
{
  if ( idx >= this->GetNumberOfIndexedOutputs() )
  {
    itkExceptionMacro(<<"Requested to graft output " << idx <<
      " but this filter only has " << this->GetNumberOfIndexedOutputs() << " Outputs.");
  }

  if ( !graft )
  {
    itkExceptionMacro(<<"Requested to graft output with a NULL pointer object" );
  }

  itk::DataObject* output = this->GetOutput(idx);
  if ( !output )
  {
    itkExceptionMacro(<<"Requested to graft output that is a NULL pointer" );
  }
  // Call Graft on NavigationData to copy member data
  output->Graft( graft );
}

itk::DataObject::Pointer mitk::NavigationDataSource::MakeOutput ( DataObjectPointerArraySizeType /*idx*/ )
{
  return mitk::NavigationData::New().GetPointer();
}

itk::DataObject::Pointer mitk::NavigationDataSource::MakeOutput( const DataObjectIdentifierType & name )
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
  {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
  }
  return static_cast<itk::DataObject *>(mitk::NavigationData::New().GetPointer());
}

mitk::PropertyList::ConstPointer mitk::NavigationDataSource::GetParameters() const
{
  mitk::PropertyList::Pointer p = mitk::PropertyList::New();
  // add properties to p like this:
  //p->SetProperty("MyFilter_MyParameter", mitk::PropertyDataType::New(m_MyParameter));
  return mitk::PropertyList::ConstPointer(p);
}