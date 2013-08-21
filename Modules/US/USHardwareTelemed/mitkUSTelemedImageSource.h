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


#ifndef MITKUSTelemedImageSource_H_HEADER_INCLUDED_
#define MITKUSTelemedImageSource_H_HEADER_INCLUDED_

#include "mitkUSImageSource.h"

namespace mitk {

class MitkUS_EXPORT TelemedImageSource : public USImageSource
{
  virtual void GetNextRawImage( cv::Mat& );
  virtual void GetNextRawImage( mitk::Image::Pointer );
}

} // namespace mitk

#endif // MITKUSTelemedImageSource_H
