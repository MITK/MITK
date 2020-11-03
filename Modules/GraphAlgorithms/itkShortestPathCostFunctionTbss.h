/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __itkShortestPathCostFunctionTbss_h
#define __itkShortestPathCostFunctionTbss_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkShapedNeighborhoodIterator.h"
#include "itkShortestPathCostFunction.h" // Superclass of Metrics
#include <itkImageRegionConstIterator.h>
#include <itkMacro.h>

namespace itk
{
  template <class TInputImageType>
  class ShortestPathCostFunctionTbss : public ShortestPathCostFunction<TInputImageType>
  {
  public:
    /** Standard class typedefs. */
    typedef ShortestPathCostFunctionTbss Self;
    typedef ShortestPathCostFunction<TInputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;
    typedef itk::ImageRegionConstIterator<TInputImageType> ConstIteratorType;
    typedef typename TInputImageType::IndexType IndexType;

    typedef itk::Image<float, 3> FloatImageType;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(Self, Superclass);

    // \brief calculates the costs for going from p1 to p2
    double GetCost(IndexType p1, IndexType p2) override;

    // \brief Initialize the metric
    void Initialize() override;

    // \brief returns the minimal costs possible (needed for A*)
    double GetMinCost() override;

    void SetThreshold(double t) { m_Threshold = t; }
  protected:
    ShortestPathCostFunctionTbss();

    ~ShortestPathCostFunctionTbss() override{};

    double m_Threshold;

  private:
  };

} // end namespace itk

#include "itkShortestPathCostFunctionTbss.txx"

#endif /* __itkShortestPathCostFunctionTbss_h */
