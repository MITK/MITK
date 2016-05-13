#ifndef MITKDIFFUSIONHEADERSIEMENSDICOMFILEHELPER_H
#define MITKDIFFUSIONHEADERSIEMENSDICOMFILEHELPER_H

#include <string>
#include <vector>

namespace mitk
{

enum SiemensDiffusionHeaderType {
  SIEMENS_CSA1 = 0,
  SIEMENS_CSA2
};

SiemensDiffusionHeaderType GetHeaderType( std::string header );

struct Siemens_Header_Format
{
  Siemens_Header_Format( size_t nlen,
                         size_t vm,
                         size_t vr,
                         size_t syngodt,
                         size_t nitems )
    : NameLength( nlen ),
      Delimiter( "\0" ),
      VM( vm ),
      VR( vr ),
      Syngodt( syngodt ),
      NumItems( nitems )
  {

  }

  size_t NameLength;
  std::string Delimiter;
  size_t VM;
  size_t VR;
  size_t Syngodt;
  size_t NumItems;
};

bool ParseInputString( std::string input, std::vector<double>& values, Siemens_Header_Format format_specs );

} //end namespace
#endif // MITKDIFFUSIONHEADERSIEMENSDICOMFILEHELPER_H
