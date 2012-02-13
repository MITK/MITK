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
#include "mitkOverwriteSliceFilter.h"
#include <vtkTemplateAliasMacro.h>


mitk::OverwriteSliceFilter::OverwriteSliceFilter(){
	m_TimeStep = 0;
}

mitk::OverwriteSliceFilter::~OverwriteSliceFilter(){
	m_Slice = NULL;
}

void mitk::OverwriteSliceFilter::SetInputSlice(mitk::Image* slice)
{
	this->m_Slice = slice->GetVtkImageData();
}

void mitk::OverwriteSliceFilter::GenerateInputRequestedRegion(){
	//As we want all pixel information fo the image in our plane, the requested region
	//is set to the largest possible region in the image.
	//This is needed because an oblique plane has a larger extent then the image
	//and the in pipeline it is checked via PropagateResquestedRegion(). But the
	//extent of the slice is actually fitting because it is oblique within the image. 
	ImageToImageFilter::InputImagePointer input =  const_cast< ImageToImageFilter::InputImageType* > ( this->GetInput() );
	input->SetRequestedRegionToLargestPossibleRegion();
}



template<class F>
static void GetOverwritefunc(int dataType, void (**overwritefunc)(F *inPtr, F *outPtr, unsigned int *mapPtr, int todo)){

	switch (dataType)
	{
		vtkTemplateAliasMacro(*((void (**)(VTK_TT *outPtr, VTK_TT *inPtr,
			unsigned int *mapPtr, int todo))overwritefunc) = \
			&Overwrite);
	default:
		overwritefunc = 0;
	}
}


template<class T>
static void Overwrite(T *inPtr, T *outPtr, unsigned int *mapPtr, int todo){

	do{
		unsigned int shift = *mapPtr;
		mapPtr++;
		*(outPtr + shift) = *inPtr++;
	}while(--todo);

}




void mitk::OverwriteSliceFilter::GenerateData()
{
	vtkSmartPointer<vtkImageData> inputVolume = (const_cast< mitk::Image * >(ImageToImageFilter::GetInput()))->GetVtkImageData(m_TimeStep);

	void* inPtr = m_Slice->GetScalarPointer();

	void* outPtr = inputVolume->GetScalarPointer();


	unsigned int* mapPtr = m_Map;


	int* dimensions = m_Slice->GetDimensions();
	int todo = dimensions[0] * dimensions[1];


	void (*overwritefunc)(void *inPtr, void *outPtr, unsigned int *mapPtr, int todo);
	GetOverwritefunc(m_Slice->GetScalarType(),&overwritefunc);

	overwritefunc(inPtr, outPtr, mapPtr, todo);
	
	//mitk::Image::Pointer resultImage = this->GetOutput();

	////initialize resultimage with the specs of the vtkImageData object returned from vtkImageReslice
	//resultImage->Initialize(inputVolume);

	////transfer the voxel data
	//resultImage->SetVolume(inputVolume->GetScalarPointer());	

	this->GetInput()->Modified();

	delete [] m_Map;
	m_Map = NULL;
}
