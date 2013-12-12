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

#include "mitkDiffusionHeaderSiemensDICOMFileReader.h"

#include "gdcmScanner.h"
#include "gdcmReader.h"

static bool ExtractBValue( std::string tag_value, float& bval)
{
  std::string::size_type tag_position =
      tag_value.find_first_of( "B_value", 0 );

  if( tag_position == std::string::npos )
  {
    MITK_ERROR << "No b value information found. ";
    return false;
  }

  return true;
}

static bool RevealBinaryTag(const gdcm::Tag tag, const gdcm::DataSet& dataset, std::string& target, bool verbose = true)
{
  if( dataset.FindDataElement( tag ) )
  {
    if(verbose) MITK_INFO << "Found tag " << tag.PrintAsPipeSeparatedString();

    const gdcm::DataElement& de = dataset.GetDataElement( tag );
    target = std::string( de.GetByteValue()->GetPointer(),
                        de.GetByteValue()->GetLength() );
    return true;

  }
  else
  {
    if(verbose) MITK_INFO << "Could not find tag " << tag.PrintAsPipeSeparatedString();
    return false;
  }
}

mitk::DiffusionHeaderSiemensDICOMFileReader
::DiffusionHeaderSiemensDICOMFileReader()
{

}

mitk::DiffusionHeaderSiemensDICOMFileReader
::~DiffusionHeaderSiemensDICOMFileReader()
{

}

bool mitk::DiffusionHeaderSiemensDICOMFileReader
::ReadDiffusionHeader(std::string filename)
{
   gdcm::Reader gdcmReader;
   gdcmReader.SetFileName( filename.c_str() );

   gdcmReader.Read();

   const gdcm::DataSet& dataset = gdcmReader.GetFile().GetDataSet();

   const gdcm::Tag t_sie_diffusion( 0x0029,0x1010 );
   const gdcm::Tag t_sie_diffusion_vec( 0x0029,0x100e );
   const gdcm::Tag t_sie_diffusion2( 0x0029,0x100c );

   std::string bval_str;
   if( RevealBinaryTag( t_sie_diffusion, dataset, bval_str ) )
   {
     float value;
     ExtractBValue( bval_str, value );
   }

   std::string test_str;
   bool test = RevealBinaryTag( t_sie_diffusion2, dataset, test_str );
   bool test2 = RevealBinaryTag( t_sie_diffusion_vec, dataset, test_str );


   // SIEMENS Dicom tag
  /* gdcm::Tag t_sie_diffusion( 0x0029,0x1010 );
   t_sie_diffusion.S
   gdcm::Tag t_sie_bvalue( 0x0019, 0x100c );

   gdcmScanner.AddTag( t_sie_diffusion );
   gdcmScanner.Scan( input_filenames );

   const char* sie_diffusion = gdcmScanner.GetValue( filename.c_str(), t_sie_diffusion );
   MITK_INFO << "got tag: " << sie_diffusion;*/

   return true;


}
