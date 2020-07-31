/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGenericImageFrameInfo_h
#define mitkDICOMGenericImageFrameInfo_h

#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

#include <map>

namespace mitk
{
  /**
    \ingroup DICOMModule
    \brief A generic storage class for image frame info with data access.
  */
  class MITKDICOM_EXPORT DICOMGenericImageFrameInfo : public DICOMDatasetAccessingImageFrameInfo
  {
    public:

      mitkClassMacro(DICOMGenericImageFrameInfo, DICOMDatasetAccessingImageFrameInfo);
      itkFactorylessNewMacro( DICOMGenericImageFrameInfo );
      mitkNewMacro1Param( DICOMGenericImageFrameInfo, const std::string&);
      mitkNewMacro2Param( DICOMGenericImageFrameInfo, const std::string&, unsigned int );
      mitkNewMacro1Param( DICOMGenericImageFrameInfo, const DICOMImageFrameInfo::Pointer& );

      ~DICOMGenericImageFrameInfo() override;

      DICOMDatasetFinding GetTagValueAsString(const DICOMTag&) const override;

      FindingsListType GetTagValueAsString(const DICOMTagPath& path) const override;

      std::string GetFilenameIfAvailable() const override;

      /** Sets the value for a passed tag path. If the tag path is already set, it will be overwritten
       with the new value.
      @pre Path must be explicit. No wildcards are allowed.
      @post The passed value is set for the passed path.
      */
      void SetTagValue(const DICOMTagPath& path, const std::string& value);

    protected:
      typedef std::map<DICOMTagPath, std::string> ValueMapType;
      ValueMapType m_Values;

      explicit DICOMGenericImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo);
      DICOMGenericImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);

    private:
      Self& operator = (const Self&);
      DICOMGenericImageFrameInfo(const Self&);
  };

}

#endif
