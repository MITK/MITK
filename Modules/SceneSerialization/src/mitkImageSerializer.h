/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageSerializer_h
#define mitkImageSerializer_h

#include <mitkBaseDataSerializer.h>

namespace mitk
{
  /**
   * \brief Serializes mitk::Image for mitk::SceneIO.
   *
   * Writes the Image object to a NRRD file (.nrrd) in the configured working
   * directory using mitk::IOUtil. The filename is composed from a unique prefix
   * and the filename hint.
   *
   * \sa BaseDataSerializer, SceneIO, Image
   */
  class ImageSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(ImageSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Serialize the Image object to a .nrrd file.
     *
     * Casts the stored BaseData to mitk::Image and writes it to disk in NRRD format.
     *
     * \return The filename (without path) of the created file, or an empty string on failure.
     *
     * \pre The Data must be set and must be castable to mitk::Image.
     * \pre The WorkingDirectory must be set.
     */
    std::string Serialize() override;

  protected:
    ImageSerializer();
    ~ImageSerializer() override;
  };

} // namespace

#endif
