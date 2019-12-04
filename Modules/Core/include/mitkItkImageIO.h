/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKITKFILEIO_H
#define MITKITKFILEIO_H

#include "mitkAbstractFileIO.h"

#include <itkImageIOBase.h>

namespace mitk
{
  /**
   * This class wraps ITK image IO objects as mitk::IFileReader and
   * mitk::IFileWriter objects.
   *
   * Instantiating this class with a given itk::ImageIOBase instance
   * will register corresponding MITK reader/writer services for that
   * ITK ImageIO object.
   */
  class MITKCORE_EXPORT ItkImageIO : public AbstractFileIO
  {
  public:
    ItkImageIO(itk::ImageIOBase::Pointer imageIO);
    ItkImageIO(const CustomMimeType &mimeType, itk::ImageIOBase::Pointer imageIO, int rank);

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData>> Read() override;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  protected:
    virtual std::vector<std::string> FixUpImageIOExtensions(const std::string &imageIOName);
    virtual void FixUpCustomMimeTypeName(const std::string &imageIOName, CustomMimeType &customMimeType);

    // Fills the m_DefaultMetaDataKeys vector with default values
    virtual void InitializeDefaultMetaDataKeys();

  private:
    ItkImageIO(const ItkImageIO &other);

    ItkImageIO *IOClone() const override;

    itk::ImageIOBase::Pointer m_ImageIO;

    std::vector<std::string> m_DefaultMetaDataKeys;
  };

} // namespace mitk

#endif /* MITKITKFILEIO_H */
