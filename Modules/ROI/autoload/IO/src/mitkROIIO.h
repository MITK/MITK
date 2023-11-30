/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkROIIO_h
#define mitkROIIO_h

#include <mitkAbstractFileIO.h>

namespace mitk
{
  class ROIIO : public AbstractFileIO
  {
  public:
    ROIIO();

    using AbstractFileReader::Read;
    void Write() override;

  protected:
    std::vector<BaseData::Pointer> DoRead() override;

  private:
    ROIIO* IOClone() const override;
  };
}

#endif
