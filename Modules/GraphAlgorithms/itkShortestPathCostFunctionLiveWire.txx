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
    m_UseApproximateGradient = false;
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
     
    /* ++++++++++++++++++++ GradientMagnitude costs ++++++++++++++++++++++++++*/
    
    if( m_UseApproximateGradient)
    {
      //approximate gradient magnitude

      // dI(x,y)/dx
      IndexType x1;
      x1[0] = p1[0] +1;
      x1[1] = p1[1];

      IndexType x2;
      x2[0] = p1[0] -1;
      x2[1] = p1[1];

      double gradientX = (this->m_Image->GetPixel(x1) - this->m_Image->GetPixel(x2)) / 2;


      // dI(x,y)/dy
      IndexType y1;
      y1[0] = p1[0];
      y1[1] = p1[1] +1;

      IndexType y2;
      y2[0] = p1[0];
      y2[1] = p1[1] -1;

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
    double laplaceImageValue = this->m_LaplacianImage->GetPixel(p2);
    if(laplaceImageValue < 0 || laplaceImageValue > 0)
    {
      laplacianCost = 1;
    }
    else
    {
      laplacianCost = 0;
    }

    /* -----------------------------------------------------------------------------*/


    ///////////////////
    // REPULSIVE COSTS 
    // check repulsive path energy       
    double repulsiveCosts = 0;
    double repulsiveCostsSig = 0;
    if ( m_UseRepulsivePoint )
    {
       // Find the nearest Repulsive Point
       repulsiveCosts = -1;
       for (int i=0; i<m_RepulsivePoints.size(); i++)
       {
          itk::Vector<float,3> vec;
          vec[0] = m_RepulsivePoints[i][0] - p2[0];
          vec[1] = m_RepulsivePoints[i][1] - p2[1];
          vec[2] = m_RepulsivePoints[i][2] - p2[2];

          if (( vec.GetNorm() > repulsiveCosts) || (repulsiveCosts == -1))
            repulsiveCosts = vec.GetNorm();
       }

       // repulsiveCosts is value between 0 (bad) and ImageX*ImageY*ImageZ (~100) (Good)
       // mean = 50 .. var= 60
       repulsiveCostsSig = SigmoidFunction(repulsiveCosts,1.0,0.0,-20,20);  
    }


    /*+++++++++++++++++++++  local component costs +++++++++++++++++++++++++++*/
    /*weights*/
    double w1 = 0.43;
    double w2 = 0.43;
    double w3 = 0.14;

    double costs = w1 * laplacianCost + w2 * iDifference;

    //if (m_UseRepulsivePoint)       
    //   costs = (iDifferenceSig * 0.5) + (repulsiveCostsSig * 0.52); // das mit den repulsive costs muss mann nochmal überarbeiten
    //
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
    }//else no filter just approximate the gradient during cost estimation


    //typename LaplacianFilterType::Pointer laplacianFilter = LaplacianFilterType::New();
    //laplacianFilter->SetInput( this->m_Image );
    ////laplacianFilter->GetOutput()->SetRequestedRegion(m_RequestedRegion);

    //laplacianFilter->Update();

    //m_LaplacianImage = laplacianFilter->GetOutput();

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
