#ifndef itSmoothedClassProbabilites_cpp
#define itSmoothedClassProbabilites_cpp

#include "itkSmoothedClassProbabilites.h"
#include "itkObjectFactory.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageIterator.h"
#include "itkImageConstIterator.h"
#include <itkDiscreteGaussianImageFilter.h>

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace itk
{

template< class TImage>
void SmoothedClassProbabilites< TImage>
::GenerateData()
{
  //    typename TImage::ConstPointer input = this->GetInput();
  typename TImage::Pointer output = this->GetOutput();
  std::vector<ImageRegionConstIterator<TImage> > vec_iter;
  auto arr = this->GetIndexedInputs();

  for(auto it = arr.begin(); it != arr.end(); it ++)
  {
    typename itk::DiscreteGaussianImageFilter<TImage,TImage>::Pointer gf = itk::DiscreteGaussianImageFilter<TImage,TImage>::New();
    gf->SetInput(static_cast<TImage *>(it->GetPointer()));
    gf->SetVariance(this->m_Sigma);
    gf->Update();
    *it = gf->GetOutput();

    ImageRegionConstIterator<TImage> imageiterator(static_cast<TImage *>(it->GetPointer()),static_cast<TImage *>(it->GetPointer())->GetLargestPossibleRegion());
    vec_iter.push_back(imageiterator);
  }

  ImageRegionConstIterator<TImage> maskiter(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());
  ImageRegionIterator<TImage> outputIter(output, output->GetLargestPossibleRegion());
  while (!outputIter.IsAtEnd())
  {

    if(maskiter.Value() > 0 ){
      int max_class = 0;
      double prob = 0.0;
      for(unsigned int i = 0 ; i < vec_iter.size(); i++)
      {
        if(vec_iter[i].Value() > prob)
        {
          max_class = i+1;
          prob = vec_iter[i].Value();
        }
      }
      outputIter.Set(max_class);
    }else
    {
      outputIter.Set(0);
    }

    for(unsigned int i = 0 ; i < vec_iter.size(); i++)
      ++vec_iter[i];

    ++outputIter;
    ++maskiter;
  }

}

}// end namespace

#endif // itSmoothedClassProbabilites_cpp
