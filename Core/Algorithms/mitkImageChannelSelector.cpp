#include "ImageChannelSelector.h"

//##ModelId=3E1B1980039C
mitk::ImageChannelSelector::ImageChannelSelector() : m_ChannelNr(0)
{
}


//##ModelId=3E1B198003B0
mitk::ImageChannelSelector::~ImageChannelSelector()
{
}

//##ModelId=3E3BD0C70343
void mitk::ImageChannelSelector::GenerateOutputInformation()
{
	mitk::Image::ConstPointer input  = this->GetInput();
	mitk::Image::Pointer output = this->GetOutput();

	itkDebugMacro(<<"GenerateOutputInformation()");

    output->Initialize(input->GetType(), input->GetDimension(), input->GetDimensions(), 1);
}

//##ModelId=3E3BD0C903DC
void mitk::ImageChannelSelector::GenerateData()
{
	itkExceptionMacro("not yet implemented.");
}

