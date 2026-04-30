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

#include <mitkBaseData.h>

namespace mitk
{
  /**
   * \brief Reader and writer for VTK XML poly data surface format (.vtp).
   *
   * Handles reading and writing of mitk::Surface data using VTK's XML poly
   * data format. Supports both file-based and stream-based I/O.
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

    // -------------- AbstractFileWriter -------------

    /**
     * \brief Write a mitk::Surface in VTK XML poly data format.
     *
     * Supports writing to both files and output streams. For multi-timestep
     * data with streams, only the first timestep is written.
     *
     * \throw mitk::Exception on write failure or if the surface is empty.
     */
    void Write() override;

  protected:
    /**
     * \brief Read VTK XML poly data into a mitk::Surface.
     *
     * Supports reading from both files and input streams.
     *
     * \return A vector containing the read Surface.
     * \throw mitk::Exception on read failure.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SurfaceVtkXmlIO *IOClone() const override;
  };
}

#endif
