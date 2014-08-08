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

#ifndef MITKIMAGEVTKWRITEACCESSOR_H
#define MITKIMAGEVTKWRITEACCESSOR_H

#include "mitkImageAccessorBase.h"

class vtkImageData;

namespace mitk {

class Image;
class ImageDataItem;

/**
 * @brief ImageVtkWriteAccessor class provides any image write access which is required by Vtk methods
 * @ingroup Data
 */
class MITK_CORE_EXPORT ImageVtkWriteAccessor : public ImageAccessorBase
{

public:

  typedef itk::SmartPointer<Image> ImagePointer;

  /** \brief Creates an ImageVtkWriteAccessor for a whole Image
   *  \param Image::Pointer specifies the associated Image
   */
  ImageVtkWriteAccessor(
      ImagePointer iP,
      const ImageDataItem* iDI,
      vtkImageData* imageDataVtk
    );

  ~ImageVtkWriteAccessor();

  vtkImageData* GetVtkImageData() const;

protected:

  virtual const Image* GetImage() const;

private:

  // due to smart pointer issues, the image is only kept as a weak pointer.
  Image* m_Image;
  vtkImageData* m_ImageDataVtk;
};

}

#endif // MITKIMAGEVTKWRITEACCESSOR_H
