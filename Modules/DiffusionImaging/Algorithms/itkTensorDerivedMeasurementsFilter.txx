
#ifndef __itkTensorDerivedMeasurementsFilter_txx
#define __itkTensorDerivedMeasurementsFilter_txx





namespace itk {

  template <class TPixel>
  TensorDerivedMeasurementsFilter<TPixel>::TensorDerivedMeasurementsFilter() : m_Measure(FA)
  {

  }

  template <class TPixel>
  void TensorDerivedMeasurementsFilter<TPixel>::GenerateData()
  {
    TensorImageType::Pointer tensorImage = static_cast< TensorImageType * >( this->ProcessObject::GetInput(0) );
    typedef ImageRegionConstIterator< TensorImageType > TensorImageIteratorType;
    typedef ImageRegionIterator< OutputImageType > OutputImageIteratorType;

    
    typename OutputImageType::Pointer outputImage = 
      static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    TensorImageType::RegionType region = tensorImage->GetLargestPossibleRegion();
    outputImage->SetRegions(region);
    outputImage->Allocate();

    TensorImageIteratorType tensorIt(tensorImage, tensorImage->GetLargestPossibleRegion());
    OutputImageIteratorType outputIt(outputImage, outputImage->GetLargestPossibleRegion());

    tensorIt.GoToBegin();
    outputIt.GoToBegin();

    while(!tensorIt.IsAtEnd() && !outputIt.IsAtEnd()){

      TensorType tensor = tensorIt.Get();

      TPixel fa = tensor.GetFractionalAnisotropy();

      outputIt.Set(fa);

      ++tensorIt;
      ++outputIt;
    }

    
  }


}  

#endif // __itkTensorDerivedMeasurements_txx
