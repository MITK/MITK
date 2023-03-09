/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkXmlIO_h
#define mitkSurfaceVtkXmlIO_h

#include "mitkSurfaceVtkIO.h"

#include "mitkBaseData.h"

namespace mitk
{
  class SurfaceVtkXmlIO : public mitk::SurfaceVtkIO
  {
  public:
    SurfaceVtkXmlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SurfaceVtkXmlIO *IOClone() const override;
  };
}

#endif
