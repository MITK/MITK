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



mitk::OverwriteSliceFilter::OverwriteSliceFilter(){

	m_Map = vtkSmartPointer<vtkImageData>::New();
	m_TimeStep = 0;

}

mitk::OverwriteSliceFilter::~OverwriteSliceFilter(){
	m_Map = NULL;
	m_Slice = NULL;
}

void mitk::OverwriteSliceFilter::SetInputMap(mitk::Image* map){
	this->m_Map = map->GetVtkImageData(this->m_TimeStep);
}

void mitk::OverwriteSliceFilter::SetInputSlice(mitk::Image* slice){
	this->m_Slice = slice->GetVtkImageData(this->m_TimeStep); 
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

void mitk::OverwriteSliceFilter::GenerateData()
{
	
	//this->SetOutput( const_cast<ImageToImageFilter::OutputImageType>(GetInput()) );
}