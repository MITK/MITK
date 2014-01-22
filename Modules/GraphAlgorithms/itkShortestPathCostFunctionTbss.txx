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
#ifndef __itkShortestPathCostFunctionTbss_cpp
#define __itkShortestPathCostFunctionTbss_cpp


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
    // Cost function for tbss tract definition
    double c;

    if(this->m_Image->GetPixel(p2) < m_Threshold)
    {
      c = std::numeric_limits<double>::max( );
    }
    else
    {

        double dxSqt = (p1[0]-p2[0]) * (p1[0]-p2[0]);// * (p1[0]-p2[0]);
        double dySqt = (p1[1]-p2[1]) * (p1[1]-p2[1]);
        double dzSqt = (p1[2]-p2[2]) * (p1[2]-p2[2]);

        double weight = this->m_Image->GetPixel(p2);

        c = sqrt(dxSqt + dySqt + dzSqt) + 1000 * (1-weight);

    }


    return c;
  }


  template<class TInputImageType>
  void ShortestPathCostFunctionTbss<TInputImageType>
    ::Initialize()
  {
  }

  template<class TInputImageType>
  double ShortestPathCostFunctionTbss<TInputImageType>
   ::GetMinCost()
  {
    return 1;
  }




} // end namespace itk

#endif // __itkShortestPathCostFunctionSimple_txx
