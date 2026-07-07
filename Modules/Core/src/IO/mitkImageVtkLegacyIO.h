/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkLegacyIO_h
#define mitkImageVtkLegacyIO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for VTK legacy structured points image format.
   *
   * Handles reading and writing of mitk::Image data using VTK's legacy
   * structured points file format (.vtk). Supports an option to save in
   * binary format. Writing is fully supported for 3D images and partially
   * supported for lower dimensions.
   *
   * \sa AbstractFileIO
   * \sa ImageVtkXmlIO
   */
  class ImageVtkLegacyIO : public mitk::AbstractFileIO
  {
  public:
    /** \brief Construct and register the VTK legacy image I/O service. */
    ImageVtkLegacyIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /**
     * \brief Determine reader confidence for the current input.
     *
     * Returns Supported if the file contains VTK structured points data.
     *
     * \return The reader confidence level.
     */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /**
     * \brief Write a mitk::Image in VTK legacy structured points format.
     *
     * Optionally writes in binary format based on the "Save as binary file" option.
     *
     * \throw mitk::Exception on write failure.
     */
    void Write() override;

    /**
     * \brief Determine writer confidence for the current input.
     *
     * Returns Supported for 3D images, PartiallySupported for lower dimensions.
     *
     * \return The writer confidence level.
     */
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    /**
     * \brief Read VTK legacy structured points data into a mitk::Image.
     * \return A vector containing the read Image.
     * \throw mitk::Exception on read failure.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ImageVtkLegacyIO *IOClone() const override;
  };
}
#endif
