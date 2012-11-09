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

#include "mitkDiffSliceOperationApplier.h"
#include "mitkRenderingManager.h"
#include <vtkSmartPointer.h>

mitk::DiffSliceOperationApplier::DiffSliceOperationApplier()
{
}

mitk::DiffSliceOperationApplier::~DiffSliceOperationApplier()
{
}

void mitk::DiffSliceOperationApplier::ExecuteOperation( Operation* operation )
{
  DiffSliceOperation* imageOperation = dynamic_cast<DiffSliceOperation*>( operation );

  //as we only support DiffSliceOperation return if operation is not type of DiffSliceOperation
  if(!imageOperation)
    return;


  //chak if the operation is valid
  if(imageOperation->IsValid())
  {
    //the actual overwrite filter (vtk)
    vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

    //Set the slice as 'input'
    reslice->SetInputSlice(imageOperation->GetSlice());

    //set overwrite mode to true to write back to the image volume
    reslice->SetOverwriteMode(true);
    reslice->Modified();

    //a wrapper for vtkImageOverwrite
    mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
    extractor->SetInput( imageOperation->GetImage() );
    extractor->SetTimeStep( imageOperation->GetTimeStep() );
    extractor->SetWorldGeometry( dynamic_cast<Geometry2D*>(imageOperation->GetWorldGeometry()) );
    extractor->SetVtkOutputRequest(true);
    extractor->SetResliceTransformByGeometry( imageOperation->GetImage()->GetTimeSlicedGeometry()->GetGeometry3D( imageOperation->GetTimeStep() ) );

    extractor->Modified();
    extractor->Update();

    //make sure the modification is rendered
    RenderingManager::GetInstance()->RequestUpdateAll();
    imageOperation->GetImage()->Modified();
  }
}

//mitk::DiffSliceOperationApplier* mitk::DiffSliceOperationApplier::s_Instance = NULL;

mitk::DiffSliceOperationApplier* mitk::DiffSliceOperationApplier::GetInstance()
{
  //if(!s_Instance)
  static DiffSliceOperationApplier*  s_Instance = new DiffSliceOperationApplier();

  return s_Instance;
}