/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_SURFACE_VTK_LEGACY_IO_H_
#define _MITK_SURFACE_VTK_LEGACY_IO_H_

#include "mitkSurfaceVtkIO.h"

#include "mitkBaseData.h"

namespace mitk
{
  class SurfaceVtkLegacyIO : public mitk::SurfaceVtkIO
  {
  public:
    SurfaceVtkLegacyIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SurfaceVtkLegacyIO *IOClone() const override;
  };
}

#endif //_MITK_SURFACE_VTK_LEGACY_IO_H_
