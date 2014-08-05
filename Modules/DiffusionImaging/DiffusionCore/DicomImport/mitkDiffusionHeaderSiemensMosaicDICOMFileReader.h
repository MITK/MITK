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

#ifndef MITKDIFFUSIONHEADERSIEMENSMOSAICDICOMFILEREADER_H
#define MITKDIFFUSIONHEADERSIEMENSMOSAICDICOMFILEREADER_H

#include "mitkDiffusionHeaderSiemensDICOMFileReader.h"

namespace mitk
{

class DiffusionHeaderSiemensMosaicDICOMFileReader
    : public DiffusionHeaderSiemensDICOMFileReader
{
public:
  mitkClassMacro( DiffusionHeaderSiemensMosaicDICOMFileReader,
                  DiffusionHeaderSiemensDICOMFileReader )
  itkNewMacro( Self )

  virtual bool ReadDiffusionHeader(std::string filename);

protected:
  DiffusionHeaderSiemensMosaicDICOMFileReader();

  virtual ~DiffusionHeaderSiemensMosaicDICOMFileReader();
};

}

#endif // MITKDIFFUSIONHEADERSIEMENSMOSAICDICOMFILEREADER_H
