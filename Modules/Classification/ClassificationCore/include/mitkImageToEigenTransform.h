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


#ifndef mitkImageToEigenTransform_h
#define mitkImageToEigenTransform_h

#include <MitkClassificationCoreExports.h>
#include <mitkImageToImageFilter.h>

// Eigen
#include <Eigen/Core>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT ImageToEigenTransform
  {
  public:

    ///
    /// @brief MatrixType
    ///
    typedef Eigen::MatrixXd MatrixType;

    ///
    /// @brief VectorType
    ///
    typedef Eigen::VectorXd VectorType;

    static VectorType transform(const mitk::Image::Pointer & image, const mitk::Image::Pointer & mask);

  private:
    typedef itk::Image<double, 3> DoubleImageType;
    typedef itk::Image<unsigned char, 3> UCharImageType;

  };
}

#endif //mitkImageToEigenTransform_h
