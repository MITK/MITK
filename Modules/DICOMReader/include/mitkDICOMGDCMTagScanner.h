/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkDICOMGDCMTagScanner_h
#define mitkDICOMGDCMTagScanner_h

#include "mitkDICOMTagScanner.h"
#include "mitkDICOMEnums.h"
#include "mitkDICOMGDCMTagCache.h"

namespace mitk
{

  /**
    \ingroup DICOMReaderModule
    \brief Encapsulates the tag scanning process for a set of DICOM files.

    Formerly integrated as a part of DICOMITKSeriesGDCMReader, the tag
    scanning part has been factored out into this DICOMGDCMTagScanner class
    in order to allow a single scan for multiple reader alternatives. This
    helps much in the selection process of e.g. DICOMFileReaderSelector.

    The class works similar to gdcm::Scanner, just with the MITK set of classes:
     - add a number of DICOM tags that should be read
     - set a list of files that should be scanned for named tags
     - call Scan()
     - retrieve the scan results
       - via GetFrameInfoList() or
       - via GetTagValue()

    When used in a process where multiple classes will access the scan
    results, care should be taken that all the tags and files of interest
    are communicated to DICOMGDCMTagScanner before requesting the results!

    @remark This scanner does only support the scanning for simple value tag.
    If you need to scann for sequence items or non-top-level elements, this scanner
    will not be sufficient. See i.a. DICOMDCMTKTagScanner for these cases.
  */
  class MITKDICOMREADER_EXPORT DICOMGDCMTagScanner : public DICOMTagScanner
  {
    public:

      mitkClassMacro(DICOMGDCMTagScanner, DICOMTagScanner);
      itkFactorylessNewMacro( DICOMGDCMTagScanner );
      itkCloneMacro(Self);

      /**
        \brief Add this tag to the scanning process.
      */
      void AddTag(const DICOMTag& tag) override;
      /**
        \brief Add a list of tags to the scanning process.
      */
      void AddTags(const DICOMTagList& tags) override;
      /**
      \brief Add this tag path to the scanning process.
      */
      void AddTagPath(const DICOMTagPath& tag) override;
      /**
      \brief Add a list of tag pathes to the scanning process.
      */
      void AddTagPaths(const DICOMTagPathList& tags) override;

      /**
        \brief Define the list of files to scan.
        This does not ADD to an internal list, but it replaces the
        whole list of files.
      */
      void SetInputFiles(const StringList& filenames) override;

      /**
        \brief Start the scanning process.
        Calling Scan() will invalidate previous scans, forgetting
        all about files and tags from files that have been scanned
        previously.
      */
      void Scan() override;

      /**
        \brief Retrieve a result list for file-by-file tag access.
      */
      DICOMDatasetAccessingImageFrameList GetFrameInfoList() const override;

      /**
      \brief Retrieve Pointer to the complete cache of the scan.
      */
      DICOMTagCache::Pointer GetScanCache() const override;

      /**
        \brief Directly retrieve the tag value for a given frame and tag.
        @pre Scan() must have been called before calling this function.
      */
      virtual DICOMDatasetFinding GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const;

    protected:

      DICOMGDCMTagScanner();
      ~DICOMGDCMTagScanner() override;

      std::set<DICOMTag> m_ScannedTags;
      StringList m_InputFilenames;
      DICOMGDCMTagCache::Pointer m_Cache;
      std::shared_ptr<gdcm::Scanner> m_GDCMScanner;

    private:
      DICOMGDCMTagScanner(const DICOMGDCMTagScanner&);
  };
}

#endif
