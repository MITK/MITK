#ifndef __itkShortestPathCostFunctionLiveWire_txx
#define __itkShortestPathCostFunctionLiveWire_txx

#include "itkShortestPathCostFunctionLiveWire.h"

#include <math.h>

#include <itkStatisticsImageFilter.h>
#include <itkZeroCrossingImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkGradientImageFilter.h>
#include <itkGradientRecursiveGaussianImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkLaplacianImageFilter.h>


namespace itk
{

  // Constructor
  template<class TInputImageType>
  ShortestPathCostFunctionLiveWire<TInputImageType>
    ::ShortestPathCostFunctionLiveWire()
  {
    m_UseRepulsivePoint = false;
    m_UseApproximateGradient = false;
    m_GradientMax = 0.0;
    m_Initialized = false;
    m_UseCostMap = false;
    m_MaxMapCosts = -1.0;
  }



  template<class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>
    ::AddRepulsivePoint(  itk::Index<3>  c )
  { 
    m_RepulsivePoints.push_back(c);
    m_UseRepulsivePoint = true;
  }



  template<class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>
    ::ClearRepulsivePoints()
  { 
    m_RepulsivePoints.clear();
    m_UseRepulsivePoint = false;
  }



  template<class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>
    ::GetCost(IndexType p1 ,IndexType  p2)
  {   

    unsigned long xMAX = this->m_Image->GetLargestPossibleRegion().GetSize()[0];
    unsigned long yMAX = this->m_Image->GetLargestPossibleRegion().GetSize()[1];

    double gradientX, gradientY;
    gradientX = gradientY = 0.0;

    double gradientCost;

    double gradientMagnitude;

    /* ++++++++++++++++++++ GradientMagnitude costs ++++++++++++++++++++++++++*/

    if( m_UseApproximateGradient)
    {
      //approximate gradient magnitude

      // dI(x,y)/dx
      IndexType x1;
      x1[0] = (p2[0] == xMAX) ? (p2[0]) :( p2[0] +1);//check for pixels at the edge of the image => x==xMAX
      x1[1] = p2[1];

      IndexType x2;
      x2[0] = (p2[0] == 0) ? (0) :( p2[0] -1);//x==0
      x2[1] = p2[1];

      gradientX = (this->m_Image->GetPixel(x1) - this->m_Image->GetPixel(x2)) / 2;


      // dI(x,y)/dy
      IndexType y1;
      y1[0] = p2[0];
      y1[1] = (p2[1] == yMAX) ? (p2[1]) :( p2[1] +1);//y==yMAX

      IndexType y2;
      y2[0] = p2[0];
      y2[1] = (p2[1] == 0) ? (0) :( p2[1] -1);//y==0

      gradientY = (this->m_Image->GetPixel(y1) - this->m_Image->GetPixel(y2)) / 2;


      // sqrt (x^2 + y^2)
      gradientMagnitude = sqrt( ( pow( gradientX, 2) + pow( gradientY, 2) ) );

    }
    else
    {
      gradientMagnitude = this->m_GradientMagnImage->GetPixel(p2);
      gradientX = m_GradientImage->GetPixel(p2)[0];
      gradientY = m_GradientImage->GetPixel(p2)[1];
    }


    if(m_UseCostMap && !m_CostMap.empty())
    {
      std::map< int, int >::iterator end = m_CostMap.end();
      std::map< int, int >::iterator last = --(m_CostMap.end());

      //current position
      std::map< int, int >::iterator x;
      //std::map< int, int >::key_type keyOfX = static_cast<std::map< int, int >::key_type>(gradientMagnitude * 1000);
      int keyOfX = static_cast<int >(gradientMagnitude * ShortestPathCostFunctionLiveWire::MAPSCALEFACTOR);
      x = m_CostMap.find( keyOfX );

      std::map< int, int >::iterator left2;
      std::map< int, int >::iterator left1;
      std::map< int, int >::iterator right1;
      std::map< int, int >::iterator right2;

      if( x == end )
      {//x can also be == end if the key is not in the map but between two other keys
        //search next key within map from x upwards
        right1 = m_CostMap.lower_bound( keyOfX );
      }
      else
      {
        right1 = x;       
      }

      if(right1 == end || right1 == last )
      {
        right2 = end;   
      }
      else//( right1 != (end-1) )
      {
        std::map< int, int >::iterator temp = right1;
        right2 = ++right1;//rght1 + 1
        right1 = temp;
      }


      if( right1 == m_CostMap.begin() )
      {
        left1 = end;
        left2 = end;
      }
      else if( right1 == (++(m_CostMap.begin())) )
      {
        std::map< int, int >::iterator temp = right1;
        left1  = --right1;//rght1 - 1
        right1 = temp;
        left2 = end;
      }
      else
      {
        std::map< int, int >::iterator temp = right1;
        left1  = --right1;//rght1 - 1
        left2 = --right1;//rght1 - 2
        right1 = temp;
      }

      double partRight1, partRight2, partLeft1, partLeft2;
      partRight1 = partRight2 = partLeft1 = partLeft2 = 0.0;


      /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      f(x) = v(bin) * e^ ( -1/2 * (|x-k(bin)| / sigma)^2 )

      gaussian approximation

      where
      v(bin) is the value in the map
      k(bin) is the key
      */
      if( left2 != end )
      {
        partLeft2 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, left2->first, left2->second);
      }

      if( left1 != end )
      {
        partLeft1 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, left1->first, left1->second);
      }

      if( right1 != end )
      {
        partRight1 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, right1->first, right1->second);
      }

      if( right2 != end )
      {
        partRight2 = ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(keyOfX, right2->first, right2->second);
      }
      /*----------------------------------------------------------------------------*/


      if( m_MaxMapCosts > 0.0 )
      {
        gradientCost = 1.0 - ( (partRight1 + partRight2 + partLeft1 + partLeft2) / m_MaxMapCosts );
      }
      else
      {//use linear mapping
        gradientCost = 1.0 - (gradientMagnitude / m_GradientMax);
      }

    }
    else
    {//use linear mapping
      //value between 0 (good) and 1 (bad)
      gradientCost = 1.0 - (gradientMagnitude / m_GradientMax);

    }
    /* -----------------------------------------------------------------------------*/



    /* ++++++++++++++++++++ Laplacian zero crossing costs ++++++++++++++++++++++++++*/
    // f(p) =     0;   if I(p)=0
    //     or     1;   if I(p)!=0
    double laplacianCost;
    Superclass::PixelType laplaceImageValue;

    if(!m_UseApproximateGradient)
    {
      //laplacianCost = this->m_ZeroCrossingsImage->GetPixel(p2);

      laplaceImageValue = m_EdgeImage->GetPixel(p2); 

      if(laplaceImageValue < 0 || laplaceImageValue > 0)
      {
        laplacianCost = 1.0;
      }
      else
      {
        laplacianCost = 0.0;
      }
    }
    else
    {
      //compute pointwise with laplacian kernel
      // 0  1  0
      // 1 -4  1
      // 0  1  0

      Superclass::PixelType up, down, left, right;
      IndexType currentIndex;

      // up 
      currentIndex[0] = p2[0];
      currentIndex[1] = (p2[1] == yMAX) ? (p2[1]) :( p2[1] +1);//y==yMAX ?

      up = this->m_Image->GetPixel(currentIndex);


      // down
      currentIndex[0] = p2[0];
      currentIndex[1] = (p2[1] == 0) ? (0) :( p2[1] -1);//y==0 ?

      down = this->m_Image->GetPixel(currentIndex);


      // left
      currentIndex[0] = (p2[0] == 0) ? (0) :( p2[0] -1);//x==0 ?
      currentIndex[1] = p2[1];

      left = this->m_Image->GetPixel(currentIndex);


      // right
      currentIndex[0] = (p2[0] == xMAX) ? (p2[0]) :( p2[0] +1);//x==xMAX ?
      currentIndex[1] = p2[1];

      right = this->m_Image->GetPixel(currentIndex);

      //convolution
      laplaceImageValue = -4.0 * this->m_Image->GetPixel(p2) + up + down + right + left;

      if(laplaceImageValue < 0 || laplaceImageValue > 0)
      {
        laplacianCost = 1.0;
      }
      else
      {
        laplacianCost = 0.0;
      }
    }

    /* -----------------------------------------------------------------------------*/



    /* ++++++++++++++++++++ Gradient direction costs ++++++++++++++++++++++++++*/
    //vector q-p   i.e. p2-p1
    double vQP[2];
    vQP[0] = p2[0] - p1[0];
    vQP[1] = p2[1] - p1[1];
    //-------

    //vector p-q   i.e. p1-p2
    double vPQ[2];
    vPQ[0] = p1[0] - p2[0];
    vPQ[1] = p1[1] - p2[1];
    //-------

    // gradient vector at p1
    double nGradientAtP1[2];
    nGradientAtP1[0] = gradientX;//previously computed for gradient magnitude
    nGradientAtP1[1] = gradientY;

    //gradient direction unit vector of p1
    nGradientAtP1[0] /= gradientMagnitude;
    nGradientAtP1[1] /= gradientMagnitude;
    //-------

    // gradient vector at p1
    double nGradientAtP2[2];

    if(m_UseApproximateGradient)
    {
      // dI(x,y)/dx
      IndexType x1;
      x1[0] = (p2[0] == xMAX) ? (p1[0]) :( p2[0] +1);//check for pixels at the edge of the image => x==xMAX
      x1[1] = p2[1];

      IndexType x2;
      x2[0] = (p2[0] == 0) ? (0) :( p2[0] -1);//x==0
      x2[1] = p2[1];
      gradientX = (this->m_Image->GetPixel(x1) - this->m_Image->GetPixel(x2)) / 2;

      // dI(x,y)/dy
      IndexType y1;
      y1[0] = p2[0];
      y1[1] = (p2[1] == yMAX) ? (p2[1]) :( p2[1] +1);//y==yMAX

      IndexType y2;
      y2[0] = p2[0];
      y2[1] = (p2[1] == 0) ? (0) :( p2[1] -1);//y==0
      gradientY = (this->m_Image->GetPixel(y1) - this->m_Image->GetPixel(y2)) / 2;

      nGradientAtP2[0] = gradientX;
      nGradientAtP2[1] = gradientY;

      gradientMagnitude = sqrt( nGradientAtP2[0] * nGradientAtP2[0] + nGradientAtP2[1] * nGradientAtP2[1]);

      //gradient direction unit vector of p2
      nGradientAtP2[0] /= gradientMagnitude;
      nGradientAtP2[1] /= gradientMagnitude;
      //--------

    }
    else
    {
      nGradientAtP2[0] = m_GradientImage->GetPixel(p2)[0];
      nGradientAtP2[1] = m_GradientImage->GetPixel(p2)[1];

      nGradientAtP2[0] /= m_GradientMagnImage->GetPixel(p2);
      nGradientAtP2[1] /= m_GradientMagnImage->GetPixel(p2);

    }

    double scalarProduct = (nGradientAtP1[0] * nGradientAtP2[0]) + (nGradientAtP1[1] * nGradientAtP2[1]);
    if( abs(scalarProduct) >= 1.0)
    {
      //this should probably not happen; make sure the input for acos is valid
      scalarProduct = 0.999999999;
    }

    double gradientDirectionCost = acos( scalarProduct ) / 3.14159265;
    /*------------------------------------------------------------------------*/


    /*+++++++++++++++++++++ intensity difference costs +++++++++++++++++++++++++++*/
    double a,b,intensityDifferenceCost;    

    a = this->m_Image->GetPixel(p1);

    b = this->m_Image->GetPixel(p2);
    intensityDifferenceCost = pow((b-a),2) / SHRT_MAX;


    //intensityDifferenceCost = SigmoidFunction(intensityDifferenceCost,1.0,0.0,10,20);
    /*------------------------------------------------------------------------*/


    /*+++++++++++++++++++++  local component costs +++++++++++++++++++++++++++*/
    /*weights*/
    double w1 = 0.40;
    double w2 = 0.43;
    double w3 = 0.05;
    double w4 = 0.12;

    double costs = w1 * laplacianCost + w2 * gradientCost /*+ w3 * intensityDifferenceCost*/ + w4 * gradientDirectionCost;


    //scale by euclidian distance
    double costScale;
    if( p1[0] == p2[0] || p1[1] == p2[1])
    {
      //horizontal or vertical neighbor
      costScale = 1.0;
    }
    else
    {
      //diagonal neighbor
      costScale = sqrt(2.0);
    }

    costs *= costScale;

    return costs;
  }



  template<class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>
    ::GetMinCost()
  { 
    return minCosts; 
  }



  template<class TInputImageType>
  void ShortestPathCostFunctionLiveWire<TInputImageType>
    ::Initialize()
  {
    if(!m_Initialized)
    {

      if( !m_UseApproximateGradient)
      {
        // init gradient magnitude image
        typedef  itk::GradientMagnitudeImageFilter< typename TInputImageType, typename TInputImageType> GradientMagnitudeFilterType;
        typename GradientMagnitudeFilterType::Pointer gradientFilter = GradientMagnitudeFilterType::New();
        gradientFilter->SetInput(this->m_Image);
        //gradientFilter->SetNumberOfThreads(4);
        //gradientFilter->GetOutput()->SetRequestedRegion(m_RequestedRegion);

        gradientFilter->Update();
        m_GradientMagnImage = gradientFilter->GetOutput();

        typedef itk::StatisticsImageFilter<TInputImageType> StatisticsImageFilterType;
        StatisticsImageFilterType::Pointer statisticsImageFilter = StatisticsImageFilterType::New();
        statisticsImageFilter->SetInput(this->m_GradientMagnImage);
        statisticsImageFilter->Update();

        m_GradientMax = statisticsImageFilter->GetMaximum();



        //Filter class is instantiated
        /*typedef itk::GradientRecursiveGaussianImageFilter<TInputImageType, VectorOutputImageType> GradientFilterType;*/

        typedef itk::GradientImageFilter< TInputImageType >  GradientFilterType;

        GradientFilterType::Pointer filter = GradientFilterType::New();
        //sigma is specified in millimeters
        //filter->SetSigma( 1.5 );
        filter->SetInput(this->m_Image); 
        filter->Update();

        m_GradientImage = filter->GetOutput();


        // init zero crossings
        //typedef  itk::ZeroCrossingImageFilter< TInputImageType, UnsignedCharImageType  > ZeroCrossingImageFilterType;
        //ZeroCrossingImageFilterType::Pointer zeroCrossingImageFilter = ZeroCrossingImageFilterType::New();
        //zeroCrossingImageFilter->SetInput(this->m_Image);
        //zeroCrossingImageFilter->SetBackgroundValue(1);
        //zeroCrossingImageFilter->SetForegroundValue(0);
        //zeroCrossingImageFilter->SetNumberOfThreads(4);
        //zeroCrossingImageFilter->Update();

        //m_ZeroCrossingsImage = zeroCrossingImageFilter->GetOutput();


        //cast image to float to apply canny edge dection filter
        typedef itk::CastImageFilter< TInputImageType, FloatImageType > CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(this->m_Image);

        typedef itk::LaplacianImageFilter<FloatImageType, FloatImageType >  filterType;
        filterType::Pointer laplacianFilter = filterType::New();
        laplacianFilter->SetInput( castFilter->GetOutput() ); // NOTE: input image type must be double or float
        laplacianFilter->Update();

        m_EdgeImage = laplacianFilter->GetOutput();

        ////init canny edge detection
        //typedef itk::CannyEdgeDetectionImageFilter<FloatImageType, FloatImageType> CannyEdgeDetectionImageFilterType;
        //CannyEdgeDetectionImageFilterType::Pointer cannyEdgeDetectionfilter = CannyEdgeDetectionImageFilterType::New();
        //cannyEdgeDetectionfilter->SetInput(castFilter->GetOutput());
        //cannyEdgeDetectionfilter->SetUpperThreshold(30);
        //cannyEdgeDetectionfilter->SetLowerThreshold(15);
        //cannyEdgeDetectionfilter->SetVariance(statisticsImageFilter->GetSigma());
        //cannyEdgeDetectionfilter->SetMaximumError(.01f);

        //cannyEdgeDetectionfilter->Update();
        //m_EdgeImage = cannyEdgeDetectionfilter->GetOutput();

      }//else no filter just approximate the gradient during cost estimation







      // set minCosts
      minCosts = 0.0; // The lower, the more thouroughly! 0 = dijkstra. If estimate costs are lower than actual costs everything is fine. If estimation is higher than actual costs, you might not get the shortest but a different path.

      m_Initialized = true;
    }

    // check start/end point value 
    startValue= this->m_Image->GetPixel(this->m_StartIndex);
    endValue= this->m_Image->GetPixel(this->m_EndIndex); 
  }



  template<class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>::SigmoidFunction(double I, double max, double min, double alpha, double beta)
  {
    // Using the SIgmoid formula from ITK Software Guide 6.3.2 Non Linear Mappings
    double Exponent = -1 * ((I - beta) / alpha);
    double Factor = 1 / (1 + exp(Exponent));
    double newI = (max - min) * Factor + min;

    return newI;
  }


  template<class TInputImageType>
  double ShortestPathCostFunctionLiveWire<TInputImageType>::Gaussian(double x, double xOfGaussian, double yOfGaussian)
  {
    return yOfGaussian * exp( -0.5 * pow( ( abs(x - xOfGaussian) / (2.0 * ShortestPathCostFunctionLiveWire<TInputImageType>::MAPSCALEFACTOR) ), 2) );
  }

} // end namespace itk

#endif // __itkShortestPathCostFunctionLiveWire_txx
