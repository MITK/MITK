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


#ifndef mitkEigenToImageTransform_h
#define mitkEigenToImageTransform_h

#include <MitkClassificationCoreExports.h>
#include <mitkImageCast.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

// Eigen
#include <Eigen/Dense>

namespace mitk
{
  class MITKCLASSIFICATIONCORE_EXPORT EigenToImageTransform
  {
  public:

    template<typename TEigenMatrixtype>
    static mitk::Image::Pointer transform(const TEigenMatrixtype & matrix, const mitk::Image::Pointer & mask)
    {
      UCharImageType::Pointer itkMask;
      mitk::CastToItkImage(mask,itkMask);

      unsigned int n_numSamples = 0;
      {
        auto mit = itk::ImageRegionConstIterator<UCharImageType>(itkMask, itkMask->GetLargestPossibleRegion());
        while (!mit.IsAtEnd())
        {
          if(mit.Value() > 0) n_numSamples++;
          ++mit;
        }
      }

      if(n_numSamples != matrix.rows())
        MITK_ERROR << "Number of samples in matrix and number of points under the masks is not the same!";


      mitk::Image::Pointer output = mask->Clone();

      DoubleImageType::Pointer itkImg;
      mitk::CastToItkImage(output,itkImg);

      auto mit = itk::ImageRegionConstIterator<UCharImageType>(itkMask, itkMask->GetLargestPossibleRegion());
      auto oit = itk::ImageRegionIterator<DoubleImageType>(itkImg, itkImg->GetLargestPossibleRegion());

      unsigned int current_row = 0;
      while(!mit.IsAtEnd())
      {
        if(mit.Value() > 0)
          oit.Set(matrix(current_row++,0));
        else
          oit.Set(0.0);
        ++mit;
        ++oit;
      }

      mitk::CastToMitkImage(itkImg,output);
      return output;
    }

  private:
    typedef itk::Image<unsigned char, 3> UCharImageType;
    typedef itk::Image<double, 3> DoubleImageType;

  };
}

#endif //mitkImageToEigenTransform_h
