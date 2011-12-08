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
    typedef ShortestPathCostFunctionTbss               Self;
    typedef ShortestPathCostFunction<TInputImageType>  Superclass;
    typedef SmartPointer<Self>                         Pointer;
    typedef SmartPointer<const Self>                   ConstPointer;
    typedef itk::ImageRegionConstIterator<TInputImageType>  ConstIteratorType;
    typedef typename TInputImageType::IndexType             IndexType;


    typedef itk::Image<float,3> FloatImageType;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(Self, Superclass);
   
    // \brief calculates the costs for going from p1 to p2
    virtual double GetCost( IndexType p1, IndexType p2);

    // \brief Initialize the metric
    virtual void Initialize ();

    // \brief returns the minimal costs possible (needed for A*)
    virtual double GetMinCost();




    void SetThreshold(double t)
    {
      m_Threshold = t;
    }


  protected:
    ShortestPathCostFunctionTbss();

    virtual ~ShortestPathCostFunctionTbss() {};

    double m_Threshold;


  private:




  };

} // end namespace itk

#include "itkShortestPathCostFunctionTbss.txx"

#endif /* __itkShortestPathCostFunctionTbss_h */
