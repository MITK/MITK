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

#include <sstream>


template <class VTKWRITER>
mitk::SurfaceVtkWriter<VTKWRITER>::SurfaceVtkWriter()
{
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfOutputs( 1 );
  this->SetNthOutput( 0, mitk::Surface::New().GetPointer() );
  
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
    ::itk::OStringStream filename;
    
    int t, timesteps;
    const mitk::TimeBounds& timebounds = input->GetTimeSlicedGeometry()->GetTimeBoundsInMS();

    timesteps = input->GetTimeSlicedGeometry()->GetTimeSteps();
    for(t = 0; t < timesteps; ++t)
    {
      filename.flush();
      filename << "w";//m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] "_T" << t << m_Extension;
      m_VtkWriter->SetFileName(filename.str().c_str());
      m_VtkWriter->SetInput(input->GetVtkPolyData(t));
    }
  }
  else
  {
    m_VtkWriter->SetFileName(m_FileName.c_str());
    m_VtkWriter->SetInput(input->GetVtkPolyData());
  }

  m_VtkWriter->Write();
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
    return 0;
  }
  else
  {
    return static_cast< const mitk::Surface * >( this->ProcessObject::GetInput( 0 ) );
  }
}

template <class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::Write()
{
  GenerateData();
}