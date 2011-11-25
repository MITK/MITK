#ifndef __itkShortestPathCostFunctionTbss_txx
#define __itkShortestPathCostFunctionTbss_txx

#include "itkShortestPathCostFunctionTbss.h"
#include <limits>



namespace itk
{ 

  // Constructor
  template<class TInputImageType>
  ShortestPathCostFunctionTbss<TInputImageType>
    ::ShortestPathCostFunctionTbss()
  {
  }


 template<class TInputImageType>
  double ShortestPathCostFunctionTbss<TInputImageType>
    ::GetCost(IndexType p1 ,IndexType  p2)
  {
    // Very simple Metric:
    // The squared difference of both values is defined as cost

    double a,b,c;    
    ConstIteratorType it( this->m_Image, this->m_Image->GetRequestedRegion());
    
    it.SetIndex(p1);
    a = it.Get();
    it.SetIndex(p2);
    b = it.Get();


    if(a==b)
    {     
      double dxSqt = (p1[0]-p2[0]) * (p1[0]-p2[0]);// * 1000;
      double dySqt = (p1[1]-p2[1]) * (p1[1]-p2[1]);
      double dzSqt = (p1[2]-p2[2]) * (p1[2]-p2[2]);
      c = sqrt(dxSqt + dySqt + dzSqt);
    }
    else if(p1[0] != p2[0])
    {
      c=std::numeric_limits<double>::max( );
    }
    else
    {
      c=std::numeric_limits<double>::max( );
    }

    return c; 
  }


  template<class TInputImageType>
  void ShortestPathCostFunctionTbss<TInputImageType>
    ::Initialize()
  {
  }

  template<class TInputImageType>
  double ShortestPathCostFunctionMitral<TInputImageType>
    ::GetMinCost()
  {
    return 1.0;
  }




} // end namespace itk

#endif // __itkShortestPathCostFunctionSimple_txx
