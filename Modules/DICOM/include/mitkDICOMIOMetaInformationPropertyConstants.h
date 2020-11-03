/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDICOMIOMETAINFORMATIONCONSTANTS_H_
#define MITKDICOMIOMETAINFORMATIONCONSTANTS_H_

#include <MitkDICOMExports.h>

#include "mitkPropertyKeyPath.h"

namespace mitk
{
  /**
   * @ingroup IO
   * @brief The IOMetaInformationPropertyConsants struct
   */
  struct MITKDICOM_EXPORT DICOMIOMetaInformationPropertyConstants
  {
    //Path to the property containing the name of the dicom reader configuration used to read the data
    static PropertyKeyPath READER_CONFIGURATION();
    //Path to the property containing the files the dicom reader used in a TemporoSpatialProperty
    static PropertyKeyPath READER_FILES();
    //Path to the property containing PixelSpacingInterpretationString for the read data
    static PropertyKeyPath READER_PIXEL_SPACING_INTERPRETATION_STRING();
    //Path to the property containing PixelSpacingInterpretation for the read data
    static PropertyKeyPath READER_PIXEL_SPACING_INTERPRETATION();
    //Path to the property containing ReaderImplementationLevelString for the read data
    static PropertyKeyPath READER_IMPLEMENTATION_LEVEL_STRING();
    //Path to the property containing ReaderImplementationLevel for the read data
    static PropertyKeyPath READER_IMPLEMENTATION_LEVEL();
    //Path to the property containing the indicator of the gantry tilt was corrected when reading the data
    static PropertyKeyPath READER_GANTRY_TILT_CORRECTED();
    //Path to the property containing the indicator of the data was read as 3D+t
    static PropertyKeyPath READER_3D_plus_t();
    //Path to the property containing the version of GDCM used to read the data
    static PropertyKeyPath READER_GDCM();
    //Path to the property containing the version of DCMTK used to read the data
    static PropertyKeyPath READER_DCMTK();
  };
}

#endif // MITKIOCONSTANTS_H_
