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

#include "mitkDICOMTagCache.h"
#include "mitkDICOMEnums.h"

#include "mitkDICOMGDCMImageFrameInfo.h"

#include <gdcmScanner.h>

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
    results, care should be taken that all the tags and files of interst
    are communicated to DICOMGDCMTagScanner before requesting the results!
  */
  class MitkDICOMReader_EXPORT DICOMGDCMTagScanner : public DICOMTagCache
  {
    public:

      mitkClassMacro( DICOMGDCMTagScanner, DICOMTagCache );
      itkNewMacro( DICOMGDCMTagScanner );

      /**
        \brief Add this tag to the scanning process.
      */
      virtual void AddTag(const DICOMTag& tag);
      /**
        \brief Add a list of tags to the scanning process.
      */
      virtual void AddTags(const DICOMTagList& tags);

      /**
        \brief Define the list of files to scan.
        This does not ADD to an internal list, but it replaces the
        whole list of files.
      */
      virtual void SetInputFiles(const StringList& filenames);

      /**
        \brief Start the scanning process.
        Calling Scan() will invalidate previous scans, forgetting
        all about files and tags from files that have been scanned
        previously.
      */
      virtual void Scan();

      /**
        \brief Retrieve a result list for file-by-file tag access.
      */
      virtual DICOMGDCMImageFrameList GetFrameInfoList() const;

      /**
        \brief Directly retrieve the tag value for a given frame and tag.
      */
      virtual std::string GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const;

      protected:

      DICOMGDCMTagScanner();
      DICOMGDCMTagScanner(const DICOMGDCMTagScanner&);
      virtual ~DICOMGDCMTagScanner();

      std::set<DICOMTag> m_ScannedTags;

      gdcm::Scanner m_GDCMScanner;
      StringList m_InputFilenames;
      DICOMGDCMImageFrameList m_ScanResult;
  };
}

#endif
