/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGenericImageFrameInfo_h
#define mitkDICOMGenericImageFrameInfo_h

#include <mitkDICOMDatasetAccessingImageFrameInfo.h>

#include <map>

namespace mitk
{
  /**
    \ingroup DICOMModule
    \brief A generic storage class for image frame info with data access.

    This class stores DICOM tag values in an internal map structure and provides
    access via the DICOMDatasetAccess interface. It is used by DICOMGenericTagCache
    and DICOMDCMTKTagScanner as a flexible, library-independent frame info container.

    \sa DICOMDatasetAccessingImageFrameInfo, DICOMGenericTagCache, DICOMGDCMImageFrameInfo
  */
  class MITKDICOM_EXPORT DICOMGenericImageFrameInfo : public DICOMDatasetAccessingImageFrameInfo
  {
    public:

      mitkClassMacro(DICOMGenericImageFrameInfo, DICOMDatasetAccessingImageFrameInfo);
      itkFactorylessNewMacro( DICOMGenericImageFrameInfo );
      mitkNewMacro1Param( DICOMGenericImageFrameInfo, const std::string&);
      mitkNewMacro2Param( DICOMGenericImageFrameInfo, const std::string&, unsigned int );
      mitkNewMacro1Param( DICOMGenericImageFrameInfo, const DICOMImageFrameInfo::Pointer& );

      ~DICOMGenericImageFrameInfo() override;

      /**
       * \brief Retrieve a tag value as a string for a single DICOM tag.
       * \param[in] tag The DICOM tag to query.
       * \return A DICOMDatasetFinding with the value if found.
       */
      DICOMDatasetFinding GetTagValueAsString(const DICOMTag& tag) const override;

      /**
       * \brief Retrieve tag values as strings for a DICOM tag path.
       * \param[in] path The tag path to query.
       * \return A list of findings matching the path.
       */
      FindingsListType GetTagValueAsString(const DICOMTagPath& path) const override;

      /**
       * \brief Return the filename of this frame if available.
       * \return The filename or an empty string.
       */
      std::string GetFilenameIfAvailable() const override;

      /**
       * \brief Set the value for a given tag path.
       *
       * If the tag path is already set, it will be overwritten with the new value.
       *
       * \param[in] path The tag path to set. Must be explicit (no wildcards).
       * \param[in] value The string value to store.
       * \pre Path must be explicit. No wildcards are allowed.
       * \post The passed value is set for the passed path.
       */
      void SetTagValue(const DICOMTagPath& path, const std::string& value);

    protected:
      typedef std::map<DICOMTagPath, std::string> ValueMapType;
      ValueMapType m_Values;

      explicit DICOMGenericImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo);
      DICOMGenericImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);

    private:
      Self& operator = (const Self&);
      DICOMGenericImageFrameInfo(const Self&);
  };

}

#endif
