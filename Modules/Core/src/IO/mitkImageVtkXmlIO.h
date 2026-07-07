/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkXmlIO_h
#define mitkImageVtkXmlIO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  /**
   * \brief Reader and writer for VTK XML image data format (.vti).
   *
   * Handles reading and writing of mitk::Image data using VTK's XML image
   * data format. Supports both file-based and stream-based I/O. Writing
   * is fully supported for 3D images and partially supported for lower
   * dimensions.
   *
   * \sa AbstractFileIO
   * \sa ImageVtkLegacyIO
   */
  class ImageVtkXmlIO : public mitk::AbstractFileIO
  {
  public:
    /** \brief Construct and register the VTK XML image I/O service. */
    ImageVtkXmlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    /**
     * \brief Determine reader confidence for the current input.
     *
     * For file-based input, checks whether vtkXMLImageDataReader can read the file.
     * For stream-based input, returns Supported unconditionally.
     *
     * \return The reader confidence level.
     */
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    /**
     * \brief Write a mitk::Image in VTK XML image data format.
     *
     * Supports writing to both files and output streams.
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
     * \brief Read VTK XML image data into a mitk::Image.
     *
     * Supports reading from both files and input streams.
     *
     * \return A vector containing the read Image.
     * \throw mitk::Exception on read failure.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ImageVtkXmlIO *IOClone() const override;
  };
}
#endif
