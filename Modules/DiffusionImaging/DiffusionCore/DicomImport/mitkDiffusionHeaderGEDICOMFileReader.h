#ifndef MITKDIFFUSIONHEADERGEDICOMFILEREADER_H
#define MITKDIFFUSIONHEADERGEDICOMFILEREADER_H

#include "MitkDiffusionCoreExports.h"
#include "mitkDiffusionHeaderDICOMFileReader.h"

namespace mitk
{

class MitkDiffusionCore_EXPORT DiffusionHeaderGEDICOMFileReader
    : public DiffusionHeaderDICOMFileReader
{
public:

  mitkClassMacro( DiffusionHeaderGEDICOMFileReader, DiffusionHeaderDICOMFileReader )
  itkNewMacro( Self )

  virtual bool ReadDiffusionHeader(std::string filename);

protected:
  DiffusionHeaderGEDICOMFileReader();

  virtual ~DiffusionHeaderGEDICOMFileReader();
};

}

#endif // MITKDIFFUSIONHEADERGEDICOMFILEREADER_H
