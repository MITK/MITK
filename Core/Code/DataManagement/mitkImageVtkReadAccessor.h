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

#ifndef MITKIMAGEVTKREADACCESSOR_H
#define MITKIMAGEVTKREADACCESSOR_H

#include "mitkImageAccessorBase.h"

class vtkImageData;

namespace mitk {

class Image;
class ImageDataItem;

/**
 * @brief ImageVtkReadAccessor class provides any image read access which is required by Vtk methods
 * @ingroup Data
 */
class MITK_CORE_EXPORT ImageVtkReadAccessor : public ImageAccessorBase
{

public:

  /** \brief Creates an ImageVtkWriteAccessor for a whole Image
   *  \param Image::Pointer specifies the associated Image
   */
  ImageVtkReadAccessor(
      ImageConstPointer iP,
      const ImageDataItem* iDI,
      const vtkImageData* imageDataVtk
    );

  ~ImageVtkReadAccessor();

  const vtkImageData* GetVtkImageData() const;

  //vtkTypeMacro(ImageVtkWriteAccessor,vtkDataSet)

protected:

  virtual const Image* GetImage() const;

private:

  // due to smart pointer issues, the image is only kept as a weak pointer.
  const Image* m_Image;
  const vtkImageData* m_ImageDataVtk;
};

}

#endif // MITKIMAGEVTKREADACCESSOR_H
