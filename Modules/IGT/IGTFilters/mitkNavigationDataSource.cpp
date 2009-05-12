/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkNavigationDataSource.h"



mitk::NavigationDataSource::NavigationDataSource() 
: itk::ProcessObject()
{
}

mitk::NavigationDataSource::~NavigationDataSource()
{
}


mitk::NavigationData* mitk::NavigationDataSource::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;

  return static_cast<NavigationData*>(this->ProcessObject::GetOutput(0));
}


mitk::NavigationData* mitk::NavigationDataSource::GetOutput(unsigned int idx)
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;
  return static_cast<NavigationData*>(this->ProcessObject::GetOutput(idx));
}


void mitk::NavigationDataSource::GraftOutput(itk::DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}


void mitk::NavigationDataSource::GraftNthOutput(unsigned int idx, itk::DataObject *graft)
{
  if ( idx >= this->GetNumberOfOutputs() )
  {
    itkExceptionMacro(<<"Requested to graft output " << idx << 
      " but this filter only has " << this->GetNumberOfOutputs() << " Outputs.");
  }  

  if ( !graft )
  {
    itkExceptionMacro(<<"Requested to graft output that is a NULL pointer" );
  }

  itk::DataObject* output = this->GetOutput(idx);

  // Call Graft on NavigationData to copy member data
  output->Graft( graft );
}

itk::ProcessObject::DataObjectPointer mitk::NavigationDataSource::MakeOutput( unsigned int /*idx */)
{
  mitk::NavigationData::Pointer p = mitk::NavigationData::New();
  return static_cast<itk::DataObject*>(p.GetPointer());
}

mitk::PropertyList::ConstPointer mitk::NavigationDataSource::GetParameters() const
{
  mitk::PropertyList::Pointer p = mitk::PropertyList::New();
  // add properties to p like this:
  //p->SetProperty("MyFilter_MyParameter", mitk::PropertyDataType::New(m_MyParameter));
  return mitk::PropertyList::ConstPointer(p);
}