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

#ifndef MITKDIFFUSIONHEADERPHILIPSDICOMREADER_H
#define MITKDIFFUSIONHEADERPHILIPSDICOMREADER_H


#include <MitkDiffusionCoreExports.h>
#include "mitkDiffusionHeaderDICOMFileReader.h"

namespace mitk
{

class MitkDiffusionCore_EXPORT DiffusionHeaderPhilipsDICOMFileReader
    : public DiffusionHeaderDICOMFileReader
{
public:

  mitkClassMacro( DiffusionHeaderPhilipsDICOMFileReader, DiffusionHeaderDICOMFileReader )
  itkNewMacro( Self )

  virtual bool ReadDiffusionHeader(std::string filename);

protected:
  DiffusionHeaderPhilipsDICOMFileReader();

  virtual ~DiffusionHeaderPhilipsDICOMFileReader();
};

}

#endif // MITKDIFFUSIONHEADERPHILIPSDICOMREADER_H
