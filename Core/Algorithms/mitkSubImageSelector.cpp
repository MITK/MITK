#include "SubImageSelector.h"

//##ModelId=3E0B4663009F
void mitk::SubImageSelector::SetPosNr(int p)
{
}

//##ModelId=3E0B4E070066
itk::DataObject* mitk::SubImageSelector::GetDataObject()
{
	return NULL;
}

//##ModelId=3E0B4E0B0080
void* mitk::SubImageSelector::GetData()
{
	return GetOutput()->GetData();
}

//##ModelId=3E0B4E0E0048
vtkImageData* mitk::SubImageSelector::GetVtkImageData()
{
	return GetOutput()->GetVtkImageData();
}

//##ModelId=3E0B4E1403D6
ipPicDescriptor* mitk::SubImageSelector::GetPic()
{
	return GetOutput()->GetPic();
}

//##ModelId=3E1A0FC903A5
mitk::ImageDataItem::Pointer mitk::SubImageSelector::GetSliceData(int s, int t, int n)
{
	mitk::Image::Pointer input  = const_cast<mitk::Image*>(this->GetInput());
	return input->GetSliceData(s,t,n);
}

//##ModelId=3E1A123E0396
void mitk::SubImageSelector::SetDataItem(mitk::ImageDataItem::Pointer dataItem, int n)
{
	mitk::Image::Pointer output  = this->GetOutput();
	if(output->IsValidChannel(n)==false) return;
	output->m_Channels[n]=dataItem;
}

//##ModelId=3E1B19AD02A6
mitk::SubImageSelector::SubImageSelector()
{
}


//##ModelId=3E1B19AD02CE
mitk::SubImageSelector::~SubImageSelector()
{
}

