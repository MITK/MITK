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

#include "mitkDiffSliceOperation.h"
#include <mitkExtractSliceFilter.h>
#include "mitkRenderingManager.h"
#include "mitkSegTool2D.h"
#include <mitkVtkImageOverwrite.h>
#include <mitkImageVtkAccessor.h>

// VTK
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


  // Check if the operation is valid
  if(imageOperation->IsValid())
  {
    //the actual overwrite filter (vtk)
    vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

    mitk::Image::Pointer slice = imageOperation->GetSlice();
    //Set the slice as 'input'
    ImageVtkAccessor accessor(slice);
    reslice->SetInputSlice(const_cast<vtkImageData*>(accessor.getVtkImageData()));

    //set overwrite mode to true to write back to the image volume
    reslice->SetOverwriteMode(true);
    reslice->Modified();

    //a wrapper for vtkImageOverwrite
    mitk::ExtractSliceFilter::Pointer extractor =  mitk::ExtractSliceFilter::New(reslice);
    extractor->SetInput( imageOperation->GetImage() );
    extractor->SetTimeStep( imageOperation->GetTimeStep() );
    extractor->SetWorldGeometry( dynamic_cast<PlaneGeometry*>(imageOperation->GetWorldGeometry()) );
    extractor->SetVtkOutputRequest(true);
    extractor->SetResliceTransformByGeometry( imageOperation->GetImage()->GetGeometry( imageOperation->GetTimeStep() ) );

    extractor->Modified();
    extractor->Update();

    //make sure the modification is rendered
    RenderingManager::GetInstance()->RequestUpdateAll();
    imageOperation->GetImage()->Modified();

    mitk::ExtractSliceFilter::Pointer extractor2 = mitk::ExtractSliceFilter::New();
    extractor2->SetInput( imageOperation->GetImage() );
    extractor2->SetTimeStep( imageOperation->GetTimeStep() );
    extractor2->SetWorldGeometry( dynamic_cast<PlaneGeometry*>(imageOperation->GetWorldGeometry()) );
    extractor2->SetResliceTransformByGeometry( imageOperation->GetImage()->GetGeometry( imageOperation->GetTimeStep() ) );
    extractor2->Modified();
    extractor2->Update();

    // TODO Move this code to SurfaceInterpolationController!
    mitk::Image::Pointer slice2 = extractor2->GetOutput();
    mitk::PlaneGeometry::Pointer plane = dynamic_cast<PlaneGeometry*>(imageOperation->GetWorldGeometry());
    slice2->DisconnectPipeline();
    mitk::SegTool2D::UpdateSurfaceInterpolation(slice2, imageOperation->GetImage(), plane, true);
  }
}

mitk::DiffSliceOperationApplier* mitk::DiffSliceOperationApplier::GetInstance()
{
  static DiffSliceOperationApplier*  s_Instance = new DiffSliceOperationApplier();
  return s_Instance;
}
