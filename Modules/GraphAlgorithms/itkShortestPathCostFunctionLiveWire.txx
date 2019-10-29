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

#ifndef __itkShortestPathCostFunctionLiveWire_txx
#define __itkShortestPathCostFunctionLiveWire_txx

#include "itkShortestPathCostFunctionLiveWire.h"

#include <cmath>

#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkGradientImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkLaplacianImageFilter.h>
#include <itkStatisticsImageFilter.h>
#include <itkZeroCrossingImageFilter.h>

namespace itk
{
  // Constructor
  template <class TInputImageType>
  ShortestPathCostFunctionLiveWire<TInputImageType>::ShortestPathCostFunctionLiveWire(): m_MinCosts(0.0), m_UseRepulsivePoints(false), m_GradientMax(0.0), m_Initialized(false),  m_UseCostMap(false), m_MaxMapCosts(-1.0)
  {
  }

  template <class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>::AddRepulsivePoint(const IndexType &index)
  {
    this->m_MaskImage->SetPixel(index, 255);
    m_UseRepulsivePoints = true;
  }

  template <class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>::RemoveRepulsivePoint(const IndexType &index)
  {
    this->m_MaskImage->SetPixel(index, 0);
  }

  template <class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>::SetImage(const TInputImageType *_arg)
  {
    if (this->m_Image != _arg)
    {
      this->m_Image = _arg;

      // initialize mask image
      this->m_MaskImage = UnsignedCharImageType::New();
      this->m_MaskImage->SetRegions(this->m_Image->GetLargestPossibleRegion());
      this->m_MaskImage->SetOrigin(this->m_Image->GetOrigin());
      this->m_MaskImage->SetSpacing(this->m_Image->GetSpacing());
      this->m_MaskImage->SetDirection(this->m_Image->GetDirection());
      this->m_MaskImage->Allocate();
      this->m_MaskImage->FillBuffer(0);

      this->Modified();
      this->m_Initialized = false;
    }
  }

  template <class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>::ClearRepulsivePoints()
  {
    m_UseRepulsivePoints = false;
    this->m_MaskImage->FillBuffer(0);
  }

  template <class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>::GetCost(IndexType p1, IndexType p2)
  {
    // local component costs
    // weights
    double w1;
    double w2;
    double w3;
    double costs = 0.0;

    // if we are on the mask, return asap
    if (m_UseRepulsivePoints)
    {
      if ((this->m_MaskImage->GetPixel(p1) != 0) || (this->m_MaskImage->GetPixel(p2) != 0))
        return 1000;
    }

    double gradientX, gradientY;
    gradientX = gradientY = 0.0;

    double gradientCost;

    double gradientMagnitude;

    // Gradient Magnitude costs
    gradientMagnitude = this->m_GradientMagnitudeImage->GetPixel(p2);
    gradientX = m_GradientImage->GetPixel(p2)[0];
    gradientY = m_GradientImage->GetPixel(p2)[1];

    if (m_UseCostMap && !m_CostMap.empty())
    {
      std::map<int, int>::iterator end = m_CostMap.end();
      std::map<int, int>::iterator last = --(m_CostMap.end());

      // current position
      std::map<int, int>::iterator x;
      // std::map< int, int >::key_type keyOfX = static_cast<std::map< int, int >::key_type>(gradientMagnitude * 1000);
      int keyOfX = static_cast<int>(gradientMagnitude /* ShortestPathCostFunctionLiveWire::MAPSCALEFACTOR*/);
      x = m_CostMap.find(keyOfX);

      std::map<int, int>::iterator left2;
      std::map<int, int>::iterator left1;
      std::map<int, int>::iterator right1;
      std::map<int, int>::iterator right2;

      if (x == end)
      { // x can also be == end if the key is not in the map but between two other keys
        // search next key within map from x upwards
        right1 = m_CostMap.lower_bound(keyOfX);
      }
      else
      {
        right1 = x;
      }

      if (right1 == end || right1 == last)
      {
        right2 = end;
      }
      else //( right1 != (end-1) )
      {
        auto temp = right1;
        right2 = ++right1; // rght1 + 1
        right1 = temp;
      }

      if (right1 == m_CostMap.begin())
      {
        left1 = end;
        left2 = end;
      }
      else if (right1 == (++(m_CostMap.begin())))
      {
        auto temp = right1;
        left1 = --right1; // rght1 - 1
        right1 = temp;
        left2 = end;
      }
      else
      {
        auto temp = right1;
        left1 = --right1; // rght1 - 1
        left2 = --right1; // rght1 - 2
        right1 = temp;
      }

      double partRight1, partRight2, partLeft1, partLeft2;
      partRight1 = partRight2 = partLeft1 = partLeft2 = 0.0;

      /*
      f(x) = v(bin) * e^ ( -1/2 * (|x-k(bin)| / sigma)^2 )

      gaussian approximation

      where
      v(bin) is the value in the map
      k(bin) is the key
      */

      if (left2 != end)
      {
        partLeft2 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, left2->first, left2->second);
      }

      if (left1 != end)
      {
        partLeft1 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, left1->first, left1->second);
      }

      if (right1 != end)
      {
        partRight1 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, right1->first, right1->second);
      }

      if (right2 != end)
      {
        partRight2 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, right2->first, right2->second);
      }

      if (m_MaxMapCosts > 0.0)
      {
        gradientCost = 1.0 - ((partRight1 + partRight2 + partLeft1 + partLeft2) / m_MaxMapCosts);
      }
      else
      { // use linear mapping
        gradientCost = 1.0 - (gradientMagnitude / m_GradientMax);
      }
    }
    else
    { // use linear mapping
      // value between 0 (good) and 1 (bad)
      gradientCost = 1.0 - (gradientMagnitude / m_GradientMax);
    }

    //  Laplacian zero crossing costs
    // f(p) =     0;   if I(p)=0
    //     or     1;   if I(p)!=0
    double laplacianCost;
    typename Superclass::PixelType laplaceImageValue;

    laplaceImageValue = m_EdgeImage->GetPixel(p2);

    if (laplaceImageValue < 0 || laplaceImageValue > 0)
    {
      laplacianCost = 1.0;
    }
    else
    {
      laplacianCost = 0.0;
    }

    // gradient vector at p1
    double nGradientAtP1[2];
    nGradientAtP1[0] = gradientX; // previously computed for gradient magnitude
    nGradientAtP1[1] = gradientY;

    // gradient direction unit vector of p1
    nGradientAtP1[0] /= gradientMagnitude;
    nGradientAtP1[1] /= gradientMagnitude;
    //-------

    // gradient vector at p1
    double nGradientAtP2[2];

    nGradientAtP2[0] = m_GradientImage->GetPixel(p2)[0];
    nGradientAtP2[1] = m_GradientImage->GetPixel(p2)[1];

    nGradientAtP2[0] /= m_GradientMagnitudeImage->GetPixel(p2);
    nGradientAtP2[1] /= m_GradientMagnitudeImage->GetPixel(p2);

    double scalarProduct = (nGradientAtP1[0] * nGradientAtP2[0]) + (nGradientAtP1[1] * nGradientAtP2[1]);
    if (std::abs(scalarProduct) >= 1.0)
    {
      // this should probably not happen; make sure the input for acos is valid
      scalarProduct = 0.999999999;
    }

    double gradientDirectionCost = acos(scalarProduct) / 3.14159265;

    if (this->m_UseCostMap)
    {
      w1 = 0.43;
      w2 = 0.43;
      w3 = 0.14;
    }
    else
    {
      w1 = 0.10;
      w2 = 0.85;
      w3 = 0.05;
    }
    costs = w1 * laplacianCost + w2 * gradientCost + w3 * gradientDirectionCost;

    // scale by euclidian distance
    double costScale;
    if (p1[0] == p2[0] || p1[1] == p2[1])
    {
      // horizontal or vertical neighbor
      costScale = 1.0;
    }
    else
    {
      // diagonal neighbor
      costScale = sqrt(2.0);
    }

    costs *= costScale;

    return costs;
  }

  template <class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>::GetMinCost()
  {
    return m_MinCosts;
  }

  template <class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>::Initialize()
  {
    if (!m_Initialized)
    {
      typedef itk::CastImageFilter<TInputImageType, FloatImageType> CastFilterType;
      typename CastFilterType::Pointer castFilter = CastFilterType::New();
      castFilter->SetInput(this->m_Image);

      // init gradient magnitude image
      typedef itk::GradientMagnitudeImageFilter<FloatImageType, FloatImageType> GradientMagnitudeFilterType;
      typename GradientMagnitudeFilterType::Pointer gradientFilter = GradientMagnitudeFilterType::New();
      gradientFilter->SetInput(castFilter->GetOutput());
      // gradientFilter->SetNumberOfThreads(4);
      // gradientFilter->GetOutput()->SetRequestedRegion(m_RequestedRegion);

      gradientFilter->Update();
      this->m_GradientMagnitudeImage = gradientFilter->GetOutput();

      typedef itk::StatisticsImageFilter<FloatImageType> StatisticsImageFilterType;
      typename StatisticsImageFilterType::Pointer statisticsImageFilter = StatisticsImageFilterType::New();
      statisticsImageFilter->SetInput(this->m_GradientMagnitudeImage);
      statisticsImageFilter->Update();

      m_GradientMax = statisticsImageFilter->GetMaximum();

      typedef itk::GradientImageFilter<FloatImageType> GradientFilterType;

      typename GradientFilterType::Pointer filter = GradientFilterType::New();
      // sigma is specified in millimeters
      // filter->SetSigma( 1.5 );
      filter->SetInput(castFilter->GetOutput());
      filter->Update();

      m_GradientImage = filter->GetOutput();

      // init zero crossings
      // typedef  itk::ZeroCrossingImageFilter< TInputImageType, UnsignedCharImageType  > ZeroCrossingImageFilterType;
      // ZeroCrossingImageFilterType::Pointer zeroCrossingImageFilter = ZeroCrossingImageFilterType::New();
      // zeroCrossingImageFilter->SetInput(this->m_Image);
      // zeroCrossingImageFilter->SetBackgroundValue(1);
      // zeroCrossingImageFilter->SetForegroundValue(0);
      // zeroCrossingImageFilter->SetNumberOfThreads(4);
      // zeroCrossingImageFilter->Update();

      // m_EdgeImage = zeroCrossingImageFilter->GetOutput();

      // cast image to float to apply canny edge dection filter
      /*typedef itk::CastImageFilter< TInputImageType, FloatImageType > CastFilterType;
      CastFilterType::Pointer castFilter = CastFilterType::New();
      castFilter->SetInput(this->m_Image);*/

      // typedef itk::LaplacianImageFilter<FloatImageType, FloatImageType >  filterType;
      // filterType::Pointer laplacianFilter = filterType::New();
      // laplacianFilter->SetInput( castFilter->GetOutput() ); // NOTE: input image type must be double or float
      // laplacianFilter->Update();

      // m_EdgeImage = laplacianFilter->GetOutput();

      // init canny edge detection
      typedef itk::CannyEdgeDetectionImageFilter<FloatImageType, FloatImageType> CannyEdgeDetectionImageFilterType;
      typename CannyEdgeDetectionImageFilterType::Pointer cannyEdgeDetectionfilter =
        CannyEdgeDetectionImageFilterType::New();
      cannyEdgeDetectionfilter->SetInput(castFilter->GetOutput());
      cannyEdgeDetectionfilter->SetUpperThreshold(30);
      cannyEdgeDetectionfilter->SetLowerThreshold(15);
      cannyEdgeDetectionfilter->SetVariance(4);
      cannyEdgeDetectionfilter->SetMaximumError(.01f);

      cannyEdgeDetectionfilter->Update();
      m_EdgeImage = cannyEdgeDetectionfilter->GetOutput();

      // set minCosts
      m_MinCosts = 0.0; // The lower, the more thouroughly! 0 = dijkstra. If estimate costs are lower than actual costs
                      // everything is fine. If estimation is higher than actual costs, you might not get the shortest
                      // but a different path.

      m_Initialized = true;
    }

    // check start/end point value
    startValue = this->m_Image->GetPixel(this->m_StartIndex);
    endValue = this->m_Image->GetPixel(this->m_EndIndex);
  }

  template <class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>::SigmoidFunction(
    double I, double max, double min, double alpha, double beta)
  {
    // Using the SIgmoid formula from ITK Software Guide 6.3.2 Non Linear Mappings
    double Exponent = -1 * ((I - beta) / alpha);
    double Factor = 1 / (1 + exp(Exponent));
    double newI = (max - min) * Factor + min;

    return newI;
  }

  template <class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(double x, double xOfGaussian, double yOfGaussian)
  {
    return yOfGaussian * exp(-0.5 * pow((x - xOfGaussian), 2));
  }

} // end namespace itk

#endif // __itkShortestPathCostFunctionLiveWire_txx
