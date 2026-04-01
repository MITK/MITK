/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGenericTagCache_h
#define mitkDICOMGenericTagCache_h

#include <mitkDICOMTagCache.h>
#include <mitkDICOMGenericImageFrameInfo.h>

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief Generic tag cache implementation using DICOMGenericImageFrameInfo.

    This cache implementation stores tag values in a simple in-memory map structure
    via DICOMGenericImageFrameInfo objects. It is used by DICOMDCMTKTagScanner for
    caching DCMTK-based scan results.

    \sa DICOMTagCache, DICOMDCMTKTagScanner, DICOMGenericImageFrameInfo
  */
  class MITKDICOM_EXPORT DICOMGenericTagCache : public DICOMTagCache
  {
    public:

      mitkClassMacro(DICOMGenericTagCache, DICOMTagCache);
      itkFactorylessNewMacro( DICOMGenericTagCache );
      itkCloneMacro(Self);

      /**
       * \brief Retrieve a tag value for a specific frame and tag.
       * \param[in] frame The image frame to query.
       * \param[in] tag The DICOM tag to retrieve.
       * \return A DICOMDatasetFinding with the tag value if found.
       */
      DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const override;

      /**
       * \brief Retrieve tag values for a specific frame and tag path.
       * \param[in] frame The image frame to query.
       * \param[in] path The DICOM tag path to retrieve.
       * \return A list of findings matching the given path.
       */
      FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const override;

      /**
       * \brief Retrieve the list of frame info objects from the cache.
       * \return A list of DICOMDatasetAccessingImageFrameInfo smart pointers.
       */
      DICOMDatasetAccessingImageFrameList GetFrameInfoList() const override;

      /**
       * \brief Add a frame info object to the cache.
       * \param[in] info The DICOMDatasetAccessingImageFrameInfo to add.
       */
      void AddFrameInfo(DICOMDatasetAccessingImageFrameInfo* info);

      /**
       * \brief Clear all cached frame info objects.
       */
      void Reset();

  protected:

      DICOMGenericTagCache();
      ~DICOMGenericTagCache() override;

      DICOMDatasetAccessingImageFrameList m_ScanResult;

    private:
      DICOMGenericTagCache(const DICOMGenericTagCache&);
  };
}

#endif
