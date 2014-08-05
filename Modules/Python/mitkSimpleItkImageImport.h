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

#ifndef __MITKSIMPLEITKIMAGEIMPORT_H
#define __MITKSIMPLEITKIMAGEIMPORT_H

#include <itkSmartPointer.h>
#include <SimpleITK.h>

// Export macro
#include <MitkPythonExports.h>

namespace mitk
{

class Image;

/** @brief Class that provides methods to copy a SimpleITK image to an MITK image
  * and vice versa.
  */
class MITK_PYTHON_EXPORT SimpleItkImageImport
{

public:

  /**
    * Copy the content of a MITK image into an SimpleITK image.
    * @param The source MITK image.
    * @returns A copy of the MITK image as SimpleITK image.
    */
  static itk::simple::Image MitkToSimpleItkImage(mitk::Image* image);

  /**
    * This method deep copies an SimpleITK image into a MITK image and return it.
    * @param The source image
    * @returns A copy as MITK image.
    */
  static itk::SmartPointer<mitk::Image> SimpleItkToMitkImage(itk::simple::Image& sitkImage);
};

}

#endif
