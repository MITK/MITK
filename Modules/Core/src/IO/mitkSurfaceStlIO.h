/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceStlIO_h
#define mitkSurfaceStlIO_h

#include "mitkSurfaceVtkIO.h"

namespace mitk
{
  class SurfaceStlIO : public mitk::SurfaceVtkIO
  {
  public:
    SurfaceStlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    // -------------- AbstractFileWriter -------------

    void Write() override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SurfaceStlIO *IOClone() const override;

    // vtkSTLReader crashes with this option
    // static std::string OPTION_MERGE_POINTS();

    static std::string OPTION_TAG_SOLIDS();
    static std::string OPTION_CLEAN();
  };
}

#endif
