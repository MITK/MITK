
#ifndef __itkTensorDerivedMeasurementsFilter_txx
#define __itkTensorDerivedMeasurementsFilter_txx





namespace itk {

  template <class TPixel>
  TensorDerivedMeasurementsFilter<TPixel>::TensorDerivedMeasurementsFilter() : m_Measure(L1)
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
    outputImage->SetRegions(region);
    outputImage->Allocate();

    typename TensorImageIteratorType tensorIt(tensorImage, tensorImage->GetLargestPossibleRegion());
    typename OutputImageIteratorType outputIt(outputImage, outputImage->GetLargestPossibleRegion());

    tensorIt.GoToBegin();
    outputIt.GoToBegin();

    while(!tensorIt.IsAtEnd() && !outputIt.IsAtEnd()){

      typename TensorType tensor = tensorIt.Get();

      switch(m_Measure)
      {
        case this->FA:
        {
          TPixel diffusionIndex = tensor.GetFractionalAnisotropy();
          outputIt.Set(diffusionIndex);
          break;
        }
        case this->RA:
        {
          TPixel diffusionIndex = tensor.GetRelativeAnisotropy();
          outputIt.Set(diffusionIndex);
          break;
        }
        case this->L1:
        {
          TPixel diffusionIndex = tensor.GetNthComponent(0);
          outputIt.Set(diffusionIndex);
          break;
        }
        case this->DR:
        {
          TPixel diffusionIndex = (tensor.GetNthComponent(1) + tensor.GetNthComponent(2)) / 2;
          outputIt.Set(diffusionIndex);
          break;
        }
      }

      ++tensorIt;
      ++outputIt;
    }

    
  }


}  

#endif // __itkTensorDerivedMeasurements_txx
