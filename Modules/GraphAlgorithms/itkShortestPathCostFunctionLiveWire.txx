#ifndef __itkShortestPathCostFunctionLiveWire_txx
#define __itkShortestPathCostFunctionLiveWire_txx

#include "itkShortestPathCostFunctionLiveWire.h"

#include <math.h>

namespace itk
{ 
  // Constructor
  template<class TInputImageType>
  ShortestPathCostFunctionLiveWire<TInputImageType>
    ::ShortestPathCostFunctionLiveWire()
  {
    SetSigma (5.0); // standard value
    m_UseRepulsivePoint = false;
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
     ///////////////////
    // INTENSITY COSTS 
    // the whiter the better
   val = this->m_Image->GetPixel(p2);
    iDifference=255-val; 
    // iDIfference is value between 0 (good) and 255 (bad)
    double iDifferenceSig = SigmoidFunction(iDifference,1.0,0.0,23, 70);  


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


    ///////////////////
    // return costs
    double costs = iDifferenceSig;

    if (m_UseRepulsivePoint)       
       costs = (iDifferenceSig * 0.5) + (repulsiveCostsSig * 0.52); // das mit den repulsive costs muss mann nochmal überarbeiten
    
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
    // init gradient magnitude image
    typename GradientMagnitudeFilterType::Pointer GradientFilter = GradientMagnitudeFilterType::New();
    GradientFilter->SetInput(this->m_Image);
    //GradientFilter->SetSigma( 1.0 );
    //GradientFilter->SetNormalizeAcrossScale(true);
    GradientFilter->Update();
    m_GradImage = GradientFilter->GetOutput();

    // check start/end point value 
    startValue= this->m_Image->GetPixel(this->m_StartIndex);
    endValue= this->m_Image->GetPixel(this->m_EndIndex);  

    // set minCosts
    minCosts = 0.3; // The lower, the more thouroughly! 0 = dijkstra. If estimate costs are lower than actual costs everything is fine. If estimation is higher than actual costs, you might not get the shortest but a different path.
    if (m_UseRepulsivePoint)
       minCosts = 0.3;
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
