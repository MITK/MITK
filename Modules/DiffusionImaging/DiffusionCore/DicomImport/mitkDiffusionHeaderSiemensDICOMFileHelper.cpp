#include "mitkDiffusionHeaderSiemensDICOMFileHelper.h"

#include <cstdlib>
#include <cmath>

mitk::SiemensDiffusionHeaderType mitk::GetHeaderType( std::string header )
{
  // The CSA2 format begins with the string ‘SV10’, the CSA1 format does not.
  if( header.find("SV10") != std::string::npos )
  {
    return mitk::SIEMENS_CSA2;
  }
  else
  {
    return mitk::SIEMENS_CSA1;
  }
}

bool mitk::ParseInputString( std::string input, std::vector<double>& values, Siemens_Header_Format format_specs )
{

  // TODO : Compute offset based on the format_specs, where does the 84 come from???
  int offset = 84;
  int vm = *(input.c_str() + format_specs.NameLength );

  for (int k = 0; k < vm; k++)
  {
    int itemLength = *(input.c_str() + offset + 4);

    int strideSize = static_cast<int> (ceil(static_cast<double>(itemLength)/4) * 4);
    std::string valueString = input.substr( offset+16, itemLength );

    double value = atof( valueString.c_str() );
    values.push_back( value );

    offset += 16+strideSize;
  }

  return true;
}


