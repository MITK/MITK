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
#include "mitkExtractSliceFilter.h"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>

mitk::ExtractSliceFilter::ExtractSliceFilter(){
	m_Reslicer = vtkSmartPointer<vtkImageReslice>::New();
}

mitk::ExtractSliceFilter::~ExtractSliceFilter(){
	
}

void mitk::ExtractSliceFilter::GenerateData(){

	mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );

	if ( !input){
		MITK_ERROR << "mitk::ExtractSliceFilter: No input image available. Please set the input!" << std::endl;
        itkExceptionMacro("mitk::ExtractSliceFilter: No input image available. Please set the input!");
        return;
    }

	if(!m_WorldGeometry){
		MITK_ERROR << "mitk::ExtractSliceFilter: No Geometry for reslicing available." << std::endl;
        itkExceptionMacro("mitk::ExtractSliceFilter: No Geometry for reslicing available.");
        return;
    }

	


/********** #BEGIN setup vtkImageRslice properties***********/

	m_Reslicer->SetInput(input->GetVtkImageData());


	/*setup the plane where vktImageReslice extracts the slice*/

	//ResliceAxesOrigin is the ancor point of the plane
	double origin[3];
	itk2vtk(m_WorldGeometry->GetOrigin(), origin);
	m_Reslicer->SetResliceAxesOrigin(origin);

	//the cosines define the plane: x and y are the direction vectors, n is the planes normal
	double cosines[9] = {
		1.0, 0.0, 0.0,  //x
		0.0, -1.0, 0.0, //y
		0.0, 0.0, 1.0   //n
	};
	m_Reslicer->SetResliceAxesDirectionCosines(cosines);	


	//we only have one slice, not a volume
	m_Reslicer->SetOutputDimensionality(2);

	//the default interpolation used by mitk
	m_Reslicer->SetInterpolationModeToNearestNeighbor();

	//start the pipeline
	m_Reslicer->Modified();
	m_Reslicer->Update();

/********** #END setup vtkImageRslice properties***********/




/********** #BEGIN Get the slice from vtkImageReslice and convert it to mitk Image***********/
	vtkImageData* reslicedImage;
	reslicedImage = m_Reslicer->GetOutput();



	if(!reslicedImage)
	{
		itkWarningMacro(<<"Reslicer returned empty image");
		return;
	}


	mitk::Image::Pointer resultImage = this->GetOutput();

	//initialize resultimage with the specs of the vtkImageData object returned from vtkImageReslice
	resultImage->Initialize(reslicedImage);

	////transfer the voxel data
	resultImage->SetVolume(reslicedImage->GetScalarPointer());	

	//set the geometry from current worldgeometry for the reusultimage
	AffineGeometryFrame3D::Pointer originalGeometryAGF = m_WorldGeometry->Clone();
	Geometry2D::Pointer originalGeometry = dynamic_cast<Geometry2D*>( originalGeometryAGF.GetPointer() );
	originalGeometry->ChangeImageGeometryConsideringOriginOffset(true);
    resultImage->SetGeometry( originalGeometry );

	resultImage->GetGeometry()->TransferItkToVtkTransform();

	
	resultImage->GetGeometry()->Modified();
/********** #END Get the slice from vtkImageReslice and convert it to mitk Image***********/
}