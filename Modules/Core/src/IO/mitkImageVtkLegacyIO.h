/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIMAGEVTKLEGACYIO_H
#define MITKIMAGEVTKLEGACYIO_H

#include "mitkAbstractFileIO.h"

namespace mitk
{
  class ImageVtkLegacyIO : public mitk::AbstractFileIO
  {
  public:
    ImageVtkLegacyIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;

    ConfidenceLevel GetWriterConfidenceLevel() const override;
  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ImageVtkLegacyIO *IOClone() const override;
  };
}
#endif // MITKIMAGEVTKLEGACYIO_H
