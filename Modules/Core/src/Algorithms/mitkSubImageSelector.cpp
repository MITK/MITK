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


#include "mitkSubImageSelector.h"


void mitk::SubImageSelector::SetPosNr(int /*p*/)
{
}

mitk::Image::ImageDataItemPointer mitk::SubImageSelector::GetVolumeData(int t, int n)
{
  mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
  return input->GetVolumeData(t,n);
}

mitk::Image::ImageDataItemPointer mitk::SubImageSelector::GetSliceData(int s, int t, int n)
{
  mitk::Image::Pointer input = const_cast<mitk::Image*>(this->GetInput());
  return input->GetSliceData(s, t, n);
}

void mitk::SubImageSelector::SetSliceData(void* data, int s, int t, int n)
{
  mitk::Image::Pointer output = this->GetOutput();
  output->SetSlice(data, s, t, n);
}

void mitk::SubImageSelector::SetVolumeItem(mitk::Image::ImageDataItemPointer dataItem, int t, int n)
{
  mitk::Image::Pointer output = this->GetOutput();
  if(output->IsValidVolume(t,n)==false) return;
  int pos;
  pos=output->GetVolumeIndex(t,n);
  output->m_Volumes[pos]=dataItem;
}

mitk::SubImageSelector::SubImageSelector()
{
}

mitk::SubImageSelector::~SubImageSelector()
{
}

