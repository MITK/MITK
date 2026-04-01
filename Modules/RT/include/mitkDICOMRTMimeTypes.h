/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMRTMimeTypes_h
#define mitkDICOMRTMimeTypes_h

#include <mitkCustomMimeType.h>
#include <MitkRTExports.h>

#include <array>
#include <memory>
#include <string>

namespace mitk {

/**
 * \brief Provides MIME type definitions for DICOM-RT file types (RTDose, RTStruct, RTPlan).
 *
 * This utility class registers custom MIME types for the three main DICOM Radiotherapy
 * object types: RT Dose, RT Structure Set, and RT Plan. Each MIME type checks the DICOM
 * Modality tag (0008,0060) to verify the file type. The class is non-instantiable; all
 * members are static.
 *
 * \sa mitk::CustomMimeType
 * \sa mitk::IOMimeTypes
 * \ingroup MitkRTModule
 */
class MITKRT_EXPORT DICOMRTMimeTypes
{
public:

  /**
   * \brief MIME type for DICOM RT Dose files.
   *
   * Identifies files whose DICOM Modality tag is "RTDOSE" and that can be read
   * by a DICOM file reader.
   *
   * \sa mitk::CustomMimeType
   */
  class MITKRT_EXPORT RTDoseMimeType : public CustomMimeType
  {
  public:
    /** \brief Default constructor. Sets category to "DICOMRT" and adds ".dcm" extension. */
    RTDoseMimeType();

    /**
     * \brief Checks whether the given file is an RT Dose DICOM file.
     * \param[in] path Filesystem path to the file to check.
     * \return True if the file has modality "RTDOSE" and can be read by a DICOM file reader.
     */
    bool AppliesTo(const std::string &path) const override;

    /**
     * \brief Creates a clone of this MIME type instance.
     * \return A pointer to a new RTDoseMimeType that is a copy of this instance.
     */
    RTDoseMimeType* Clone() const override;
  };

  /**
   * \brief MIME type for DICOM RT Structure Set files.
   *
   * Identifies files whose DICOM Modality tag is "RTSTRUCT".
   *
   * \sa mitk::CustomMimeType
   */
  class MITKRT_EXPORT RTStructMimeType : public CustomMimeType
  {
  public:
    /** \brief Default constructor. Sets category to "DICOMRT" and adds ".dcm" extension. */
    RTStructMimeType();

    /**
     * \brief Checks whether the given file is an RT Structure Set DICOM file.
     * \param[in] path Filesystem path to the file to check.
     * \return True if the file has modality "RTSTRUCT".
     */
    bool AppliesTo(const std::string &path) const override;

    /**
     * \brief Creates a clone of this MIME type instance.
     * \return A pointer to a new RTStructMimeType that is a copy of this instance.
     */
    RTStructMimeType* Clone() const override;
  };

  /**
   * \brief MIME type for DICOM RT Plan files.
   *
   * Identifies files whose DICOM Modality tag is "RTPLAN".
   *
   * \sa mitk::CustomMimeType
   */
  class MITKRT_EXPORT RTPlanMimeType : public CustomMimeType
  {
  public:
    /** \brief Default constructor. Sets category to "DICOMRT" and adds ".dcm" extension. */
    RTPlanMimeType();

    /**
     * \brief Checks whether the given file is an RT Plan DICOM file.
     * \param[in] path Filesystem path to the file to check.
     * \return True if the file has modality "RTPLAN".
     */
    bool AppliesTo(const std::string &path) const override;

    /**
     * \brief Creates a clone of this MIME type instance.
     * \return A pointer to a new RTPlanMimeType that is a copy of this instance.
     */
    RTPlanMimeType* Clone() const override;
  };

  /**
   * \brief Returns an array of all DICOM-RT MIME types (Dose, Plan, Struct).
   * \return An array of three unique_ptr<CustomMimeType> covering all RT modalities.
   */
  static std::array<std::unique_ptr<CustomMimeType>, 3> Get();

  /**
   * \brief Returns an RTDoseMimeType instance.
   * \return A default-constructed RTDoseMimeType.
   */
  static RTDoseMimeType DICOMRT_DOSE_MIMETYPE();

  /**
   * \brief Returns an RTStructMimeType instance.
   * \return A default-constructed RTStructMimeType.
   */
  static RTStructMimeType DICOMRT_STRUCT_MIMETYPE();

  /**
   * \brief Returns an RTPlanMimeType instance.
   * \return A default-constructed RTPlanMimeType.
   */
  static RTPlanMimeType DICOMRT_PLAN_MIMETYPE();

  /**
   * \brief Returns the registered name string for the RT Dose MIME type.
   * \return The MIME type name string (e.g. "application/vnd.mitk.dicomrt.dose").
   */
  static std::string DICOMRT_DOSE_MIMETYPE_NAME();

  /**
   * \brief Returns the registered name string for the RT Struct MIME type.
   * \return The MIME type name string (e.g. "application/vnd.mitk.dicomrt.struct").
   */
  static std::string DICOMRT_STRUCT_MIMETYPE_NAME();

  /**
   * \brief Returns the registered name string for the RT Plan MIME type.
   * \return The MIME type name string (e.g. "application/vnd.mitk.dicomrt.plan").
   */
  static std::string DICOMRT_PLAN_MIMETYPE_NAME();

  /**
   * \brief Returns a human-readable description for the RT Dose MIME type.
   * \return The description string "RTDOSE reader".
   */
  static std::string DICOMRT_DOSE_MIMETYPE_DESCRIPTION();

  /**
   * \brief Returns a human-readable description for the RT Struct MIME type.
   * \return The description string "RTSTRUCT reader".
   */
  static std::string DICOMRT_STRUCT_MIMETYPE_DESCRIPTION();

  /**
   * \brief Returns a human-readable description for the RT Plan MIME type.
   * \return The description string "RTPLAN reader".
   */
  static std::string DICOMRT_PLAN_MIMETYPE_DESCRIPTION();

  DICOMRTMimeTypes() = delete;
  DICOMRTMimeTypes(const DICOMRTMimeTypes&) = delete;

  /**
   * \brief Checks whether the given file can be read by any built-in DICOM file reader.
   * \param[in] path Filesystem path to the DICOM file.
   * \return True if a suitable DICOMFileReader is found for the file.
   */
  static bool canReadByDicomFileReader(const std::string & path);

  /**
   * \brief Reads the DICOM Modality tag (0008,0060) from the given file.
   * \param[in] path Filesystem path to the DICOM file.
   * \return The modality string (e.g. "RTDOSE", "RTSTRUCT", "RTPLAN"), or an empty string if unavailable.
   */
  static std::string GetModality(const std::string & path);
};

}

#endif
