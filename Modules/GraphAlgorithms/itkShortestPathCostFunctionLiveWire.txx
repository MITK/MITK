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
      x1[0] = (p1[0] == xMAX) ? (p1[0]) :( p1[0] +1);//check for pixels at the edge of the image => x==xMAX
      x1[1] = p1[1];

      IndexType x2;
      x2[0] = (p1[0] == 0) ? (0) :( p1[0] -1);//x==0
      x2[1] = p1[1];

      gradientX = (this->m_Image->GetPixel(x1) - this->m_Image->GetPixel(x2)) / 2;


      // dI(x,y)/dy
      IndexType y1;
      y1[0] = p1[0];
      y1[1] = (p1[1] == yMAX) ? (p1[1]) :( p1[1] +1);//y==yMAX

      IndexType y2;
      y2[0] = p1[0];
      y2[1] = (p1[1] == 0) ? (0) :( p1[1] -1);//y==0

      gradientY = (this->m_Image->GetPixel(y1) - this->m_Image->GetPixel(y2)) / 2;


      // sqrt (x^2 + y^2)
      gradientMagnitude = sqrt( ( pow( gradientX, 2) + pow( gradientY, 2) ) );
      
    }
    else
    {
      gradientMagnitude = this->m_GradImage->GetPixel(p2);
    }


    // iDIfference is value between 0 (good) and 1 (bad)
    gradientCost = 1 - gradientMagnitude / 255;

    
    //double iDifferenceSig = SigmoidFunction(iDifference,1.0,0.0,23, 70);

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
    double pGradientP1[2];
    pGradientP1[0] = gradientX;//previously computed for gradient magnitude
    pGradientP1[1] = gradientY;

    //gradient direction unit vector of p1
    pGradientP1[0] /= gradientMagnitude;
    pGradientP1[1] /= gradientMagnitude;
    //-------

    // gradient vector at p1
    double pGradientP2[2];
    
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

    pGradientP2[0] = gradientX;
    pGradientP2[1] = gradientY;

    gradientMagnitude = sqrt( pGradientP2[0] * pGradientP2[0] + pGradientP2[1] * pGradientP2[1]);

    //gradient direction unit vector of p2
    pGradientP2[0] /= gradientMagnitude;
    pGradientP2[1] /= gradientMagnitude;
    //--------

    double gradientDirectionCost = acos( (pGradientP1[0] * pGradientP2[0]) + (pGradientP1[1] * pGradientP2[1]) ) / 3.14159265;
    /*------------------------------------------------------------------------*/


    /*+++++++++++++++++++++ intensity difference costs +++++++++++++++++++++++++++*/
    double a,b,intensityDifferenceCost;    

    a = this->m_Image->GetPixel(p1);

    b = this->m_Image->GetPixel(p2);
    intensityDifferenceCost = abs(b-a)/255;


    //intensityDifferenceCost = SigmoidFunction(intensityDifferenceCost,1.0,0.0,10,20);
    /*------------------------------------------------------------------------*/


    /*+++++++++++++++++++++  local component costs +++++++++++++++++++++++++++*/
    /*weights*/
    double w1 = 0.33;
    double w2 = 0.55;
    double w3 = 0.12;
    double w4 = 0.05;

    double costs = w1 * laplacianCost + w2 * gradientCost + w3 * intensityDifferenceCost + w4 * gradientDirectionCost;


    //scale by euclidian distance
    double costScale;
    if( p1[0] == p2[0] || p1[1] == p2[1])
    {
      //horizontal or vertical neighbor
      costScale = 1 / sqrt(2.0);
    }
    else
    {
      //diagonal neighbor
      costScale = 1.0;
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
    minCosts = 0.0; // The lower, the more thouroughly! 0 = dijkstra. If estimate costs are lower than actual costs everything is fine. If estimation is higher than actual costs, you might not get the shortest but a different path.
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
