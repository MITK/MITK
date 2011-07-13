/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#ifndef _MITK_UNSTRUCTURED_GRID_VTKWRITER_TXX_
#define _MITK_UNSTRUCTURED_GRID_VTKWRITER_TXX_

#include <itkLightObject.h>
#include <vtkUnstructuredGrid.h>
#include <vtkLinearTransform.h>
#include <vtkTransformFilter.h>
#include <vtkUnstructuredGridWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>

#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>

namespace mitk {

template<class VTKWRITER>
UnstructuredGridVtkWriter<VTKWRITER>::UnstructuredGridVtkWriter()
  : m_Success(false)
{
  this->SetNumberOfRequiredInputs(1);
}

template<class VTKWRITER>
UnstructuredGridVtkWriter<VTKWRITER>::~UnstructuredGridVtkWriter()
{

}

template<class VTKWRITER>
void UnstructuredGridVtkWriter<VTKWRITER>::GenerateData()
{
  m_Success = false;
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  mitk::UnstructuredGrid::Pointer input = const_cast<mitk::UnstructuredGrid*>(this->GetInput());

  if (input.IsNull())
  {
    itkWarningMacro( << "Sorry, input to mitk::UnstructuredGridVtkWriter is NULL");
    return;
  }

  VTKWRITER* unstructuredGridWriter = VTKWRITER::New();
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
        filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << GetDefaultExtension();
      }
      else
      {
        itkWarningMacro(<<"Error on write: TimeSlicedGeometry invalid of unstructured grid " << filename << ".");
        filename <<  m_FileName.c_str() << "_T" << t << GetDefaultExtension();
      }
      geometry->TransferItkToVtkTransform();
      transformPointSet->SetInput(input->GetVtkUnstructuredGrid(t));
      transformPointSet->SetTransform(geometry->GetVtkTransform());
      transformPointSet->UpdateWholeExtent();
      unstructuredGrid = static_cast<vtkUnstructuredGrid*>(transformPointSet->GetOutput());

      unstructuredGridWriter->SetFileName(filename.str().c_str());
      unstructuredGridWriter->SetInput(unstructuredGrid);

      ExecuteWrite( unstructuredGridWriter );
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

    unstructuredGridWriter->SetFileName(m_FileName.c_str());
    unstructuredGridWriter->SetInput(unstructuredGrid);

    ExecuteWrite( unstructuredGridWriter );
  }
  transformPointSet->Delete();
  unstructuredGridWriter->Delete();

  m_Success = true;
}

template<class VTKWRITER>
void UnstructuredGridVtkWriter<VTKWRITER>::ExecuteWrite( VTKWRITER* vtkWriter )
{
  struct stat fileStatus;
  time_t timeBefore=0;
  if (!stat(vtkWriter->GetFileName(), &fileStatus))
  {
    timeBefore = fileStatus.st_mtime;
  }
  if (!vtkWriter->Write())
  {
    itkExceptionMacro( << "Error during unstructured grid writing.");
  }

  // check if file can be written because vtkWriter doesn't check that
  if (stat(vtkWriter->GetFileName(), &fileStatus) || (timeBefore == fileStatus.st_mtime))
  {
    itkExceptionMacro(<<"Error during unstructured grid writing: file could not be written");
  }
}

template<class VTKWRITER>
void UnstructuredGridVtkWriter<VTKWRITER>::SetInput(UnstructuredGrid *input)
{
  this->ProcessObject::SetNthInput(0, input);
}

template<class VTKWRITER>
const UnstructuredGrid* UnstructuredGridVtkWriter<VTKWRITER>::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
  {
    return 0;
  }
  else
  {
    return dynamic_cast<UnstructuredGrid*>(this->ProcessObject::GetInput(0));
  }
}

template<class VTKWRITER>
bool UnstructuredGridVtkWriter<VTKWRITER>::CanWriteBaseDataType(BaseData::Pointer data)
{
  return (dynamic_cast<mitk::UnstructuredGrid*>(data.GetPointer()) != 0);
}

template<class VTKWRITER>
void UnstructuredGridVtkWriter<VTKWRITER>::DoWrite(BaseData::Pointer data)
{
  if (CanWriteBaseDataType(data))
  {
    this->SetInput(dynamic_cast<mitk::UnstructuredGrid*>(data.GetPointer()));
    this->Update();
  }
}

template<class VTKWRITER>
std::vector<std::string> UnstructuredGridVtkWriter<VTKWRITER>::GetPossibleFileExtensions()
{
  throw std::exception(); // no specialization available!
}

template<class VTKWRITER>
const char* UnstructuredGridVtkWriter<VTKWRITER>::GetDefaultFilename()
{
  throw std::exception(); // no specialization available!
}

template<class VTKWRITER>
const char* UnstructuredGridVtkWriter<VTKWRITER>::GetFileDialogPattern()
{
  throw std::exception(); // no specialization available!
}

template<class VTKWRITER>
const char* UnstructuredGridVtkWriter<VTKWRITER>::GetDefaultExtension()
{
  throw std::exception(); // no specialization available!
}

template<>
std::vector<std::string> UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetPossibleFileExtensions();

template<>
std::vector<std::string> UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetPossibleFileExtensions();

template<>
std::vector<std::string> UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetPossibleFileExtensions();

template<>
const char * UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetDefaultFilename();

template<>
const char * UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetDefaultFilename();

template<>
const char * UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetDefaultFilename();

template<>
const char * UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetFileDialogPattern();

template<>
const char * UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetFileDialogPattern();

template<>
const char * UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetFileDialogPattern();

template<>
const char * UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>::GetDefaultExtension();

template<>
const char * UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>::GetDefaultExtension();

template<>
const char * UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>::GetDefaultExtension();

#ifndef __APPLE__
// On MacOS, we get duplicate symbol errors during linking of mitkCoreExt.
// It looks like the linker does not coalesce the two instantiations in
// mitkUnstructuredGridVtkWriter.cpp and mitkCoreExtObjectFactory.cpp
// (which both include this file)
template class MitkExt_EXPORT UnstructuredGridVtkWriter<vtkUnstructuredGridWriter>;
template class MitkExt_EXPORT UnstructuredGridVtkWriter<vtkXMLUnstructuredGridWriter>;
template class MitkExt_EXPORT UnstructuredGridVtkWriter<vtkXMLPUnstructuredGridWriter>;
#endif

}

#endif
