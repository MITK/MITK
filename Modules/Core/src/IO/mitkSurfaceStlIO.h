/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceStlIO_h
#define mitkSurfaceStlIO_h

#include <mitkSurfaceVtkIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for STL (Stereolithography) surface files.
   *
   * Reads and writes mitk::Surface data in STL format. On reading, the points
   * shared by triangles are merged and point normals are generated. On
   * writing, the surface is triangulated before being saved.
   *
   * \sa SurfaceVtkIO
   * \sa SurfaceVtkLegacyIO
   * \sa SurfaceVtkXmlIO
   */
  class SurfaceStlIO : public mitk::SurfaceVtkIO
  {
  public:
    /** \brief Construct and register the STL I/O service. */
    SurfaceStlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

  protected:
    vtkSmartPointer<vtkPolyData> ReadPolyData(const std::string &fileName) override;
    void WritePolyData(vtkPolyData *polyData, const std::string &fileName) override;

  private:
    SurfaceStlIO *IOClone() const override;
  };
}

#endif
