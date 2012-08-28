#ifndef __itkShortestPathCostFunctionLiveWire_txx
#define __itkShortestPathCostFunctionLiveWire_txx

#include "itkShortestPathCostFunctionLiveWire.h"

#include <math.h>

#include <itkLaplacianOperator.h>


namespace itk
{ 
  // Constructor
  template<class TInputImageType>
  ShortestPathCostFunctionLiveWire<TInputImageType>
    ::ShortestPathCostFunctionLiveWire()
  {
    SetSigma (5.0); // standard value
    m_UseRepulsivePoint = false;
    m_UseApproximateGradient = true;
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


    /* ++++++++++++++++++++ GradientMagnitude costs ++++++++++++++++++++++++++*/
    
    if( m_UseApproximateGradient)
    {
      //approximate gradient magnitude

      // dI(x,y)/dx
      IndexType x1;
      x1[0] = (p1[0] == xMAX) ? (p1[0]) :( p1[0] +1);//check for pixels at the edge of the image => x==xMAX
      x1[1] = p1[1];

      IndexType x2;
      x2[0] = (p1[0] == 0) ? (0) :( p1[0] -1);//x==0
      x2[1] = p1[1];

      double gradientX = (this->m_Image->GetPixel(x1) - this->m_Image->GetPixel(x2)) / 2;


      // dI(x,y)/dy
      IndexType y1;
      y1[0] = p1[0];
      y1[1] = (p1[1] == yMAX) ? (p1[1]) :( p1[1] +1);//y==yMAX

      IndexType y2;
      y2[0] = p1[0];
      y2[1] = (p1[1] == 0) ? (0) :( p1[1] -1);//y==0

      double gradientY = (this->m_Image->GetPixel(y1) - this->m_Image->GetPixel(y2)) / 2;


      // sqrt (x^2 + y^2)
      val = sqrt( ( pow( gradientX, 2) + pow( gradientY, 2) ) );
      
    }
    else
    {
      val = this->m_GradImage->GetPixel(p2);
    }

    //scale by euclidian distance
    double gradientScale;
    if( p1[0] == p2[0] || p1[1] == p2[1])
    {
      //horizontal or vertical neighbor
      gradientScale = 1 / sqrt(2.0);
    }
    else
    {
      //diagonal neighbor
      gradientScale = 1.0;
    }

    // iDIfference is value between 0 (good) and 1 (bad)
    iDifference = 1 - val / 255;
    iDifference *= gradientScale;
    
    // double iDifferenceSig = SigmoidFunction(iDifference,1.0,0.0,23, 70);  

    /* -----------------------------------------------------------------------------*/


    /* ++++++++++++++++++++ Laplacian zero crossing costs ++++++++++++++++++++++++++*/
    // f(p) =     0;   if I(p)=0
    //     or     1;   if I(p)!=0
    double laplacianCost;
    Superclass::PixelType laplaceImageValue;

    if(!m_UseApproximateGradient)
    {
      laplaceImageValue = this->m_LaplacianImage->GetPixel(p2);
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
      currentIndex[0] = p1[0];
      currentIndex[1] = (p1[1] == yMAX) ? (p1[1]) :( p1[1] +1);//y==yMAX ?

      up = this->m_Image->GetPixel(currentIndex);


      // down
      currentIndex[0] = p1[0];
      currentIndex[1] = (p1[1] == 0) ? (0) :( p1[1] -1);//y==0 ?

      down = this->m_Image->GetPixel(currentIndex);


      // left
      currentIndex[0] = (p1[0] == 0) ? (0) :( p1[0] -1);//x==0 ?
      currentIndex[1] = p1[1];

      left = this->m_Image->GetPixel(currentIndex);


      // right
      currentIndex[0] = (p1[0] == xMAX) ? (p1[0]) :( p1[0] +1);//x==xMAX ?
      currentIndex[1] = p1[1];

      right = this->m_Image->GetPixel(currentIndex);

      //convolution
      laplaceImageValue = -4.0 * this->m_Image->GetPixel(p1) + up + down + right + left;
    }

    if(laplaceImageValue < 0 || laplaceImageValue > 0)
    {
      laplacianCost = 1;
    }
    else
    {
      laplacianCost = 0;
    }

    /* -----------------------------------------------------------------------------*/


    /* ++++++++++++++++++++ Gradient direction costs ++++++++++++++++++++++++++*/
    //double gradientDirectionCost = M_1_PI * ( 1/ cos(foo) + 1/ cos(bar) );
    /*------------------------------------------------------------------------*/


    /*+++++++++++++++++++++  local component costs +++++++++++++++++++++++++++*/
    /*weights*/
    double w1 = 0.43;
    double w2 = 0.43;
    double w3 = 0.14;

    double costs = w1 * laplacianCost + w2 * iDifference;


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

    typedef  itk::GradientMagnitudeImageFilter< typename TInputImageType, typename TInputImageType> GradientMagnitudeFilterType;
    //typedef  itk::LaplacianImageFilter< typename TInputImageType, typename TInputImageType > LaplacianFilterType;

    if( !m_UseApproximateGradient)
    {
      // init gradient magnitude image
      typename GradientMagnitudeFilterType::Pointer gradientFilter = GradientMagnitudeFilterType::New();
      gradientFilter->SetInput(this->m_Image);
      gradientFilter->GetOutput()->SetRequestedRegion(m_RequestedRegion);

      gradientFilter->Update();
      m_GradImage = gradientFilter->GetOutput();

      //typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
      //laplacianFilter->SetInput( this->m_Image );
      ////laplacianFilter->GetOutput()->SetRequestedRegion(m_RequestedRegion);

      //laplacianFilter->Update();

      //m_LaplacianImage = laplacianFilter->GetOutput();//typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
      //laplacianFilter->SetInput( this->m_Image );
      ////laplacianFilter->GetOutput()->SetRequestedRegion(m_RequestedRegion);

      //laplacianFilter->Update();

      //m_LaplacianImage = laplacianFilter->GetOutput();
    }//else no filter just approximate the gradient during cost estimation




    // check start/end point value 
    startValue= this->m_Image->GetPixel(this->m_StartIndex);
    endValue= this->m_Image->GetPixel(this->m_EndIndex);  

    // set minCosts
    minCosts = 0.3; // The lower, the more thouroughly! 0 = dijkstra. If estimate costs are lower than actual costs everything is fine. If estimation is higher than actual costs, you might not get the shortest but a different path.
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



} // end namespace itk

#endif // __itkShortestPathCostFunctionLiveWire_txx
