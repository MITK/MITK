#ifndef __itkShortestPathCostFunctionLiveWire_h
#define __itkShortestPathCostFunctionLiveWire_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkShortestPathCostFunction.h" // Superclass of Metrics

#include <itkImageRegionConstIterator.h>




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


    typedef itk::Image<unsigned char, 2>  UnsignedCharImageType;
    typedef itk::Image<float, 2>  FloatImageType;

    typedef float ComponentType;
    typedef itk::CovariantVector< ComponentType, 2  > OutputPixelType;
    typedef itk::Image< OutputPixelType, 2 > VectorOutputImageType;

    typedef typename TInputImageType::IndexType             IndexType;
    typedef TInputImageType                                 ImageType;
    typedef itk::ImageRegion<2>                             RegionType;


    /** \brief calculates the costs for going from p1 to p2*/
    virtual double GetCost(IndexType p1, IndexType p2);

    /** \brief returns the minimal costs possible (needed for A*)*/
    virtual double GetMinCost();

    /** \brief Initialize the metric*/
    virtual void Initialize ();


    /** \brief Set repulsive path*/
    virtual void AddRepulsivePoint( itk::Index<3>  );

    /** \brief Clear repulsive path*/
    virtual void ClearRepulsivePoints(  );



    ShortestPathCostFunctionLiveWire();


    itkSetMacro (RequestedRegion, RegionType);
    itkGetMacro (RequestedRegion, RegionType);

    // Set/Get function for sigma parameter
    itkSetMacro (UseApproximateGradient, bool);
    itkGetMacro (UseApproximateGradient, bool);

    virtual void SetImage(const TInputImageType* _arg) 
    { 
      if (this->m_Image != _arg) 
      { 
        this->m_Image = _arg; 
        this->Modified(); 
        this->m_Initialized = false;
      } 
    } 

  protected:

    virtual ~ShortestPathCostFunctionLiveWire() {};


    typename ImageType::Pointer m_GradientMagnImage;
    typename UnsignedCharImageType::Pointer m_ZeroCrossingsImage;
    typename FloatImageType::Pointer m_EdgeImage;
    typename VectorOutputImageType::Pointer m_GradientImage;

    double minCosts;

    bool m_UseRepulsivePoint;

    std::vector<  itk::Index<3>  > m_RepulsivePoints;

    typename Superclass::PixelType val;

    typename Superclass::PixelType startValue;
    typename Superclass::PixelType endValue;

    double m_GradientMax;

    RegionType m_RequestedRegion;

    bool m_UseApproximateGradient;

    bool m_Initialized;

  private:

    double SigmoidFunction(double I, double max, double min, double alpha, double beta);


  };

} // end namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkShortestPathCostFunctionLiveWire.txx"
#endif

#endif /* __itkShortestPathCostFunctionLiveWire_h */
