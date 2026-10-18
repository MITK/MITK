/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkXmlIO_h
#define mitkSurfaceVtkXmlIO_h

#include <mitkSurfaceVtkIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for VTK XML poly data surface format (.vtp).
   *
   * \sa SurfaceVtkIO
   * \sa SurfaceVtkLegacyIO
   * \sa SurfaceStlIO
   */
  class SurfaceVtkXmlIO : public mitk::SurfaceVtkIO
  {
  public:
    /** \brief Construct and register the VTK XML surface I/O service. */
    SurfaceVtkXmlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /**
     * \brief Determine reader confidence for the current input.
     *
     * For file-based input, checks whether vtkXMLPolyDataReader can read the file.
     * For stream-based input, returns Supported unconditionally.
     *
     * \return The reader confidence level.
     */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

  protected:
    vtkSmartPointer<vtkPolyData> ReadPolyData(const std::string &fileName) override;
    void WritePolyData(vtkPolyData *polyData, const std::string &fileName) override;

  private:
    SurfaceVtkXmlIO *IOClone() const override;
  };
}

#endif
