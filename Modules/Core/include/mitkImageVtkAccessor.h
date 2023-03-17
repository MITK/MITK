/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageVtkAccessor_h
#define mitkImageVtkAccessor_h

#include "mitkImageVtkWriteAccessor.h"

namespace mitk
{
  /**
   * @brief ImageVtkAccessor class provides any image read access which is required by Vtk methods
   * @ingroup Data
   * @deprecatedSince{2014_10} Use ImageVtkWriteAccessor instead
   */
  class ImageVtkAccessor : public ImageVtkWriteAccessor
  {
  protected:
    ImageVtkAccessor(ImagePointer iP, const ImageDataItem *iDI);

  public:
    DEPRECATED(static ImageVtkAccessor *New(ImagePointer, const ImageDataItem *));
  };
}

#endif
