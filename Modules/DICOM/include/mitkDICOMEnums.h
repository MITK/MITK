/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMEnums_h
#define mitkDICOMEnums_h

#include <string>
#include <vector>
#include <utility>

namespace mitk
{
  /** \brief A list of strings, used throughout the DICOM module to hold file paths and similar data.
   * \sa DICOMFilePathList
   */
  typedef std::vector<std::string> StringList;

  /** \brief A list of boolean values, used e.g. for tracking per-slice loaded states. */
  typedef std::vector<bool> BoolList;

  /**
   \ingroup DICOMModule
   \brief How the mitk::Image spacing should be interpreted (see mitk::DICOMFileReader).

   Compare DICOM PS 3.3 10.7 (Basic Pixel Spacing Calibration Macro).

   \sa DICOMImageBlockDescriptor::GetPixelSpacingInterpretation
 */
 typedef enum
  {
    SpacingInPatient,  ///< distances are mm within a patient
    SpacingAtDetector, ///< distances are mm at detector surface
    SpacingUnknown     ///< NO spacing information is present, we use (1,1) as default
  } PixelSpacingInterpretation;

  /**
   \ingroup DICOMModule
    \brief Describes how well the reader is tested for a certain file type (see mitk::DICOMFileReader).

    Applications should not rely on the outcome for images which are reported
    Implemented or Unsupported.

    Errors to load images which are reported as Supported
    are considered bugs. For PartlySupported please check the reader documentation on specifics.

    \sa DICOMFileReader, DICOMImageBlockDescriptor::GetReaderImplementationLevel
  */
  typedef enum
  {
    SOPClassSupported,       ///< loader code and tests are established
    SOPClassPartlySupported, ///< loader code and tests are established for specific parts of a SOP Class
    SOPClassImplemented,     ///< loader code is implemented but not accompanied by tests
    SOPClassUnsupported,     ///< loader code is not known to work with this SOP Class
    SOPClassUnknown,         ///< loader did not yet inspect any images, unknown fitness
  } ReaderImplementationLevel;


  /**
   * \brief Convert a PixelSpacingInterpretation enum value to a human readable string.
   * \param[in] value The enum value to convert.
   * \return A string representation such as "In Patient", "At Detector", or "Unknown".
   */
  std::string PixelSpacingInterpretationToString(const PixelSpacingInterpretation& value);

  /**
   * \brief Convert a ReaderImplementationLevel enum value to a human readable string.
   * \param[in] enumValue The enum value to convert.
   * \return A string representation such as "Supported", "PartlySupported", "Implemented", "Unsupported", or "Unknown".
   */
  std::string ReaderImplementationLevelToString( const ReaderImplementationLevel& enumValue );
}

#endif
