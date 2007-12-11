/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkSubImageSelector.h"
#include <itkSmartPointerForwardReference.txx>

//template class itk::SmartPointerForwardReference<ImageDataItem>;


//##ModelId=3E0B4663009F
void mitk::SubImageSelector::SetPosNr(int /*p*/)
{
}

//##ModelId=3E1A0FC903A5
mitk::Image::ImageDataItemPointer mitk::SubImageSelector::GetSliceData(int s, int t, int n)
{
	mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
	return input->GetSliceData(s,t,n);
}

mitk::Image::ImageDataItemPointer mitk::SubImageSelector::GetVolumeData(int t, int n)
{
	mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
	return input->GetVolumeData(t,n);
}

mitk::Image::ImageDataItemPointer mitk::SubImageSelector::GetChannelData(int n)
{
	mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
	return input->GetChannelData(n);
}

//##ModelId=3E1A123E0396
void mitk::SubImageSelector::SetChannelItem(mitk::Image::ImageDataItemPointer dataItem, int n)
{
	mitk::Image::Pointer output  = this->GetOutput();
	if(output->IsValidChannel(n)==false) return;
	output->m_Channels[n]=dataItem;
}

void mitk::SubImageSelector::SetVolumeItem(mitk::Image::ImageDataItemPointer dataItem, int t, int n)
{
	mitk::Image::Pointer output  = this->GetOutput();
	if(output->IsValidVolume(t,n)==false) return;
  int pos;
  pos=output->GetVolumeIndex(t,n);
	output->m_Volumes[pos]=dataItem;
}

void mitk::SubImageSelector::SetSliceItem(mitk::Image::ImageDataItemPointer dataItem, int s, int t, int n)
{
	mitk::Image::Pointer output  = this->GetOutput();
	if(output->IsValidSlice(s,t,n)==false) return;
  int pos;
  pos=output->GetSliceIndex(s,t,n);
	output->m_Slices[pos]=dataItem;
}

//##ModelId=3E1B19AD02A6
mitk::SubImageSelector::SubImageSelector()
{
}


//##ModelId=3E1B19AD02CE
mitk::SubImageSelector::~SubImageSelector()
{
}

