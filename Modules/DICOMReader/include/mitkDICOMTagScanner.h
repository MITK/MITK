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

#ifndef mitkDICOMTagScanner_h
#define mitkDICOMTagScanner_h

#include <stack>
#include "itkMutexLock.h"

#include "mitkDICOMEnums.h"
#include "mitkDICOMTagPath.h"
#include "mitkDICOMTagCache.h"
#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

namespace mitk
{

  /**
    \ingroup DICOMReaderModule
    \brief Abstracts the tag scanning process for a set of DICOM files.

    Formerly integrated as a part of DICOMITKSeriesGDCMReader, the tag
    scanning part has been factored out into DICOMTagScanner classes
    in order to allow a single scan for multiple reader alternatives. This
    helps much in the selection process of e.g. DICOMFileReaderSelector.

    This is an abstract base class for concrete scanner implementations.

    @remark When used in a process where multiple classes will access the scan
    results, care should be taken that all the tags and files of interest
    are communicated to DICOMTagScanner before requesting the results!
  */
  class MITKDICOMREADER_EXPORT DICOMTagScanner : public itk::Object
  {
    public:
      mitkClassMacroItkParent(DICOMTagScanner, itk::Object);

      /**
        \brief Add this tag to the scanning process.
      */
      virtual void AddTag(const DICOMTag& tag) = 0;
      /**
        \brief Add a list of tags to the scanning process.
      */
      virtual void AddTags(const DICOMTagList& tags) = 0;
      /**
      \brief Add this tag path to the scanning process.
      */
      virtual void AddTagPath(const DICOMTagPath& path) = 0;
      /**
      \brief Add a list of tag pathes to the scanning process.
      */
      virtual void AddTagPaths(const DICOMTagPathList& paths) = 0;

      /**
        \brief Define the list of files to scan.
        This does not ADD to an internal list, but it replaces the
        whole list of files.
      */
      virtual void SetInputFiles(const StringList& filenames) = 0;

      /**
        \brief Start the scanning process.
        Calling Scan() will invalidate previous scans, forgetting
        all about files and tags from files that have been scanned
        previously.
      */
      virtual void Scan() = 0;

      /**
        \brief Retrieve a result list for file-by-file tag access.
      */
      virtual DICOMDatasetAccessingImageFrameList GetFrameInfoList() const = 0;

      /**
      \brief Retrieve Pointer to the complete cache of the scan.
      */
      virtual DICOMTagCache::Pointer GetScanCache() const = 0;

    protected:

      /** \brief Return active C locale */
      static std::string GetActiveLocale();
      /**
      \brief Remember current locale on stack, activate "C" locale.
      "C" locale is required for correct parsing of numbers by itk::ImageSeriesReader
      */
      void PushLocale() const;
      /**
      \brief Activate last remembered locale from locale stack
      "C" locale is required for correct parsing of numbers by itk::ImageSeriesReader
      */
      void PopLocale() const;

      DICOMTagScanner();
      ~DICOMTagScanner() override;

    private:

      static itk::MutexLock::Pointer s_LocaleMutex;

      mutable std::stack<std::string> m_ReplacedCLocales;
      mutable std::stack<std::locale> m_ReplacedCinLocales;

      DICOMTagScanner(const DICOMTagScanner&);
  };
}

#endif
