/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGDCMTagCache_h
#define mitkDICOMGDCMTagCache_h

#include <mitkDICOMTagCache.h>

#include <set>
#include <memory>

#include <gdcmScanner.h>

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief Tag cache implementation used by the DICOMGDCMTagScanner.
  */
  class MITKDICOM_EXPORT DICOMGDCMTagCache : public DICOMTagCache
  {
    public:

      mitkClassMacro(DICOMGDCMTagCache, DICOMTagCache);
      itkFactorylessNewMacro( DICOMGDCMTagCache );
      itkCloneMacro(Self);

      /**
       * \brief Retrieve a tag value for a specific frame and tag.
       * \param[in] frame The image frame to query.
       * \param[in] tag The DICOM tag to retrieve.
       * \return A DICOMDatasetFinding with the tag value if found.
       * \throw std::invalid_argument if the frame or tag was not part of the scan.
       */
      DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const override;

      /**
       * \brief Retrieve tag values for a specific frame and tag path.
       *
       * Only supports explicit single-element paths (direct tag lookups).
       *
       * \param[in] frame The image frame to query.
       * \param[in] path The DICOM tag path to retrieve.
       * \return A list of findings matching the given path.
       */
      FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const override;

      /**
       * \brief Retrieve the list of frame info objects from the scan result.
       * \return A list of DICOMDatasetAccessingImageFrameInfo smart pointers.
       */
      DICOMDatasetAccessingImageFrameList GetFrameInfoList() const override;

      /**
       * \brief Initialize the cache with scan results from a gdcm::Scanner.
       * \param[in] scannedTags The set of tags that were scanned.
       * \param[in] scanner Shared pointer to the gdcm::Scanner containing the results.
       * \param[in] inputFiles The list of input file paths that were scanned.
       */
      void InitCache(const std::set<DICOMTag>& scannedTags, const std::shared_ptr<gdcm::Scanner>& scanner, const StringList& inputFiles);

      /**
       * \brief Get a const reference to the underlying gdcm::Scanner.
       * \return Const reference to the GDCM scanner.
       */
      const gdcm::Scanner& GetScanner() const;

  protected:

      DICOMGDCMTagCache();
      ~DICOMGDCMTagCache() override;

      std::set<DICOMTag> m_ScannedTags;

      std::shared_ptr<gdcm::Scanner> m_Scanner;

      DICOMDatasetAccessingImageFrameList m_ScanResult;

    private:
      DICOMGDCMTagCache(const DICOMGDCMTagCache&);
  };
}

#endif
