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

#ifndef mitkDICOMDCMTKTagScanner_h
#define mitkDICOMDCMTKTagScanner_h

#include <set>

#include "mitkDICOMTagScanner.h"
#include "mitkDICOMEnums.h"
#include "mitkDICOMGenericTagCache.h"

namespace mitk
{

  /**
    \ingroup DICOMReaderModule
    \brief Encapsulates the tag scanning process for a set of DICOM files.

    For the scanning process it uses DCMTK functionality.
  */
  class MITKDICOMREADER_EXPORT DICOMDCMTKTagScanner : public DICOMTagScanner
  {
    public:

      mitkClassMacro(DICOMDCMTKTagScanner, DICOMTagScanner);
      itkFactorylessNewMacro( DICOMDCMTKTagScanner );
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

    protected:

      DICOMDCMTKTagScanner();
      ~DICOMDCMTKTagScanner() override;

      std::set<DICOMTagPath> m_ScannedTags;
      StringList m_InputFilenames;
      DICOMGenericTagCache::Pointer m_Cache;

    private:
      DICOMDCMTKTagScanner(const DICOMDCMTKTagScanner&);
  };
}

#endif
