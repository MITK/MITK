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

#ifndef mitkDICOMImageFrameInfo_h
#define mitkDICOMImageFrameInfo_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "MitkDICOMReaderExports.h"

namespace mitk
{
  /**
    \ingroup DICOMReaderModule
    \brief Describes a frame within a DICOM file.

    This is a minimal data structure to describe a single frame
    (think of DICOM multi-frame classes) within a DICOM file.
    To be used by DICOMFileReader%s and as part of DICOMImageBlockDescriptor.
  */
  class MitkDICOMReader_EXPORT DICOMImageFrameInfo : public itk::LightObject
  {
    public:

      /// absolute filename
      std::string Filename;
      /// frame number, starting with 0
      unsigned int FrameNo;

      mitkClassMacro( DICOMImageFrameInfo, itk::LightObject )

      itkNewMacro( DICOMImageFrameInfo );
      mitkNewMacro1Param( DICOMImageFrameInfo, const std::string&);
      mitkNewMacro2Param( DICOMImageFrameInfo, const std::string&, unsigned int );

      bool operator==(const DICOMImageFrameInfo& other) const;

    protected:

      DICOMImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);
  };

  typedef std::vector<DICOMImageFrameInfo::Pointer> DICOMImageFrameList;
}

#endif
