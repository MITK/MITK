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

#ifndef __itkTensorDerivedMeasurementsFilter_txx
#define __itkTensorDerivedMeasurementsFilter_txx





namespace itk {

  template <class TPixel>
      TensorDerivedMeasurementsFilter<TPixel>::TensorDerivedMeasurementsFilter() : m_Measure(AD)
  {
  }

  template <class TPixel>
      void TensorDerivedMeasurementsFilter<TPixel>::GenerateData()
  {
    typename TensorImageType::Pointer tensorImage = static_cast< TensorImageType * >( this->ProcessObject::GetInput(0) );
    typedef ImageRegionConstIterator< TensorImageType > TensorImageIteratorType;
    typedef ImageRegionIterator< OutputImageType > OutputImageIteratorType;


    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    typename TensorImageType::RegionType region = tensorImage->GetLargestPossibleRegion();


    outputImage->SetSpacing( tensorImage->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( tensorImage->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( tensorImage->GetDirection() );  // Set the image direction
    outputImage->SetRegions( tensorImage->GetLargestPossibleRegion());
    outputImage->Allocate();

    TensorImageIteratorType tensorIt(tensorImage, tensorImage->GetLargestPossibleRegion());
    OutputImageIteratorType outputIt(outputImage, outputImage->GetLargestPossibleRegion());

    tensorIt.GoToBegin();
    outputIt.GoToBegin();

    while(!tensorIt.IsAtEnd() && !outputIt.IsAtEnd()){

      TensorType tensor = tensorIt.Get();

      switch(m_Measure)
      {
      case FA:
        {
          TPixel diffusionIndex = tensor.GetFractionalAnisotropy();
          outputIt.Set(diffusionIndex);
          break;
        }
      case RA:
        {
          TPixel diffusionIndex = tensor.GetRelativeAnisotropy();
          outputIt.Set(diffusionIndex);
          break;
        }
      case AD:
        {
          // eigenvalues are sorted in ascending order by default because the
          // itk::SymmetricEigenAnalysis defaults are not touched in the tensor implementation
          typename TensorType::EigenValuesArrayType evs;
          tensor.ComputeEigenValues(evs);
          outputIt.Set(evs[2]);
          break;
        }
      case RD:
        {
          // eigenvalues are sorted in ascending order by default because the
          // itk::SymmetricEigenAnalysis defaults are not touched in the tensor implementation
          typename TensorType::EigenValuesArrayType evs;
          tensor.ComputeEigenValues(evs);
          outputIt.Set((evs[0]+evs[1])/2.0);

          break;
        }
      case CA:
        {
          // eigenvalues are sorted in ascending order by default because the
          // itk::SymmetricEigenAnalysis defaults are not touched in the tensor implementation
          typename TensorType::EigenValuesArrayType evs;
          tensor.ComputeEigenValues(evs);
          if (evs[2] == 0)
          {
            outputIt.Set(0);
            break;
          }
          outputIt.Set(1.0-(evs[0]+evs[1])/(2.0*evs[2]));
          break;
        }
      case L2:
        {
          // eigenvalues are sorted in ascending order by default because the
          // itk::SymmetricEigenAnalysis defaults are not touched in the tensor implementation
          typename TensorType::EigenValuesArrayType evs;
          tensor.ComputeEigenValues(evs);
          outputIt.Set(evs[1]);
          break;
        }
      case L3:
        {
          // eigenvalues are sorted in ascending order by default because the
          // itk::SymmetricEigenAnalysis defaults are not touched in the tensor implementation
          typename TensorType::EigenValuesArrayType evs;
          tensor.ComputeEigenValues(evs);
          outputIt.Set(evs[0]);
          break;
        }
      case MD:
        {
          typename TensorType::EigenValuesArrayType evs;
          tensor.ComputeEigenValues(evs);
          outputIt.Set((evs[0]+evs[0]+evs[0])/3.0);
          break;
        }
      }

      ++tensorIt;
      ++outputIt;
    }


  }


}

#endif // __itkTensorDerivedMeasurements_txx
