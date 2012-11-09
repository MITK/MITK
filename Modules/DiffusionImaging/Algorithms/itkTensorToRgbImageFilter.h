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

#ifndef __itkTensorToRgbImageFilter_h
#define __itkTensorToRgbImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkRGBAPixel.h"

namespace itk
{

#define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );

  /** \class TensorToRgbImageFilter
 *
 */
  template <typename TInputImage,
  typename TOutputImage=itk::Image<itk::RGBAPixel<unsigned char>,3> >
                        class TensorToRgbImageFilter :
                        public ImageToImageFilter<TInputImage,TOutputImage>
                        {
                        public:
    /** Standard class typedefs. */
    typedef TensorToRgbImageFilter  Self;
    typedef ImageToImageFilter<TInputImage,TOutputImage>
        Superclass;
    typedef SmartPointer<Self>                     Pointer;
    typedef SmartPointer<const Self>               ConstPointer;

    typedef typename Superclass::InputImageType    InputImageType;
    typedef typename Superclass::OutputImageType    OutputImageType;
    typedef typename OutputImageType::PixelType     OutputPixelType;
    typedef typename TInputImage::PixelType         InputPixelType;
    typedef typename InputPixelType::ValueType      InputValueType;

    /** Run-time type information (and related methods).   */
    itkTypeMacro( TensorToRgbImageFilter, ImageToImageFilter );

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Print internal ivars */
    void PrintSelf(std::ostream& os, Indent indent) const
    { this->Superclass::PrintSelf( os, indent ); }

#ifdef ITK_USE_CONCEPT_CHECKING
    /** Begin concept checking */
    itkConceptMacro(InputHasNumericTraitsCheck,
                    (Concept::HasNumericTraits<InputValueType>));
    /** End concept checking */
#endif

protected:
    TensorToRgbImageFilter() {};
    virtual ~TensorToRgbImageFilter() {};

    void GenerateData()
    {

      typename InputImageType::Pointer tensorImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

      typename OutputImageType::Pointer outputImage =
          static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

      typename InputImageType::RegionType region = tensorImage->GetLargestPossibleRegion();

      outputImage->SetSpacing( tensorImage->GetSpacing() );   // Set the image spacing
      outputImage->SetOrigin( tensorImage->GetOrigin() );     // Set the image origin
      outputImage->SetDirection( tensorImage->GetDirection() );  // Set the image direction
      outputImage->SetRegions( tensorImage->GetLargestPossibleRegion());
      outputImage->Allocate();

      typedef ImageRegionConstIterator< InputImageType > TensorImageIteratorType;
      TensorImageIteratorType tensorIt(tensorImage, tensorImage->GetLargestPossibleRegion());

      typedef ImageRegionIterator< OutputImageType > OutputImageIteratorType;
      OutputImageIteratorType outputIt(outputImage, outputImage->GetLargestPossibleRegion());

      tensorIt.GoToBegin();
      outputIt.GoToBegin();

      while(!tensorIt.IsAtEnd() && !outputIt.IsAtEnd()){

        InputPixelType x = tensorIt.Get();

        // eigenvalues are sorted in ascending order by default because the
        // itk::SymmetricEigenAnalysis defaults are not touched in the tensor implementation

        typename InputPixelType::EigenValuesArrayType eigenvalues;
        typename InputPixelType::EigenVectorsMatrixType eigenvectors;
        x.ComputeEigenAnalysis(eigenvalues, eigenvectors);

        //      int index = 2;
        //      if( (eigenvalues[0] >= eigenvalues[1])
        //        && (eigenvalues[0] >= eigenvalues[2]) )
        //        index = 0;
        //      else if(eigenvalues[1] >= eigenvalues[2])
        //        index = 1;

        const float fa = x.GetFractionalAnisotropy();
        float r = fabs(eigenvectors(2/*index*/,0)) * fa;
        float g = fabs(eigenvectors(2/*index*/,1)) * fa;
        float b = fabs(eigenvectors(2/*index*/,2)) * fa;
//        float a = (fa-(m_OpacLevel-m_OpacWindow/2.0f))/m_OpacWindow;
        float a = fa;

        __IMG_DAT_ITEM__CEIL_ZERO_ONE__(r);
        __IMG_DAT_ITEM__CEIL_ZERO_ONE__(g);
        __IMG_DAT_ITEM__CEIL_ZERO_ONE__(b);
        __IMG_DAT_ITEM__CEIL_ZERO_ONE__(a);

        OutputPixelType out;
        out.SetRed(   r * 255.0f);
        out.SetGreen( g * 255.0f);
        out.SetBlue(  b * 255.0f);
        out.SetAlpha( a * 255.0f);

        outputIt.Set(out);

        ++tensorIt;
        ++outputIt;
      }

    }

  private:
    TensorToRgbImageFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
  };



} // end namespace itk

#endif
