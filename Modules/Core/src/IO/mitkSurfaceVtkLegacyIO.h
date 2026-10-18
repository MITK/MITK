/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkLegacyIO_h
#define mitkSurfaceVtkLegacyIO_h

#include <mitkSurfaceVtkIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for VTK legacy poly data surface format.
   *
   * Handles reading and writing of mitk::Surface data using VTK's legacy
   * poly data file format (.vtk). Supports an option to save in binary format.
   *
   * \sa SurfaceVtkIO
   * \sa SurfaceVtkXmlIO
   * \sa SurfaceStlIO
   */
  class SurfaceVtkLegacyIO : public mitk::SurfaceVtkIO
  {
  public:
    /** \brief Construct and register the VTK legacy surface I/O service. */
    SurfaceVtkLegacyIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /**
     * \brief Determine reader confidence for the current input.
     *
     * Returns Supported if the file is valid VTK poly data with the default
     * header "vtk output", PartiallySupported for other valid poly data files.
     *
     * \return The reader confidence level.
     */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

  protected:
    vtkSmartPointer<vtkPolyData> ReadPolyData(const std::string &fileName) override;
    void WritePolyData(vtkPolyData *polyData, const std::string &fileName) override;

  private:
    SurfaceVtkLegacyIO *IOClone() const override;
  };
}

#endif
