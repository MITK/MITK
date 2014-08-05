#include "mitkDiffusionHeaderSiemensDICOMFileHelper.h"
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
    mitk::DiffusionImageMosaicDICOMHeaderInformation header_values;

    this->ExtractSiemensDiffusionTagInformation( siemens_diffusionheader_str, header_values );

    //MITK_INFO << siemens_diffusionheader_str;

    mitk::SiemensDiffusionHeaderType hformat = GetHeaderType( siemens_diffusionheader_str );
    Siemens_Header_Format specs = this->m_SiemensFormatsCollection.at( hformat );

    std::string::size_type tag_position = siemens_diffusionheader_str.find( "NumberOfImagesInMosaic", 0 );
    if( tag_position != std::string::npos )
    {
      std::vector<double> value_array;
      ParseInputString( siemens_diffusionheader_str.substr( tag_position, siemens_diffusionheader_str.size() - tag_position + 1 ),
                        value_array,
                        specs
                        );

      header_values.n_images = value_array[0];
    }

    tag_position = siemens_diffusionheader_str.find("SliceNormalVector", 0);
    if( tag_position != std::string::npos )
    {
      std::vector<double> value_array;
      ParseInputString( siemens_diffusionheader_str.substr( tag_position, siemens_diffusionheader_str.size() - tag_position + 1 ),
                        value_array,
                        specs
                        );

      MITK_DEBUG << "SliceNormal";
      for( unsigned int i=0; i<value_array.size(); i++)
      {
        MITK_DEBUG << value_array.at(i);
      }

      if( value_array.size() > 2 )
      {
        header_values.slicenormalup = (value_array[2] > 0);
      }

    }

    m_HeaderInformationList.push_back( header_values );

  }

  return (m_HeaderInformationList.size() > 0);

}
