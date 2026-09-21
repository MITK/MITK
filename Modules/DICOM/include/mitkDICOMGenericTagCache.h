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

#include <unordered_map>
#include <vector>

namespace mitk
{

  /**
    \ingroup DICOMModule
    \brief Generic tag cache implementation using DICOMGenericImageFrameInfo.

    This cache implementation stores tag values in a simple in-memory map structure
    via DICOMGenericImageFrameInfo objects. It is used by DICOMDCMTKTagScanner for
    caching DCMTK-based scan results.

    \remark A tag cache is used single-threaded for the duration of one read. It is
    shared between the reader and every block through DICOMFileReader::SetTagCache,
    and blocks query it while updating their properties, all on the calling thread.
    GetFrameInfo() relies on that: it is const and fills its map lazily. If a read is
    ever parallelised, this is one of the places that has to be revisited.

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
       * \return A DICOMDatasetFinding with the value if found.
       */
      DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const override;

      /**
       * \brief Retrieve tag values for a specific frame and tag path.
       * \param[in] frame The image frame to query. The passed info decides which
       *            view answers: a file-level info reports the file under literal
       *            rooted paths, a frame-scoped one reports its frame under
       *            frame-relative paths.
       * \param[in] path The DICOM tag path to retrieve.
       * \return A list of findings matching the given path.
       */
      FindingsListType GetTagValue(DICOMImageFrameInfo* frame, const DICOMTagPath& path) const override;

      /**
       * \brief Retrieve the list of frame info objects from the cache.
       * \return A list of DICOMDatasetAccessingImageFrameInfo smart pointers,
       *         one per scanned file. Frame-scoped infos are not part of it.
       */
      DICOMDatasetAccessingImageFrameList GetFrameInfoList() const override;

      DICOMFrameLayout GetFrameLayout(const DICOMImageFrameInfo* frame) const override;

      bool HasAnyFrameModel() const override;

      DICOMDatasetAccessingImageFrameInfo::Pointer GetFrameInfo(const std::string& filename,
                                                                unsigned int frameNo) const override;

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
      const DICOMGenericImageFrameInfo* FindFile(const std::string& filename) const;

      std::unordered_map<std::string, DICOMGenericImageFrameInfo*> m_ByFilename;
      bool m_HasAnyFrameModel = false;

      /** Frame-scoped infos are created on demand and kept, so that resolving the
          same frame twice yields the same object. */
      mutable std::unordered_map<std::string, std::vector<DICOMDatasetAccessingImageFrameInfo::Pointer>> m_FrameScoped;

      DICOMGenericTagCache(const DICOMGenericTagCache&);
  };
}

#endif
