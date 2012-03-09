/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "DiffSliceOperationApplier.h"
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

	if(!imageOperation)
		return;

	if(imageOperation->IsValid())
	{
		vtkSmartPointer<mitkVtkImageOverwrite> reslice = vtkSmartPointer<mitkVtkImageOverwrite>::New();

		//Set the slice as 'input'
		reslice->SetInputSlice(imageOperation->GetSlice());

		//set overwrite mode to true to write back to the image volume
		reslice->SetOverwriteMode(true);
		reslice->Modified();

		mitk::ExtractSliceFilter::Pointer extractor =	mitk::ExtractSliceFilter::New(reslice);
		extractor->SetInput( imageOperation->GetImage() );
		extractor->SetTimeStep( imageOperation->GetTimeStep() );
		extractor->SetWorldGeometry( dynamic_cast<Geometry2D*>(imageOperation->GetWorldGeometry()) );
		extractor->SetVtkOutputRequest(true);
		extractor->SetResliceTransformByGeometry( imageOperation->GetImage()->GetTimeSlicedGeometry()->GetGeometry3D( imageOperation->GetTimeStep() ) );

		extractor->Modified();
		extractor->Update();

		RenderingManager::GetInstance()->RequestUpdateAll();
		imageOperation->GetImage()->Modified();
	}
}

mitk::DiffSliceOperationApplier* mitk::DiffSliceOperationApplier::GetInstance()
{
  static DiffSliceOperationApplier* s_Instance = new DiffSliceOperationApplier();

  return s_Instance;
}