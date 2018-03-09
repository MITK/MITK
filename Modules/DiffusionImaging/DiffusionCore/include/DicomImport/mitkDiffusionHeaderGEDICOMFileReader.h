#ifndef MITKDIFFUSIONHEADERGEDICOMFILEREADER_H
#define MITKDIFFUSIONHEADERGEDICOMFILEREADER_H

#include "MitkDiffusionCoreExports.h"
#include "mitkDiffusionHeaderDICOMFileReader.h"

namespace mitk
{

class MITKDIFFUSIONCORE_EXPORT DiffusionHeaderGEDICOMFileReader
    : public DiffusionHeaderDICOMFileReader
{
public:

  mitkClassMacro( DiffusionHeaderGEDICOMFileReader, DiffusionHeaderDICOMFileReader )
  itkNewMacro( Self )

  bool ReadDiffusionHeader(std::string filename) override;

protected:
  DiffusionHeaderGEDICOMFileReader();

  ~DiffusionHeaderGEDICOMFileReader() override;
};

}

#endif // MITKDIFFUSIONHEADERGEDICOMFILEREADER_H
