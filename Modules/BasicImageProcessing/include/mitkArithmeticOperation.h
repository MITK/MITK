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

#ifndef mitkArithmeticOperation_h
#define mitkArithmeticOperation_h

#include <mitkImage.h>
#include <MitkBasicImageProcessingExports.h>

namespace mitk
{
  /** \brief Executes a arithmetic operations on one or two images
  *
  * All parameters of the arithmetic operations must be specified during construction.
  * The actual operation is executed when calling GetResult().
  */
  class MITKBASICIMAGEPROCESSING_EXPORT ArithmeticOperation {
    static mitk::Image::Pointer Add(mitk::Image::Pointer & imageA, mitk::Image::Pointer & imageB, bool outputAsDouble = true);
}

#endif // mitkArithmeticOperation_h