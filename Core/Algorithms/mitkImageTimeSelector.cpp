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

  int dim=(input->GetDimension()<3?input->GetDimension():3);
	output->Initialize(input->GetPixelType(), dim, input->GetDimensions());

  // initialize geometry
  output->SetGeometry(input->GetSlicedGeometry(m_TimeNr)->Clone());

//  std::cout << "in timeSelector" << std::endl;
//  const float *spacing = input->GetSlicedGeometry()->GetSpacing();
//	std::cout << "   in: xres=" << spacing[0] << " yres=" << spacing[1] << " zres=" << spacing[2] << std::endl;
  
  output->SetPropertyList(input->GetPropertyList()->Clone());

//  spacing = input->GetSlicedGeometry()->GetSpacing();
//	std::cout << "   out: xres=" << spacing[0] << " yres=" << spacing[1] << " zres=" << spacing[2] << std::endl;
  
}

//##ModelId=3E3BD0CE0194
void mitk::ImageTimeSelector::GenerateData()
{
	SetDataItem(GetVolumeData(m_TimeNr, m_ChannelNr), 0);
}

