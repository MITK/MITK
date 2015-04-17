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
#include <mitkImageCast.h>
#include <itkImageRegionConstIterator.h>

// Eigen
#include <Eigen/Core>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT ImageToEigenTransform
  {
  public:

    template<typename TEigenMatrixType>
    static void transform(const mitk::Image::Pointer & image, const mitk::Image::Pointer & mask, TEigenMatrixType & out_matrix)
    {
      UCharImageType::Pointer current_mask;
      mitk::CastToItkImage(mask,current_mask);

      DoubleImageType::Pointer current_input;
      mitk::CastToItkImage(image,current_input);

      unsigned int n_numSamples = countIf(mask, [](double c){ return c > 0;});

      out_matrix.resize(n_numSamples,1);
      out_matrix.fill(0);

      auto mit = itk::ImageRegionConstIterator<UCharImageType>(current_mask, current_mask->GetLargestPossibleRegion());
      auto iit = itk::ImageRegionConstIterator<DoubleImageType>(current_input,current_input->GetLargestPossibleRegion());
      unsigned int current_row = 0;
      while (!mit.IsAtEnd()) {
        if(mit.Value() > 0)
          out_matrix(current_row++) = iit.Value();
        ++mit;
        ++iit;
      }

    }

    static unsigned int countIf(const mitk::Image::Pointer & maks, bool (*func_pointer) (double));

  private:
    typedef itk::Image<double, 3> DoubleImageType;
    typedef itk::Image<unsigned char, 3> UCharImageType;

  };
}

#endif //mitkImageToEigenTransform_h
