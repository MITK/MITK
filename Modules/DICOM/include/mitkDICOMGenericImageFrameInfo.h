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
#include <mitkDICOMFrameLayout.h>

#include <memory>

namespace mitk
{
  /**
    \ingroup DICOMModule
    \brief A generic storage class for image frame info with data access.

    This class stores DICOM tag values in an internal map structure and provides
    access via the DICOMDatasetAccess interface. It is used by DICOMGenericTagCache
    and DICOMDCMTKTagScanner as a flexible, library-independent frame info container.

    Two views on one file exist, and they answer differently on purpose:

    - A **file-level** info, the kind GetFrameInfoList() returns, reports every
      finding of the file under the literal path it was found at, including the
      functional-group root.
    - A **frame-scoped** info, created by NewFrameScoped() for frame \c FrameNo
      of a file with a frame model, presents that frame's dataset as DICOM
      defines it: the top-level attributes, the macros of the shared item, and
      the macros of per-frame item \c FrameNo, with every functional-group
      finding reported under its path relative to the functional-group item.
      Other frames' items are omitted.

    A query is always matched against the stored path; only the reported path is
    frame-relative.

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

      /** \brief The values of one file, shared by its file-level info and its frame-scoped infos. */
      class ValueStore;
      using ValueStorePointer = std::shared_ptr<ValueStore>;

      ~DICOMGenericImageFrameInfo() override;

      /**
       * \brief Create the frame-scoped view of \p frameNo on an existing file's values.
       * \param[in] filename The file the frame belongs to.
       * \param[in] frameNo The stored frame index, which is also the per-frame item index.
       * \param[in] store The value store of the file, shared rather than copied.
       */
      static Pointer NewFrameScoped(const std::string& filename, unsigned int frameNo, ValueStorePointer store);

      /**
       * \brief Retrieve a tag value as a string for a single DICOM tag.
       * \param[in] tag The DICOM tag to query.
       * \return A DICOMDatasetFinding with the value if found.
       */
      DICOMDatasetFinding GetTagValueAsString(const DICOMTag& tag) const override;

      /**
       * \brief Retrieve tag values as strings for a DICOM tag path.
       * \param[in] path The tag path to query, matched against the stored paths.
       * \return A list of findings matching the path. For a frame-scoped info,
       *         only this frame's findings, reported under their frame-relative path.
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

      /** \brief The layout of the file this info belongs to. */
      const DICOMFrameLayout& GetFrameLayout() const;

      /** \brief Record the file's layout. Called by the scanner that parsed it. */
      void SetFrameLayout(const DICOMFrameLayout& layout);

      /** \brief The file's value store, for sharing it with the file's frame-scoped infos. */
      ValueStorePointer GetStore() const;

    protected:
      ValueStorePointer m_Store;

      explicit DICOMGenericImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo);
      DICOMGenericImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);

    private:
      Self& operator = (const Self&);
      DICOMGenericImageFrameInfo(const Self&);

      bool m_FrameScoped = false;
  };

}

#endif
