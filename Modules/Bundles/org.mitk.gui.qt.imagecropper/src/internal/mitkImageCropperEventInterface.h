/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-02-26 12:27:39 +0100 (Fr, 26 Feb 2010) $
Version:   $Revision: 21501 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
