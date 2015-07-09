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

#ifndef itkLabelSampler_cpp
#define itkLabelSampler_cpp

#include "itkLabelSampler.h"
#include "itkObjectFactory.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageIterator.h"
#include "itkImageConstIterator.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

namespace itk
{

template< class TImage>
void LabelSampler< TImage>
::GenerateData()
{


  srand (time(NULL));
  this->AllocateOutputs();

  typename TImage::ConstPointer input = this->GetInput();
  TImage * output = this->GetOutput();

  m_NumberOfSampledVoxels = 0;

  ImageRegionConstIterator<TImage> inputIter(input, input->GetLargestPossibleRegion());
  ImageRegionIterator<TImage> outputIter(output, output->GetLargestPossibleRegion());
  while (!inputIter.IsAtEnd())
  {
    if (inputIter.Get() > 0)
    {

      if(m_LabelVoxelCountMap.find(inputIter.Get()) == m_LabelVoxelCountMap.end())
        m_LabelVoxelCountMap[inputIter.Get()] = 0;
      m_LabelVoxelCountMap[inputIter.Get()]++;

      double r;
      if(inputIter.Get() == m_Label || m_Label == -1 ){
        r = (double)(rand()) / RAND_MAX;
        if (r < m_AcceptRate)
        {
          outputIter.Set(inputIter.Get());
          m_NumberOfSampledVoxels++;
        }
      }else
        outputIter.Set(0);

    }
    else
    {
      outputIter.Set(0);
    }
    ++inputIter;
    ++outputIter;
  }

}

}// end namespace

#endif // itkLabelSampler_cpp
