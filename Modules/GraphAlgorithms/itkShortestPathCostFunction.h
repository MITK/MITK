#ifndef __itkShortestPathCostFunction_h
#define __itkShortestPathCostFunction_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkShapedNeighborhoodIterator.h"
#include <itkImageRegionConstIterator.h>
 

namespace itk
{

  // \brief this is a pure virtual superclass for all cost function for the itkShortestPathImageFilter
  template <class TInputImageType>            
  class ShortestPathCostFunction : public Object
  {
  public:

    /** Standard class typedefs. */
    typedef ShortestPathCostFunction Self;
    typedef Object  Superclass;
    typedef SmartPointer<Self>   Pointer;
    typedef SmartPointer<const Self>  ConstPointer;
    typedef ShapedNeighborhoodIterator< TInputImageType > itkShapedNeighborhoodIteratorType;

	
    /** Run-time type information (and related methods). */
    itkTypeMacro(ShortestPathCostFunction, Object);

    /** Type definition for the input image. */
    typedef TInputImageType  ImageType;

    // More typdefs for convenience
    typedef typename TInputImageType::Pointer               ImagePointer;
    typedef typename TInputImageType::ConstPointer          ImageConstPointer;


    typedef typename TInputImageType::PixelType             PixelType;    

    typedef typename TInputImageType::IndexType             IndexType;

    /** Set the input image. */
    itkSetConstObjectMacro(Image,TInputImageType);

    // \brief calculates the costs for going from pixel1 to pixel2
    virtual double GetCost( IndexType p1, IndexType p2) = 0;

    // \brief returns the minimal costs possible (needed for A*)
    virtual double GetMinCost() = 0;

    // \brief Initialize the metric
    virtual void Initialize () = 0;

    // \brief Set Starpoint for Path
    void SetStartIndex (const IndexType & StartIndex);

    // \brief Set Endpoint for Path
    void SetEndIndex(const IndexType & EndIndex);


    ShortestPathCostFunction();

  protected:

    virtual ~ShortestPathCostFunction() {};
    void PrintSelf(std::ostream& os, Indent indent) const;
    ImageConstPointer    m_Image;
    IndexType m_StartIndex, m_EndIndex; 

  private:

    ShortestPathCostFunction(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

  };

} // end namespace itk


#include "itkShortestPathCostFunction.txx"

#endif /* __itkShortestPathCostFunction_h */
