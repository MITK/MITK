/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGeometryDataSerializer_h
#define mitkGeometryDataSerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
  /**
   * \brief Serializes mitk::GeometryData for mitk::SceneIO.
   *
   * Writes the GeometryData object to a ".mitkgeometry" file in the configured
   * working directory using mitk::IOUtil. The filename is composed from
   * a unique prefix and the filename hint.
   *
   * \warning Depends on mitk::GeometryDataWriterService which is currently implemented
   *          only for the Geometry3D class. Check the status of that class before using
   *          other geometry types.
   *
   * \sa BaseDataSerializer, SceneIO, GeometryData
   */
  class GeometryDataSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(GeometryDataSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)

    /**
     * \brief Serialize the GeometryData object to a .mitkgeometry file.
     *
     * Casts the stored BaseData to GeometryData and writes it to disk.
     *
     * \return The relative filename of the created file, or an empty string on failure.
     *
     * \pre The Data must be set and must be castable to mitk::GeometryData.
     * \pre The WorkingDirectory must be set.
     */
    std::string Serialize() override;

  protected:
    GeometryDataSerializer();
    ~GeometryDataSerializer() override;
  };

} // namespace
#endif
