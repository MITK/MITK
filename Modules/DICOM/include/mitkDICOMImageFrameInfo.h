/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMImageFrameInfo_h
#define mitkDICOMImageFrameInfo_h

#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include <MitkDICOMExports.h>

namespace mitk
{
  /**
    \ingroup DICOMModule
    \brief Describes a frame within a DICOM file.

    This is a minimal data structure to describe a single frame
    (think of DICOM multi-frame classes) within a DICOM file.
    To be used by DICOMFileReader%s and as part of DICOMImageBlockDescriptor.
  */
  class MITKDICOM_EXPORT DICOMImageFrameInfo : public itk::LightObject
  {
    public:

      const std::string Filename;    ///< Absolute filename of the DICOM file.
      const unsigned int FrameNo;    ///< Frame number within the file, starting with 0.

      mitkClassMacroItkParent( DICOMImageFrameInfo, itk::LightObject );

      itkFactorylessNewMacro( DICOMImageFrameInfo );
      mitkNewMacro1Param( DICOMImageFrameInfo, const std::string&);
      mitkNewMacro2Param( DICOMImageFrameInfo, const std::string&, unsigned int );

      /**
       * \brief Equality comparison based on filename and frame number.
       * \param[in] other The frame info to compare with.
       * \return true if filename and frame number match.
       */
      bool operator==(const DICOMImageFrameInfo& other) const;

    protected:

      DICOMImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);
  };

  typedef std::vector<DICOMImageFrameInfo::Pointer> DICOMImageFrameList;
}

#endif
