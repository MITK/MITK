#ifndef itkNeighborhoodFunctorImageFilter_cpp
#define itkNeighborhoodFunctorImageFilter_cpp

#include "itkNeighborhoodFunctorImageFilter.h"
#include "itkNeighborhoodAlgorithm.h"
#include <itkVectorImageToImageAdaptor.h>

namespace itk
{

template< typename TInputImage, typename TFeatureImageType , class  FunctorType>
void
NeighborhoodFunctorImageFilter< TInputImage, TFeatureImageType, FunctorType >
::BeforeThreadedGenerateData()
{
  const TInputImage * input = this->GetInput(0);

  for(unsigned int i = 0 ; i < FunctorType::OutputCount; i ++)
  {
    typename FeatureImageType::Pointer output = TFeatureImageType::New();
    output->SetRegions(input->GetLargestPossibleRegion());
    output->SetSpacing(input->GetSpacing());
    output->SetOrigin(input->GetOrigin());
    output->SetDirection(input->GetDirection());
    output->Allocate();
    this->SetNthOutput( i, output.GetPointer() );
  }

  if(m_MaskImage.IsNull())
  {
    m_MaskImage = MaskImageType::New();
    m_MaskImage->SetRegions(input->GetLargestPossibleRegion());
    m_MaskImage->Allocate();
    m_MaskImage->FillBuffer(1);
  }
}

template< typename TInputImage, typename TFeatureImageType , class  FunctorType>
void
NeighborhoodFunctorImageFilter< TInputImage, TFeatureImageType, FunctorType >
::GenerateInputRequestedRegion()
{

  // call the superclass' implementation of this method. this should
  // copy the output requested region to the input requested region
  Superclass::GenerateInputRequestedRegion();

  // get pointers to the input and output
  auto inputPtr = const_cast<InputImageType *>(this->GetInput());

  if ( !inputPtr )
  {
    return;
  }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_Size );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop( inputPtr->GetLargestPossibleRegion() ) )
  {
    inputPtr->SetRequestedRegion(inputRequestedRegion);
    return;
  }
  else
  {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion(inputRequestedRegion);

    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
  }
}

template< typename TInputImage, typename TFeatureImageType, class FunctorType >
void
NeighborhoodFunctorImageFilter< TInputImage, TFeatureImageType, FunctorType >
::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                       ThreadIdType /*threadId*/)
{


  typedef NeighborhoodAlgorithm::ImageBoundaryFacesCalculator< InputImageType > BFC;
  typedef typename BFC::FaceListType FaceListType;
  BFC           faceCalculator;
  FaceListType  faceList;

  // Allocate output
  const InputImageType *input = this->GetInput();

  // Break the input into a series of regions.  The first region is free
  // of boundary conditions, the rest with boundary conditions. Note,
  // we pass in the input image and the OUTPUT requested region. We are
  // only concerned with centering the neighborhood operator at the
  // pixels that correspond to output pixels.
  faceList = faceCalculator( input, outputRegionForThread, m_Size );

  typename FaceListType::iterator fit;
  ImageRegionConstIterator< MaskImageType > mit;

  // Process non-boundary region and each of the boundary faces.
  // These are N-d regions which border the edge of the buffer.
  ConstNeighborhoodIterator< InputImageType > bit;
//  for ( fit = faceList.begin(); fit != faceList.end(); ++fit )
//  {
    bit = ConstNeighborhoodIterator< InputImageType >(m_Size, input, outputRegionForThread);

    mit = ImageRegionConstIterator< MaskImageType >(m_MaskImage, outputRegionForThread);

    std::vector<ImageRegionIterator< FeatureImageType > > featureImageIterators;

    for(unsigned int i = 0; i < FunctorType::OutputCount; i++)
    {
      featureImageIterators.push_back(ImageRegionIterator< FeatureImageType >(this->GetOutput(i), outputRegionForThread));
      featureImageIterators[i].GoToBegin();
    }

    mit.GoToBegin();
    bit.GoToBegin();

    while ( !bit.IsAtEnd() || !mit.IsAtEnd() )
    {
      if(mit.Value() != 0)
      {
        //bit.GetNeighborhood().Print(std::cout);
        typename FunctorType::OutputVectorType features = ( m_Functor( bit ) );

        for(unsigned int i = 0 ; i < FunctorType::OutputCount; i++)
          featureImageIterators[i].Set(features[i]);
      }

      for(unsigned int i = 0 ; i < FunctorType::OutputCount; i++)
        ++featureImageIterators[i];
      ++bit;
      ++mit;
    }
//  }

  std::cout << "Thread done!" << std::endl;
}
} // end namespace itk


#endif //itkNeighborhoodFunctorImageFilter_cpp
