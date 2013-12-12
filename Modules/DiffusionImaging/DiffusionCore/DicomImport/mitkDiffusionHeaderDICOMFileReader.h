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

#ifndef MITKDIFFUSIONHEADERFILEREADER_H
#define MITKDIFFUSIONHEADERFILEREADER_H

#include <DiffusionCoreExports.h>

#include <itkLightObject.h>
#include "mitkDiffusionImage.h"

namespace mitk
{

class DiffusionCore_EXPORT DiffusionHeaderDICOMFileReader
    : public itk::LightObject
{
public:


  mitkClassMacro( DiffusionHeaderDICOMFileReader, itk::LightObject )

  /**
   * @brief IsDiffusionHeader Parse the given dicom file and collect the special diffusion image information
   * @return
   */
  virtual bool ReadDiffusionHeader( std::string ) = 0;

protected:
  DiffusionHeaderDICOMFileReader();

  virtual ~DiffusionHeaderDICOMFileReader();
};

} // end namespace mitk

#endif // MITKDIFFUSIONHEADERFILEREADER_H
