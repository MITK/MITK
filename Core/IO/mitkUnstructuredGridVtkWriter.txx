/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:  $RCSfile$
Language:  C++
Date:    $Date$
Version:   $Revision: 10863 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUnstructuredGridVtkWriter.h"
class vtkDataObject;
#include <vtkConfigure.h>
#include <vtkUnstructuredGrid.h>
#include <vtkLinearTransform.h>
#include <vtkTransformFilter.h>
#include <sstream>

template <class VTKWRITER>
mitk::UnstructuredGridVtkWriter<VTKWRITER>::UnstructuredGridVtkWriter()
: m_WriterWriteHasReturnValue( false )
{
  this->SetNumberOfRequiredInputs( 1 );
  
  m_VtkWriter = VtkWriterType::New();

  //enable to write ascii-formatted-file
  //m_VtkWriter->SetFileTypeToASCII();

  SetDefaultExtension(); // and information about the Writer's Write() method
}

template <class VTKWRITER>
mitk::UnstructuredGridVtkWriter<VTKWRITER>::~UnstructuredGridVtkWriter()
{
  m_VtkWriter->Delete();
}

template <class VTKWRITER>
void mitk::UnstructuredGridVtkWriter<VTKWRITER>::SetDefaultExtension()
{
  m_Extension = ".vtk";
}

template<class VTKWRITER>
void mitk::UnstructuredGridVtkWriter<VTKWRITER>::ExecuteWrite( VtkWriterType* m_VtkWriter, vtkTransformFilter* /*transformPointSet*/ )
{
  m_VtkWriter->Write(); // without any return type. see .cpp for an alternative
}

template <class VTKWRITER>
void mitk::UnstructuredGridVtkWriter<VTKWRITER>::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  mitk::UnstructuredGrid::Pointer input = const_cast<mitk::UnstructuredGrid*>(this->GetInput());
  
  vtkTransformFilter* transformPointSet = vtkTransformFilter::New();
  vtkUnstructuredGrid * unstructuredGrid;
  Geometry3D* geometry;

  if(input->GetTimeSlicedGeometry()->GetTimeSteps()>1)
  {
    
    int t, timesteps;

    timesteps = input->GetTimeSlicedGeometry()->GetTimeSteps();
    for(t = 0; t < timesteps; ++t)
    {
      ::itk::OStringStream filename;
      geometry = input->GetGeometry(t);
      if(input->GetTimeSlicedGeometry()->IsValidTime(t))
      {
        const mitk::TimeBounds& timebounds = geometry->GetTimeBounds();
        filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << m_Extension;
      }
      else
      {
        itkWarningMacro(<<"Error on write: TimeSlicedGeometry invalid of unstructured grid " << filename << ".");
        filename <<  m_FileName.c_str() << "_T" << t << m_Extension;
      }
      geometry->TransferItkToVtkTransform();
      transformPointSet->SetInput(input->GetVtkUnstructuredGrid(t));
      transformPointSet->SetTransform(geometry->GetVtkTransform());
      transformPointSet->UpdateWholeExtent();
      unstructuredGrid = static_cast<vtkUnstructuredGrid*>(transformPointSet->GetOutput());
      
      m_VtkWriter->SetFileName(filename.str().c_str());
#if VTK_MAJOR_VERSION >= 5 
      m_VtkWriter->SetInput((vtkDataObject*)unstructuredGrid);
#else
      m_VtkWriter->SetInput(unstructuredGrid);
#endif
     
      ExecuteWrite( m_VtkWriter, transformPointSet );
    }
  }
  else
  {
    geometry = input->GetGeometry();
    geometry->TransferItkToVtkTransform();
    transformPointSet->SetInput(input->GetVtkUnstructuredGrid());
    transformPointSet->SetTransform(geometry->GetVtkTransform());
    transformPointSet->UpdateWholeExtent();
    unstructuredGrid = static_cast<vtkUnstructuredGrid*>(transformPointSet->GetOutput());
      
    m_VtkWriter->SetFileName(m_FileName.c_str());
#if VTK_MAJOR_VERSION >= 5 
    m_VtkWriter->SetInput((vtkDataObject*)unstructuredGrid);
#else
    m_VtkWriter->SetInput(unstructuredGrid);
#endif
    
    ExecuteWrite( m_VtkWriter, transformPointSet );
  }
  transformPointSet->Delete();
  m_MimeType = "image/ug";
}

template <class VTKWRITER>
void mitk::UnstructuredGridVtkWriter<VTKWRITER>::SetInput( mitk::UnstructuredGrid* ug )
{
  this->ProcessObject::SetNthInput( 0, ug );
}

template <class VTKWRITER>
const mitk::UnstructuredGrid* mitk::UnstructuredGridVtkWriter<VTKWRITER>::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return static_cast< const mitk::UnstructuredGrid * >( this->ProcessObject::GetInput( 0 ) );
  }
}

template <class VTKWRITER>
bool mitk::UnstructuredGridVtkWriter<VTKWRITER>::CanWrite( DataTreeNode* input )
{
  if ( input )
  {
    mitk::BaseData* data = input->GetData();
    if ( data )
    {
       mitk::UnstructuredGrid::Pointer ug = dynamic_cast<mitk::UnstructuredGrid*>( data );
       if( ug.IsNotNull() )
       {
         SetDefaultExtension();
         return true;
       }
    }
  }
  return false;
}

template <class VTKWRITER>
void mitk::UnstructuredGridVtkWriter<VTKWRITER>::SetInput( DataTreeNode* input )
{
  if( input && CanWrite( input ) )
   SetInput( dynamic_cast<mitk::UnstructuredGrid*>( input->GetData() ) );
}

template <class VTKWRITER>
std::string mitk::UnstructuredGridVtkWriter<VTKWRITER>::GetWritenMIMEType()
{
  return m_MimeType;
}

template <class VTKWRITER>
std::string mitk::UnstructuredGridVtkWriter<VTKWRITER>::GetFileExtension()
{
  return m_Extension;
}
