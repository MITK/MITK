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

#include "QmitkImageCropper.h"
#include "mitkImageCropperEventInterface.h"


mitk::ImageCropperEventInterface::ImageCropperEventInterface()
{
}

mitk::ImageCropperEventInterface::~ImageCropperEventInterface()
{
}

void mitk::ImageCropperEventInterface::ExecuteOperation(mitk::Operation* op)
{
  m_ImageCropper->ExecuteOperation( op );
}
