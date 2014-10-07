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

#ifndef MITKDIFFUSIONHEADERSIEMENSDICOMFILEREADER_H
#define MITKDIFFUSIONHEADERSIEMENSDICOMFILEREADER_H

#include "MitkDiffusionCoreExports.h"

#include "mitkDiffusionHeaderDICOMFileReader.h"
#include "mitkDiffusionHeaderSiemensDICOMFileHelper.h"

namespace mitk
{

class MitkDiffusionCore_EXPORT DiffusionHeaderSiemensDICOMFileReader
    : public DiffusionHeaderDICOMFileReader
{
public:

  mitkClassMacro( DiffusionHeaderSiemensDICOMFileReader, DiffusionHeaderDICOMFileReader )
  itkNewMacro( Self )

  virtual bool ReadDiffusionHeader(std::string filename);

protected:
  DiffusionHeaderSiemensDICOMFileReader();

  ~DiffusionHeaderSiemensDICOMFileReader();

  bool ExtractSiemensDiffusionTagInformation( std::string tag_value, mitk::DiffusionImageDICOMHeaderInformation& values );

  std::vector< Siemens_Header_Format > m_SiemensFormatsCollection;
};

}
#endif // MITKDIFFUSIONHEADERSIEMENSDICOMFILEREADER_H
