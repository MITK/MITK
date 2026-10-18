/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfacePlyIO_h
#define mitkSurfacePlyIO_h

#include <mitkSurfaceVtkIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for Stanford PLY surface files.
   *
   * Reads binary and ASCII files and writes binary files.
   */
  class SurfacePlyIO : public SurfaceVtkIO
  {
  public:
    SurfacePlyIO();

    using AbstractFileReader::Read;

  protected:
    vtkSmartPointer<vtkPolyData> ReadPolyData(const std::string &fileName) override;
    void WritePolyData(vtkPolyData *polyData, const std::string &fileName) override;

  private:
    SurfacePlyIO *IOClone() const override;
  };
}

#endif
