/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMIOMetaInformationPropertyConstants.h"

namespace mitk
{

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_FILES()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "files" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_PIXEL_SPACING_INTERPRETATION_STRING()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "PixelSpacingInterpretationString" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_PIXEL_SPACING_INTERPRETATION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "PixelSpacingInterpretation" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_IMPLEMENTATION_LEVEL_STRING()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "ReaderImplementationLevelString" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_IMPLEMENTATION_LEVEL()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "ReaderImplementationLevel" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_GANTRY_TILT_CORRECTED()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "GantyTiltCorrected" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_3D_plus_t()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "3D+t" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_GDCM()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "gdcm" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_DCMTK()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "dcmtk" });
  }

  PropertyKeyPath DICOMIOMetaInformationPropertyConstants::READER_CONFIGURATION()
  {
    return PropertyKeyPath({ "MITK", "IO", "reader", "DICOM", "configuration" });
  }
}
