/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itSmoothedClassProbabilites_cpp
#define itSmoothedClassProbabilites_cpp

#include "itkSmoothedClassProbabilites.h"
#include "itkObjectFactory.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include <itkDiscreteGaussianImageFilter.h>


#include <mitkImageCast.h>
#include <mitkIOUtil.h>

#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
#include <sstream>
namespace itk
{

template< class TImage>
void SmoothedClassProbabilites< TImage>
::GenerateData()
{

  typename TImage::Pointer out = this->GetOutput(0);
  out->SetRegions(this->GetInput(0)->GetLargestPossibleRegion());
  out->Allocate();

  for(unsigned int i = 0 ; i < this->GetNumberOfInputs(); i++)
  {

    auto gf = itk::DiscreteGaussianImageFilter<TImage,TImage>::New();
    gf->SetInput(this->GetInput(i));
    gf->SetVariance(this->m_Sigma);
    gf->Update();

    ImageRegionConstIterator<TImage> git(gf->GetOutput(),gf->GetOutput()->GetLargestPossibleRegion());
    ImageRegionIterator<TImage> maskiter(m_MaskImage, m_MaskImage->GetLargestPossibleRegion());
    ImageRegionIterator<TImage> outputIter(out, out->GetLargestPossibleRegion());

    while (!outputIter.IsAtEnd())
    {
      if(maskiter.Value() > 0 ){

        if(git.Value() > outputIter.Value())
          outputIter.Set(i);
      }else
      {
        outputIter.Set(0);
      }

      ++git;
      ++outputIter;
      ++maskiter;
    }
  }






}

}// end namespace

#endif // itSmoothedClassProbabilites_cpp
