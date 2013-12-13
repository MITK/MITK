#include "mitkDiffusionHeaderSiemensMosaicDICOMFileReader.h"

mitk::DiffusionHeaderSiemensMosaicDICOMFileReader::DiffusionHeaderSiemensMosaicDICOMFileReader()
  : DiffusionHeaderSiemensDICOMFileReader()
{

}

mitk::DiffusionHeaderSiemensMosaicDICOMFileReader
::~DiffusionHeaderSiemensMosaicDICOMFileReader()
{

}

bool mitk::DiffusionHeaderSiemensMosaicDICOMFileReader
::ReadDiffusionHeader(std::string filename)
{
  gdcm::Reader gdcmReader;
  gdcmReader.SetFileName( filename.c_str() );

  gdcmReader.Read();

  MITK_INFO << " -- Analyzing: " << filename;

  const gdcm::DataSet& dataset = gdcmReader.GetFile().GetDataSet();

  const gdcm::Tag t_sie_diffusion( 0x0029,0x1010 );
  std::string siemens_diffusionheader_str;
  if( RevealBinaryTag( t_sie_diffusion, dataset, siemens_diffusionheader_str ) )
  {
    mitk::DiffusionImageDICOMHeaderInformation header_values;

    this->ExtractSiemensDiffusionTagInformation( siemens_diffusionheader_str, header_values );


  }

}
