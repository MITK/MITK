#include "mitkImageTimeSelector.h"

//##ModelId=3E1B1975031E
mitk::ImageTimeSelector::ImageTimeSelector() : m_TimeNr(0), m_ChannelNr(0)
{
}


//##ModelId=3E1B1975033C
mitk::ImageTimeSelector::~ImageTimeSelector()
{
}

//##ModelId=3E3BD0CC0232
void mitk::ImageTimeSelector::GenerateOutputInformation()
{
	mitk::Image::ConstPointer input  = this->GetInput();
	mitk::Image::Pointer output = this->GetOutput();

	itkDebugMacro(<<"GenerateOutputInformation()");

	output->Initialize(input->GetPixelType(), (input->GetDimension()<3?input->GetDimension():3), input->GetDimensions());
}

//##ModelId=3E3BD0CE0194
void mitk::ImageTimeSelector::GenerateData()
{
	SetDataItem(GetVolumeData(m_TimeNr, m_ChannelNr), 0);
}

