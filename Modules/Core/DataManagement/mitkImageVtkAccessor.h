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

#ifndef MITKIMAGEVTKACCESSOR_H
#define MITKIMAGEVTKACCESSOR_H

#include "mitkImageVtkWriteAccessor.h"

namespace mitk {

/**
 * @brief ImageVtkAccessor class provides any image read access which is required by Vtk methods
 * @ingroup Data
 * @deprecatedSince{2014_10} Use ImageVtkWriteAccessor instead
 */
class ImageVtkAccessor : public ImageVtkWriteAccessor
{
protected:

  ImageVtkAccessor(
      ImagePointer iP,
      const ImageDataItem* iDI
      );

public:

  DEPRECATED(static ImageVtkAccessor* New(ImagePointer, const ImageDataItem*));
};

}

#endif // MITKIMAGEVTKACCESSOR_H
