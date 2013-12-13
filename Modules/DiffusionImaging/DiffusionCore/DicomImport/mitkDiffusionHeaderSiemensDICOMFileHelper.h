#ifndef MITKDIFFUSIONHEADERSIEMENSDICOMFILEHELPER_H
#define MITKDIFFUSIONHEADERSIEMENSDICOMFILEHELPER_H


enum SiemensDiffusionHeaderType {
  SIEMENS_CSA1 = 0,
  SIEMENS_CSA2
};

static SiemensDiffusionHeaderType GetHeaderType( std::string header )
{
  // The CSA2 format begins with the string ‘SV10’, the CSA1 format does not.
  if( header.find("SV10") != std::string::npos )
  {
    return SIEMENS_CSA2;
  }
  else
  {
    return SIEMENS_CSA1;
  }
}

struct Siemens_Header_Format
{
  Siemens_Header_Format( size_t nlen,
                         size_t vm,
                         size_t vr,
                         size_t syngodt,
                         size_t nitems )
    : NameLength( nlen ),
      VM( vm ),
      VR( vr ),
      Syngodt( syngodt ),
      NumItems( nitems ),
      Delimiter( "\0" )
  {}
  size_t NameLength;
  std::string Delimiter;
  size_t VM;
  size_t VR;
  size_t Syngodt;
  size_t NumItems;
};

static bool ParseInputString( std::string input, std::vector<double>& values, Siemens_Header_Format format_specs )
{

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


#endif // MITKDIFFUSIONHEADERSIEMENSDICOMFILEHELPER_H
