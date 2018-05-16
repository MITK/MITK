/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
    std::vector<BaseData::Pointer> Read() override;

    ConfidenceLevel GetReaderConfidenceLevel() const override;

    // -------------- AbstractFileWriter -------------

    void Write() override;

    ConfidenceLevel GetWriterConfidenceLevel() const override;

  private:
    ImageVtkXmlIO *IOClone() const override;
  };
}
#endif // MITKIMAGEVTKXMLIO_H
