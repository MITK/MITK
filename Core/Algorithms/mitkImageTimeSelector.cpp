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
  SlicedGeometry3D::Pointer geometry = SlicedGeometry3D::New();
  geometry->Initialize(input->GetDimensions()[2], 1);
  geometry->SetGeometry2D(input->GetSlicedGeometry()->GetGeometry2D(0, m_TimeNr).GetPointer(), 0, 0);
  if(input->GetSlicedGeometry()->GetEvenlySpaced())
    geometry->SetEvenlySpaced();
  else
  {
    int s;
    for(s=1;s<dim;++s)
    {
      geometry->SetGeometry2D(input->GetSlicedGeometry()->GetGeometry2D(s, m_TimeNr), s, 0);
    }
  }

//  std::cout << "in timeSelector" << std::endl;
//  const float *spacing = input->GetSlicedGeometry()->GetSpacing();
//	std::cout << "   in: xres=" << spacing[0] << " yres=" << spacing[1] << " zres=" << spacing[2] << std::endl;
  
  geometry->SetSpacing(input->GetSlicedGeometry()->GetSpacing());

  output->SetPropertyList(input->GetPropertyList()->Clone());
  output->SetGeometry(geometry);

//  spacing = input->GetSlicedGeometry()->GetSpacing();
//	std::cout << "   out: xres=" << spacing[0] << " yres=" << spacing[1] << " zres=" << spacing[2] << std::endl;
  
}

//##ModelId=3E3BD0CE0194
void mitk::ImageTimeSelector::GenerateData()
{
	SetDataItem(GetVolumeData(m_TimeNr, m_ChannelNr), 0);
}

