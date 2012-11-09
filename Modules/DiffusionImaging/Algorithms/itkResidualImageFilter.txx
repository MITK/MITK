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
#include <cmath>

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

    // per slice, per volume
    std::vector< std::vector <std::vector<double> > > residualsPerSlice;

    // Detrmine number of B0 images
    int numberB0=0;
    for(int i=0; i<m_Gradients->Size(); i++)
    {
      GradientDirectionType grad = m_Gradients->ElementAt(i);

      if(fabs(grad[0]) < 0.001 && fabs(grad[1]) < 0.001 && fabs(grad[2]) < 0.001)
      {
        numberB0++;
      }
    }

    residuals.resize(this->GetInput()->GetVectorLength()-numberB0);

    // Calculate the standard residual image and for each volume put all residuals in a vector
    for(int z=0; z<size[2]; z++)
    {
      std::vector< std::vector<double> > sliceResiduals; // residuals per volume for this slice
      sliceResiduals.resize(this->GetInput()->GetVectorLength()-numberB0);

      for(int y=0; y<size[1]; y++)
      {
        for(int x=0; x<size[0]; x++)
        {

          // Check if b0 exceeds threshold
          itk::Index<3> ix;
          ix[0] = x;
          ix[1] = y;
          ix[2] = z;


          typename InputImageType::PixelType p1 = this->GetInput()->GetPixel(ix);
          typename InputImageType::PixelType p2 = m_SecondDiffusionImage->GetPixel(ix);


          int s1 = p1.GetSize();
          int s2 = p2.GetSize();

          if(p1.GetSize() != p2.GetSize())
          {
            MITK_ERROR << "Vector sizes do not match";
            return;
          }


          if(p1.GetElement(m_B0Index) <= m_B0Threshold)
          {
            continue;
          }



          double res = 0;
          int shift = 0; // correction for the skipped B0 images

          for(int i = 0; i<p1.GetSize(); i++)
          {



            GradientDirectionType grad = m_Gradients->ElementAt(i);
            if(!(fabs(grad[0]) < 0.001 && fabs(grad[1]) < 0.001 && fabs(grad[2]) < 0.001))
            {
              double val1 = (double)p1.GetElement(i);
              double val2 = (double)p2.GetElement(i);

              res += abs(val1-val2);

              residuals[i-shift].push_back(val1-val2);
              sliceResiduals[i-shift].push_back(val1-val2);

            }
            else
            {
              shift++;
            }



          }


          res = res/p1.GetSize();

          outputImage->SetPixel(ix, res);

        }
      }

      residualsPerSlice.push_back(sliceResiduals);
    }





    // for each dw volume: sort the the measured residuals (for each voxel) to enable determining Q1 and Q3; calculate means
    // determine percentage of errors as described in QUALITY ASSESSMENT THROUGH ANALYSIS OF RESIDUALS OF DIFFUSION IMAGE FITTING
    // Leemans et al 2008

    double q1,q3, median;
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



      median = res[0.5*res.size()];
      double iqr = q3-q1;
      double outlierThreshold = median + 1.5*iqr;
      double numberOfOutliers = 0.0;

      std::vector<double>::iterator resIt = res.begin();
      double mean = 0;
      while(resIt != res.end())
      {
        double f = *resIt;
        if(f>outlierThreshold)
        {
          numberOfOutliers++;
        }
        mean += f;
        ++resIt;
      }

      double percOfOutliers = 100 * numberOfOutliers / res.size();
      m_PercentagesOfOutliers.push_back(percOfOutliers);

      mean /= res.size();
      m_Means.push_back(mean);

      ++it;
    }



    // Calculate for each slice the number of outliers per volume(dw volume)
    std::vector< std::vector <std::vector<double> > >::iterator sliceIt = residualsPerSlice.begin();

    while(sliceIt != residualsPerSlice.end())
    {
      std::vector< std::vector<double> > currentSlice = *sliceIt;
      std::vector<double> percentages;

      std::vector< std::vector<double> >::iterator volIt = currentSlice.begin();
      while(volIt != currentSlice.end())
      {


        std::vector<double> currentVolume = *volIt;

        //sort
        std::sort(currentVolume.begin(), currentVolume.end());



        q1 = currentVolume[0.25*currentVolume.size()];
        q3 = currentVolume[0.75*currentVolume.size()];
        median = currentVolume[0.5*currentVolume.size()];
        double iqr = q3-q1;
        double outlierThreshold = median + 1.5*iqr;
        double numberOfOutliers = 0.0;

        std::vector<double>::iterator resIt = currentVolume.begin();
        double mean;
        while(resIt != currentVolume.end())
        {
          double f = *resIt;
          if(f>outlierThreshold)
          {
            numberOfOutliers++;
          }
          mean += f;
          ++resIt;
       }

       double percOfOutliers = 100 * numberOfOutliers / currentVolume.size();
       percentages.push_back(percOfOutliers);

       ++volIt;
     }

     m_OutliersPerSlice.push_back(percentages);



      ++sliceIt;
    }




  }






} // end of namespace
