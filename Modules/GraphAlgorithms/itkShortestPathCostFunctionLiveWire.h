#ifndef __itkShortestPathCostFunctionLiveWire_h
#define __itkShortestPathCostFunctionLiveWire_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkShortestPathCostFunction.h" // Superclass of Metrics

#include <itkImageRegionConstIterator.h>

#include <itkGradientMagnitudeImageFilter.h>
#include <itkLaplacianImageFilter.h>


namespace itk
{

  template <class TInputImageType>
  class ITK_EXPORT ShortestPathCostFunctionLiveWire : public ShortestPathCostFunction<TInputImageType>
  {
  public:
    /** Standard class typedefs. */
    typedef ShortestPathCostFunctionLiveWire                Self;
    typedef ShortestPathCostFunction<TInputImageType>       Superclass;
    typedef SmartPointer<Self>                              Pointer;
    typedef SmartPointer<const Self>                        ConstPointer;    
    typedef itk::ImageRegionConstIterator<TInputImageType>  ConstIteratorType;


    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(ShortestPathCostFunctionLiveWire, ShortestPathCostFunction);


    typedef typename TInputImageType::IndexType             IndexType;
    typedef TInputImageType                                 ImageType;
    typedef itk::ImageRegion<2>                             RegionType;


    // \brief calculates the costs for going from p1 to p2
    virtual double GetCost(IndexType p1, IndexType p2);

    // \brief returns the minimal costs possible (needed for A*)
    virtual double GetMinCost();

    // \brief Initialize the metric
    virtual void Initialize ();


    // \brief Set repulsive path
    virtual void AddRepulsivePoint( itk::Index<3>  );

    // \brief Clear repulsive path
    virtual void ClearRepulsivePoints(  );



    ShortestPathCostFunctionLiveWire();

    // Set/Get function for sigma parameter
    itkSetMacro (Sigma, double);
    itkGetMacro (Sigma, double);

    itkSetMacro (RequestedRegion, RegionType);
    itkGetMacro (RequestedRegion, RegionType);

    // Set/Get function for sigma parameter
    itkSetMacro (UseApproximateGradient, bool);
    itkGetMacro (UseApproximateGradient, bool);

  protected:

    virtual ~ShortestPathCostFunctionLiveWire() {};

    double m_Sigma;

    typename ImageType::Pointer m_GradImage;
    typename ImageType::Pointer m_LaplacianImage;

    double iDifference;
    double pSmoothness;
    double sIDifference;

    double minCosts;

    bool m_UseRepulsivePoint;

    std::vector<  itk::Index<3>  > m_RepulsivePoints;

    //typename Superclass::PixelType a,b, a2, b2;
    typename Superclass::PixelType val;

    typename Superclass::PixelType startValue;
    typename Superclass::PixelType endValue;

    double m_GradientMax;

    RegionType m_RequestedRegion;

    bool m_UseApproximateGradient;

  private:

    double SigmoidFunction(double I, double max, double min, double alpha, double beta);


  };

} // end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShortestPathCostFunctionLiveWire.txx"
#endif

#endif /* __itkShortestPathCostFunctionLiveWire_h */
