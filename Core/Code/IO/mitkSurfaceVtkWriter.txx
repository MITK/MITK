/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include <vtkConfigure.h>
#include <vtkPolyData.h>
#include <vtkLinearTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkErrorCode.h>

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

template <class VTKWRITER>
mitk::SurfaceVtkWriter<VTKWRITER>::SurfaceVtkWriter()
: m_WriterWriteHasReturnValue( false )
{
  this->SetNumberOfRequiredInputs( 1 );

  m_VtkWriter = vtkSmartPointer<VtkWriterType>::New();

  //enable to write ascii-formatted-file
  //m_VtkWriter->SetFileTypeToASCII();

  SetDefaultExtension(); // and information about the Writer's Write() method
}

template <class VTKWRITER>
mitk::SurfaceVtkWriter<VTKWRITER>::~SurfaceVtkWriter()
{
}

template <class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::SetDefaultExtension()
{
  m_Extension = ".vtk";
}

template<class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::ExecuteWrite( VtkWriterType* vtkWriter )
{
  if ( vtkWriter->Write() == 0 || vtkWriter->GetErrorCode() != 0 )
  {
    itkExceptionMacro(<<"Error during surface writing: " << vtkErrorCode::GetStringFromErrorCode(vtkWriter->GetErrorCode()) );
  }
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

  vtkSmartPointer<vtkTransformPolyDataFilter> transformPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  vtkPolyData * polyData;
  Geometry3D* geometry;

  unsigned int t, timesteps = input->GetTimeSlicedGeometry()->GetTimeSteps();

  for(t = 0; t < timesteps; ++t)
  {
    // surfaces do not have to exist in all timeteps; therefor, only write valid surfaces
    if( input->GetVtkPolyData(t) == NULL ) continue;

    ::itk::OStringStream filename;
    geometry = input->GetGeometry(t);
    if ( timesteps > 1 )
    {
      if(input->GetTimeSlicedGeometry()->IsValidTime(t))
      {
        const TimeBounds& timebounds = geometry->GetTimeBounds();
        filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << m_Extension;
      }
      else
      {
        itkWarningMacro(<<"Error on write: TimeSlicedGeometry invalid of surface " << filename << ".");
        filename <<  m_FileName.c_str() << "_T" << t << m_Extension;
      }
      m_VtkWriter->SetFileName(filename.str().c_str());
    }
    else 
      m_VtkWriter->SetFileName(m_FileName.c_str());

    geometry->TransferItkToVtkTransform();
    transformPolyData->SetInput(input->GetVtkPolyData(t));
    transformPolyData->SetTransform(geometry->GetVtkTransform());
    transformPolyData->UpdateWholeExtent();
    polyData = transformPolyData->GetOutput();

    m_VtkWriter->SetInput(polyData);

    ExecuteWrite( m_VtkWriter );
  }

  m_MimeType = "application/MITK.Surface";
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
    return static_cast< const Surface * >( this->ProcessObject::GetInput( 0 ) );
  }
}

template <class VTKWRITER>
bool mitk::SurfaceVtkWriter<VTKWRITER>::CanWriteDataType( DataNode* input )
{
  if ( input )
  {
    BaseData* data = input->GetData();
    if ( data )
    {
      Surface::Pointer surface = dynamic_cast<Surface*>( data );
      if( surface.IsNotNull() )
      {
        SetDefaultExtension();
        return true;
      }
    }
  }
  return false;
}

template <class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::SetInput( DataNode* input )
{
  if( input && CanWriteDataType( input ) )
    SetInput( dynamic_cast<Surface*>( input->GetData() ) );
}

template <class VTKWRITER>
std::string mitk::SurfaceVtkWriter<VTKWRITER>::GetWritenMIMEType()
{
  return m_MimeType;
}

template <class VTKWRITER>
std::string mitk::SurfaceVtkWriter<VTKWRITER>::GetFileExtension()
{
  return m_Extension;
}
