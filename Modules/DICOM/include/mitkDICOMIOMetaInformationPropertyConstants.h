/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMIOMetaInformationPropertyConstants_h
#define mitkDICOMIOMetaInformationPropertyConstants_h

#include <MitkDICOMExports.h>

#include <mitkPropertyKeyPath.h>

namespace mitk
{
  /**
   * \ingroup DICOMModule
   * \brief Constants for property key paths used to store DICOM I/O meta information.
   *
   * These constants define the property key paths under which the DICOM reader stores
   * meta information about the reading process itself (reader configuration, pixel spacing
   * interpretation, gantry tilt correction status, library versions, etc.).
   *
   * \sa DICOMImageBlockDescriptor, DICOMFileReader
   */
  struct MITKDICOM_EXPORT DICOMIOMetaInformationPropertyConstants
  {
    /** \brief Path to the property containing the name of the DICOM reader configuration used to read the data. */
    static PropertyKeyPath READER_CONFIGURATION();
    /** \brief Path to the property containing the files the DICOM reader used, stored as a TemporoSpatialProperty. */
    static PropertyKeyPath READER_FILES();
    /** \brief Path to the property containing the PixelSpacingInterpretation as a human-readable string. */
    static PropertyKeyPath READER_PIXEL_SPACING_INTERPRETATION_STRING();
    /** \brief Path to the property containing the PixelSpacingInterpretation enum value. */
    static PropertyKeyPath READER_PIXEL_SPACING_INTERPRETATION();
    /** \brief Path to the property containing the ReaderImplementationLevel as a human-readable string. */
    static PropertyKeyPath READER_IMPLEMENTATION_LEVEL_STRING();
    /** \brief Path to the property containing the ReaderImplementationLevel enum value. */
    static PropertyKeyPath READER_IMPLEMENTATION_LEVEL();
    /** \brief Path to the property indicating whether gantry tilt was corrected when reading the data. */
    static PropertyKeyPath READER_GANTRY_TILT_CORRECTED();
    /** \brief Path to the property indicating whether the data was read as a 3D+t image. */
    static PropertyKeyPath READER_3D_plus_t();
    /** \brief Path to the property containing the version of GDCM used to read the data. */
    static PropertyKeyPath READER_GDCM();
    /** \brief Path to the property containing the version of DCMTK used to read the data. */
    static PropertyKeyPath READER_DCMTK();
  };
}

#endif
