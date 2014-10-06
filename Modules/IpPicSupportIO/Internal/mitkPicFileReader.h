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


#ifndef PICFILEREADER_H_HEADER_INCLUDED_C1F48A22
#define PICFILEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkAbstractFileReader.h"

#include "mitkImage.h"

#include "mitkIpPic.h"


namespace mitk {
//##Documentation
//## @brief Reader to read files in DKFZ-pic-format
class PicFileReader : public AbstractFileReader
{

public:

  PicFileReader();

  using AbstractFileReader::Read;
  std::vector<mitk::BaseData::Pointer> Read();

protected:

  void FillImage(Image::Pointer image);

  Image::Pointer CreateImage();

private:

  static void ConvertHandedness(mitkIpPicDescriptor* pic);

  PicFileReader* Clone() const;

};

} // namespace mitk

#endif /* PICFILEREADER_H_HEADER_INCLUDED_C1F48A22 */
