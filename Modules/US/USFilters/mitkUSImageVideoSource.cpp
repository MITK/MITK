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

#include "mitkUSImageVideoSource.h"



mitk::USImageVideoSource::USImageVideoSource()
: itk::ProcessObject()
{
    m_IsVideoReady = false;
    m_IsMetadataReady = false;
    m_IsGeometryReady = false;
}


mitk::USImageVideoSource::~USImageVideoSource()
{
}



void mitk::USImageVideoSource::SetVideoFileInput(std::string path)
{
  m_OpenCVVideoSource = mitk::OpenCVVideoSource::New();

  // Example: "C:\\Users\\maerz\\Videos\\Debut\\us.avi"
  m_OpenCVVideoSource->SetVideoFileInput(path.c_str(),true,false);
  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
}

    
void mitk::USImageVideoSource::SetCameraInput(int deviceID){
  m_OpenCVVideoSource->SetVideoCameraInput(deviceID);

  m_OpenCVVideoSource->StartCapturing();
  m_OpenCVVideoSource->FetchFrame();
  
  // Let's see if we have been successful
  m_IsVideoReady = m_OpenCVVideoSource->IsCapturingEnabled();
}

void mitk::USImageVideoSource::GenerateData()
{
  

}


mitk::USImage* mitk::USImageVideoSource::GetOutput()
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;

  return static_cast<USImage*>(this->ProcessObject::GetOutput(0));
}


mitk::USImage* mitk::USImageVideoSource::GetOutput(unsigned int idx)
{
  if (this->GetNumberOfOutputs() < 1)
    return NULL;
  return static_cast<USImage*>(this->ProcessObject::GetOutput(idx));
}


void mitk::USImageVideoSource::GraftOutput(itk::DataObject *graft)
{
  this->GraftNthOutput(0, graft);
}


void mitk::USImageVideoSource::GraftNthOutput(unsigned int idx, itk::DataObject *graft)
{
  if ( idx >= this->GetNumberOfOutputs() )
  {
    itkExceptionMacro(<<"Requested to graft output " << idx <<
      " but this filter only has " << this->GetNumberOfOutputs() << " Outputs.");
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
  // Call Graft on USImage to copy member data
  output->Graft( graft );
}


itk::ProcessObject::DataObjectPointer mitk::USImageVideoSource::MakeOutput( unsigned int /*idx */)
{
  mitk::USImage::Pointer p = mitk::USImage::New();
  return static_cast<itk::DataObject*>(p.GetPointer());
}