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

#ifndef mitkDICOMTagCache_h
#define mitkDICOMTagCache_h

#include "itkObjectFactory.h"
#include "mitkCommon.h"

#include "mitkDICOMTag.h"

#include "MitkDICOMReaderExports.h"

namespace mitk
{

  class DICOMImageFrameInfo;

  /**
    \ingroup DICOMReaderModule
    \brief ...
  */
  class MitkDICOMReader_EXPORT DICOMTagCache : public itk::Object
  {
    public:

      mitkClassMacro( DICOMTagCache, itk::Object );

      virtual std::string GetTagValue(DICOMImageFrameInfo* frame, const DICOMTag& tag) const = 0;

      protected:

      DICOMTagCache();
      DICOMTagCache(const DICOMTagCache&);
      virtual ~DICOMTagCache();
  };
}

#endif
