/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:  $RCSfile$
Language:  C++
Date:    $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSurfaceVtkWriter.h"
class vtkDataObject;
#include <vtkConfigure.h>
#include <sstream>

template <class VTKWRITER>
mitk::SurfaceVtkWriter<VTKWRITER>::SurfaceVtkWriter()
{
  this->SetNumberOfRequiredInputs( 1 );
  
  m_VtkWriter = VtkWriterType::New();
  
  SetDefaultExtension();
}

template <class VTKWRITER>
mitk::SurfaceVtkWriter<VTKWRITER>::~SurfaceVtkWriter()
{
  m_VtkWriter->Delete();
}

template <class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::SetDefaultExtension()
{
  m_Extension = ".vtk";
}

template <class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  mitk::Surface::Pointer input = const_cast<mitk::Surface*>(this->GetInput());

  if(input->GetTimeSlicedGeometry()->GetTimeSteps()>1)
  {
    
    int t, timesteps;

    timesteps = input->GetTimeSlicedGeometry()->GetTimeSteps();
    for(t = 0; t < timesteps; ++t)
    {
      ::itk::OStringStream filename;
      if(input->GetTimeSlicedGeometry()->IsValidTime(t))
      {
        const mitk::TimeBounds& timebounds = input->GetTimeSlicedGeometry()->GetGeometry3D(t)->GetTimeBounds();
        filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << m_Extension;
      }
      else
      {
        itkWarningMacro(<<"Error on write: TimeSlicedGeometry invalid of surface " << filename << ".");
        filename <<  m_FileName.c_str() << "_T" << t << m_Extension;
      }
      m_VtkWriter->SetFileName(filename.str().c_str());
#if VTK_MAJOR_VERSION >= 5 
      m_VtkWriter->SetInput((vtkDataObject*)input->GetVtkPolyData(t));
#else
      m_VtkWriter->SetInput(input->GetVtkPolyData(t));
#endif
      m_VtkWriter->Write();
    }
  }
  else
  {
    m_VtkWriter->SetFileName(m_FileName.c_str());
#if VTK_MAJOR_VERSION >= 5 
    m_VtkWriter->SetInput((vtkDataObject*)input->GetVtkPolyData());
#else
    m_VtkWriter->SetInput(input->GetVtkPolyData());
#endif
    m_VtkWriter->Write();
  }

}

template <class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::SetInput( mitk::Surface* surface )
{
  this->ProcessObject::SetNthInput( 0, surface );
}

template <class VTKWRITER>
const mitk::Surface* mitk::SurfaceVtkWriter<VTKWRITER>::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return static_cast< const mitk::Surface * >( this->ProcessObject::GetInput( 0 ) );
  }
}
