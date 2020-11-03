/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PICFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PICFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkAbstractFileReader.h"

#include "mitkImage.h"

#include "mitkIpPic.h"

namespace mitk
{
  //##Documentation
  //## @brief Reader to read files in DKFZ-pic-format
  class PicFileReader : public AbstractFileReader
  {
  public:
    PicFileReader();

    using AbstractFileReader::Read;

  protected:
    void FillImage(Image::Pointer image);

    Image::Pointer CreateImage();

    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    static void ConvertHandedness(mitkIpPicDescriptor *pic);

    PicFileReader *Clone() const override;
  };

} // namespace mitk

#endif /* PICFILEREADER_H_HEADER_INCLUDED_C1F48A22 */
