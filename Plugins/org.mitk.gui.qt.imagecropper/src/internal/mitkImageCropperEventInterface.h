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

#ifndef MITKIMAGECROPPEREVENTINTERFACE_H
#define MITKIMAGECROPPEREVENTINTERFACE_H

#include "itkObject.h"

#include "mitkOperation.h"
#include "mitkOperationActor.h"

class QmitkImageCropper;

namespace mitk
{
  class ImageCropperEventInterface : public itk::Object, public OperationActor
  {

  public:

    ImageCropperEventInterface();
    ~ImageCropperEventInterface();

    void SetImageCropper( QmitkImageCropper* imageCropper )
    {
      m_ImageCropper = imageCropper;
    }

    virtual void  ExecuteOperation(mitk::Operation* op);

  private:

    QmitkImageCropper* m_ImageCropper;

  };
}

#endif // MITKIMAGECROPPEREVENTINTERFACE_H
