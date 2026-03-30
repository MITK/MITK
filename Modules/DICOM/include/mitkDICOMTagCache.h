/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMTagCache_h
#define mitkDICOMTagCache_h

#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include <mitkDICOMTag.h>
#include <mitkDICOMDatasetAccess.h>
#include <mitkDICOMEnums.h>

#include <mitkDICOMDatasetAccessingImageFrameInfo.h>
#include <MitkDICOMExports.h>

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief Abstract base class for caching DICOM tag values scanned from files.

    A DICOMTagCache stores the results of a tag scanning process and provides
    fast access to tag values for individual frames. Concrete implementations
    include DICOMGDCMTagCache (GDCM-based) and DICOMGenericTagCache (generic storage).

    \sa DICOMTagScanner, DICOMGDCMTagCache, DICOMGenericTagCache
  */
  class MITKDICOM_EXPORT DICOMTagCache : public itk::Object
  {
    public:
      mitkClassMacroItkParent( DICOMTagCache, itk::Object );

      /** \brief A list of DICOMDatasetFinding instances. */
      typedef std::list<DICOMDatasetFinding> FindingsListType;

      /**
       * \brief Define the list of files that were scanned to populate the cache.
       * \param[in] filenames The list of absolute file paths.
       */
      virtual void SetInputFiles(const StringList& filenames);

      /**
       * \brief Retrieve a tag value for a specific frame and tag.
       * \param[in] frame The image frame to query.
       * \param[in] tag The DICOM tag to retrieve.
       * \return A DICOMDatasetFinding with the tag value if found.
       */
      virtual DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const = 0;

      /**
       * \brief Retrieve tag values for a specific frame and tag path.
       * \param[in] frame The image frame to query.
       * \param[in] path The DICOM tag path to retrieve.
       * \return A list of findings matching the given path.
       */
      virtual FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const = 0;

      /**
       * \brief Retrieve a result list for file-by-file tag access.
       * \return A list of DICOMDatasetAccessingImageFrameInfo instances, one per scanned file.
       */
      virtual DICOMDatasetAccessingImageFrameList GetFrameInfoList() const = 0;

    protected:

      StringList m_InputFilenames;
      DICOMTagCache();
      DICOMTagCache(const DICOMTagCache&);
      ~DICOMTagCache() override;
  };
}

#endif
