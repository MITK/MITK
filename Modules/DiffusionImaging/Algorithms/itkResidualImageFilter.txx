/*=========================================================================

Program:   Tensor ToolKit - TTK
Module:    $URL: svn://scm.gforge.inria.fr/svn/ttk/trunk/Algorithms/itkTensorImageToDiffusionImageFilter.txx $
Language:  C++
Date:      $Date: 2010-06-07 13:39:13 +0200 (Mo, 07 Jun 2010) $
Version:   $Revision: 68 $

Copyright (c) INRIA 2010. All rights reserved.
See LICENSE.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itk_ResidualImageFilter_txx_
#define _itk_ResidualImageFilter_txx_
#endif

#include "itkResidualImageFilter.h"
#include <mitkCommon.h>

namespace itk
{




  template <class TInputScalarType, class TOutputScalarType>
  void
  ResidualImageFilter<TInputScalarType, TOutputScalarType>
  ::GenerateData()
  {
    typename InputImageType::SizeType size = this->GetInput()->GetLargestPossibleRegion().GetSize();
    typename InputImageType::SizeType size2 = m_SecondDiffusionImage->GetLargestPossibleRegion().GetSize();

    if(size != size2)
    {
      MITK_ERROR << "Sizes do not match";
      return;
    }

    // Initialize output image
    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    outputImage->SetSpacing( this->GetInput()->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( this->GetInput()->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( this->GetInput()->GetDirection() );  // Set the image direction
    outputImage->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
    outputImage->Allocate();
    outputImage->FillBuffer(0.0);





    std::vector< std::vector<double> > residuals;
    residuals.resize(this->GetInput()->GetVectorLength());



    // Calculate the standard residual image and for each volume put all residuals in a vector
    for(int x=0; x<size[0]; x++)
    {
      for(int y=0; y<size[1]; y++)
      {
        for(int z=0; z<size[2]; z++)
        {
          itk::Index<3> ix;
          ix[0] = x;
          ix[1] = y;
          ix[2] = z;

          typename InputImageType::PixelType p1 = this->GetInput()->GetPixel(ix);
          typename InputImageType::PixelType p2 = m_SecondDiffusionImage->GetPixel(ix);



          if(p1.GetSize() != p2.GetSize())
          {
            MITK_ERROR << "Vector sizes do not match";
            return;
          }

          double res = 0;

          for(int i = 0; i<p1.GetSize(); i++)
          {
            double val1 = (double)p1.GetElement(i);
            double val2 = (double)p2.GetElement(i);

            res += abs(val1-val2);

            residuals[i].push_back(val1-val2);

          }


          res = res/p1.GetSize();

          outputImage->SetPixel(ix, res);

        }
      }
    }


    // for each dw volume: sort the the measured residuals (for each voxel) to enable determining Q1 and Q3; calculate means


    double q1,q3;
    std::vector< std::vector<double> >::iterator it = residuals.begin();
    while(it != residuals.end())
    {
      std::vector<double> res = *it;

      // sort
      std::sort(res.begin(), res.end());

      q1 = res[0.25*res.size()];
      m_Q1.push_back(q1);
      q3 = res[0.75*res.size()];
      m_Q3.push_back(q3);

      std::vector<double>::iterator resIt = res.begin();
      double mean;
      while(resIt != res.end())
      {
        double f = *resIt;
        mean += f;
        ++resIt;
      }

      mean /= res.size();
      m_Means.push_back(mean);

      std::cout << "mean: " << mean << '\n'<< "\nq1: " << q1 << "\nq3: " << q3 << '\n';

      ++it;
    }
    std::cout << std::endl;



  }






} // end of namespace
