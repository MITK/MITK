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
  BaseGeometry* geometry;

  if(input->GetTimeGeometry()->CountTimeSteps()>1)
  {

    int t, timesteps;

    timesteps = input->GetTimeGeometry()->CountTimeSteps();
    for(t = 0; t < timesteps; ++t)
    {
      std::ostringstream filename;
      geometry = input->GetGeometry(t);
      if(input->GetTimeGeometry()->IsValidTimeStep(t))
      {
        const mitk::TimeBounds& timebounds = input->GetTimeGeometry()->GetTimeBounds(t);
        filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << GetDefaultExtension();
      }
      else
      {
        itkWarningMacro(<<"Error on write: TimeGeometry invalid of unstructured grid " << filename.str() << ".");
        filename <<  m_FileName.c_str() << "_T" << t << GetDefaultExtension();
      }
      transformPointSet->SetInputData(input->GetVtkUnstructuredGrid(t));
      transformPointSet->SetTransform(geometry->GetVtkTransform());
      transformPointSet->UpdateWholeExtent();
      unstructuredGrid = static_cast<vtkUnstructuredGrid*>(transformPointSet->GetOutput());

      unstructuredGridWriter->SetFileName(filename.str().c_str());
      unstructuredGridWriter->SetInputData(unstructuredGrid);

      ExecuteWrite( unstructuredGridWriter );
    }
  }
  else
  {
    geometry = input->GetGeometry();
    transformPointSet->SetInputData(input->GetVtkUnstructuredGrid());
    transformPointSet->SetTransform(geometry->GetVtkTransform());
    transformPointSet->UpdateWholeExtent();
    unstructuredGrid = static_cast<vtkUnstructuredGrid*>(transformPointSet->GetOutput());

    unstructuredGridWriter->SetFileName(m_FileName.c_str());
    unstructuredGridWriter->SetInputData(unstructuredGrid);

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
void UnstructuredGridVtkWriter<VTKWRITER>::SetInput(BaseData *input)
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

}

#endif
