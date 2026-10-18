/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkHdfIO_h
#define mitkSurfaceVtkHdfIO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for surfaces in the VTKHDF format (.vtkhdf).
   *
   * VTKHDF holds all time steps of a surface in a single file. The geometry of
   * each time step is baked into its points, like in the other surface formats.
   * VTKHDF stores a single time value per time step, in single precision, so
   * the exact time bounds of each time step are stored alongside as field data.
   * They are only used on reading if they still agree with the time values of
   * the file at that precision. Time steps that do not start at increasing
   * time points are written as numbered time steps. A surface
   * that is valid at every time point, like a 3D image, is written without
   * time steps, and a file without time steps is read as such a surface.
   */
  class SurfaceVtkHdfIO : public AbstractFileIO
  {
  public:
    SurfaceVtkHdfIO();

    using AbstractFileReader::Read;

    /** \brief Supported for VTKHDF files that hold poly data. */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    void Write() override;

  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SurfaceVtkHdfIO *IOClone() const override;
  };
}

#endif
