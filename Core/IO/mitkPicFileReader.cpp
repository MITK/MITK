#include "PicFileReader.h"
#include <itkImageFileReader.h>

//##ModelId=3E1878400265
void mitk::PicFileReader::GenerateOutputInformation()
{
	mitk::Image::Pointer output = this->GetOutput();

	if ((output->IsInitialized()) && (this->GetMTime() <= m_ReadHeaderTime.GetMTime()))
		return;

	itkDebugMacro(<<"Reading file for GenerateOutputInformation()" << m_FileName);

	// Check to see if we can read the file given the name or prefix
	//
	if ( m_FileName == "" && m_FilePrefix == "" )
	{
		throw itk::ImageFileReaderException(__FILE__, __LINE__, "One of FileName or FilePrefix must be non-empty");
	}

	ipPicDescriptor* header=ipPicGetHeader(const_cast<char *>(m_FileName.c_str()), NULL);

    header=ipPicGetTags(const_cast<char *>(m_FileName.c_str()), header);

	if( header == NULL)
	{
		itk::ImageFileReaderException e(__FILE__, __LINE__);
		itk::OStringStream msg;
		msg << " Could not read file "
			<< m_FileName.c_str();
		e.SetDescription(msg.str().c_str());
		throw e;
		return;
	}

	output->Initialize(header);

	m_ReadHeaderTime.Modified();
}

//##ModelId=3E187255016C
void mitk::PicFileReader::GenerateData()
{
	mitk::Image::Pointer output = this->GetOutput();

	char * c=const_cast<char *>(m_FileName.c_str());
	ipPicDescriptor* pic=ipPicGet(const_cast<char *>(m_FileName.c_str()), NULL);

	output->SetPicVolume(pic);
}

//##ModelId=3E1874D202D0
mitk::PicFileReader::PicFileReader()
{
}

//##ModelId=3E1874E50179
mitk::PicFileReader::~PicFileReader()
{
}
