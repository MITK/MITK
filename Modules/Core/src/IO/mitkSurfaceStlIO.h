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
  /**
   * \brief Reader and writer for STL (Stereolithography) surface files.
   *
   * Reads and writes mitk::Surface data in STL format. On reading, supports
   * optional tagging of solids and cleaning of the poly data (removing
   * degenerate cells). Normals are always generated. On writing, the surface
   * is triangulated before being saved.
   *
   * \sa SurfaceVtkIO
   * \sa SurfaceVtkLegacyIO
   * \sa SurfaceVtkXmlIO
   */
  class SurfaceStlIO : public mitk::SurfaceVtkIO
  {
  public:
    /** \brief Construct and register the STL I/O service with default options. */
    SurfaceStlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    // -------------- AbstractFileWriter -------------

    /**
     * \brief Write a mitk::Surface to STL format.
     *
     * Triangulates the surface before writing. For multi-timestep data writing
     * to streams, only the first timestep is written.
     *
     * \throw mitk::Exception on write failure.
     */
    void Write() override;

  protected:
    /**
     * \brief Read an STL file into a mitk::Surface.
     *
     * Generates normals and optionally cleans the poly data based on
     * reader options.
     *
     * \return A vector containing the read Surface.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    SurfaceStlIO *IOClone() const override;

    static std::string OPTION_TAG_SOLIDS();  ///< Reader option key for tagging solids.
    static std::string OPTION_CLEAN();       ///< Reader option key for cleaning poly data.
  };
}

#endif
