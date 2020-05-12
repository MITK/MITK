/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKIMAGEVTKXMLIO_H
#define MITKIMAGEVTKXMLIO_H

#include "mitkAbstractFileIO.h"

namespace mitk
{
  class ImageVtkXmlIO : public mitk::AbstractFileIO
  {
  public:
    ImageVtkXmlIO();

    // -------------- AbstractFileReader -------------

    using AbstractFileReader::Read;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;

    ConfidenceLevel GetWriterConfidenceLevel() const override;
  protected:
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    ImageVtkXmlIO *IOClone() const override;
  };
}
#endif // MITKIMAGEVTKXMLIO_H
