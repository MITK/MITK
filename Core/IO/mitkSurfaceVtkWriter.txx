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
#include <vtkPolyData.h>
#include <vtkLinearTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

template <class VTKWRITER>
mitk::SurfaceVtkWriter<VTKWRITER>::SurfaceVtkWriter()
: m_WriterWriteHasReturnValue( false )
{
  this->SetNumberOfRequiredInputs( 1 );

  m_VtkWriter = VtkWriterType::New();

  //enable to write ascii-formatted-file
  //m_VtkWriter->SetFileTypeToASCII();

  SetDefaultExtension(); // and information about the Writer's Write() method
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

template<class VTKWRITER>
void mitk::SurfaceVtkWriter<VTKWRITER>::ExecuteWrite( VtkWriterType* m_VtkWriter, vtkTransformPolyDataFilter* )
{
  struct stat fileStatus;
  time_t timeBefore=0;
  if (stat(m_FileName.c_str(),&fileStatus))
  {
	timeBefore = fileStatus.st_mtime;
  }
   if (!m_VtkWriter->Write())
  {
    itkExceptionMacro(<<"Error during surface writing.");
  }
  // check if file really was written, because vtkWriter doesn't check that
  if (!stat(m_FileName.c_str(),&fileStatus)/*||(timeBefore==fileStatus.st_mtime)*/)
  {
    itkExceptionMacro(<<"Error during surface writing: file could not be written");
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

  vtkTransformPolyDataFilter* transformPolyData = vtkTransformPolyDataFilter::New();
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
        const mitk::TimeBounds& timebounds = geometry->GetTimeBounds();
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

#if VTK_MAJOR_VERSION >= 5 
    m_VtkWriter->SetInput((vtkDataObject*)polyData);
#else
    m_VtkWriter->SetInput(polyData);
#endif

    ExecuteWrite( m_VtkWriter, transformPolyData );
  }

  transformPolyData->Delete();
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
    return static_cast< const mitk::Surface * >( this->ProcessObject::GetInput( 0 ) );
  }
}

template <class VTKWRITER>
bool mitk::SurfaceVtkWriter<VTKWRITER>::CanWriteDataType( DataTreeNode* input )
{
  if ( input )
  {
    mitk::BaseData* data = input->GetData();
    if ( data )
    {
      mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface*>( data );
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
void mitk::SurfaceVtkWriter<VTKWRITER>::SetInput( DataTreeNode* input )
{
  if( input && CanWriteDataType( input ) )
    SetInput( dynamic_cast<mitk::Surface*>( input->GetData() ) );
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
