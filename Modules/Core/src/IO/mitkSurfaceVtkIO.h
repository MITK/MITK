/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceVtkIO_h
#define mitkSurfaceVtkIO_h

#include <mitkAbstractFileIO.h>

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \internal
   * \brief Base class for VTK-based surface readers and writers.
   *
   * Provides common functionality for reading and writing mitk::Surface data
   * using VTK. If the surface contains multiple time steps, each time step is
   * written to a separate file with time bounds encoded in the filename:
   * \c &lt;filename&gt;_S&lt;start&gt;E&lt;end&gt;_T&lt;timestep&gt;
   *
   * Subclasses implement the format-specific reading (DoRead) and writing (Write).
   *
   * \sa AbstractFileIO
   * \sa SurfaceStlIO
   * \sa SurfaceVtkLegacyIO
   * \sa SurfaceVtkXmlIO
   */
  class SurfaceVtkIO : public mitk::AbstractFileIO
  {
  public:
    /**
     * \brief Construct with data type, MIME type, and description.
     *
     * \param[in] baseDataType The class name of the supported BaseData (typically "Surface").
     * \param[in] mimeType The MIME type this I/O handles.
     * \param[in] description A human-readable description.
     */
    SurfaceVtkIO(const std::string &baseDataType, const CustomMimeType &mimeType, const std::string &description);

    /**
     * \brief Determine writer confidence level.
     *
     * Returns PartiallySupported for multi-timestep surfaces (since multiple
     * files are written or only the first timestep for streams), Supported otherwise.
     *
     * \return The writer confidence level.
     */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
     * \brief Get transformed poly data for a given time step.
     *
     * Applies the geometry transform to the surface's vtkPolyData and computes
     * the output filename (including time bounds if applicable).
     *
     * \param[in] t The time step index.
     * \param[out] fileName The computed output filename for this time step.
     * \return The transformed vtkPolyData, or nullptr if the surface is empty at time step t.
     */
    vtkSmartPointer<vtkPolyData> GetPolyData(unsigned int t, std::string &fileName);
  };
}

#endif
