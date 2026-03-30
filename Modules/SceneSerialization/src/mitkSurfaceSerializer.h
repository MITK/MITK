/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceSerializer_h
#define mitkSurfaceSerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
  /**
   * \brief Serializes mitk::Surface for mitk::SceneIO.
   *
   * Writes the Surface object to a VTK XML PolyData file (.vtp) in the configured
   * working directory using mitk::IOUtil. The filename is composed from a unique
   * prefix and the filename hint.
   *
   * \sa BaseDataSerializer, SceneIO, Surface
   */
  class SurfaceSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(SurfaceSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Serialize the Surface object to a .vtp file.
     *
     * Casts the stored BaseData to mitk::Surface and writes it to disk
     * in VTK XML PolyData format.
     *
     * \return The relative filename of the created file, or an empty string on failure.
     *
     * \pre The Data must be set and must be castable to mitk::Surface.
     * \pre The WorkingDirectory must be set.
     */
    std::string Serialize() override;

  protected:
    SurfaceSerializer();
    ~SurfaceSerializer() override;
  };

} // namespace

#endif
