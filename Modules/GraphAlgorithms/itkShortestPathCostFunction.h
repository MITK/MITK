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
#ifndef __itkShortestPathCostFunction_h
#define __itkShortestPathCostFunction_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkShapedNeighborhoodIterator.h"


namespace itk
{

  // \brief this is a pure virtual superclass for all cost functions used in itkShortestPathImageFilter
  template <class TInputImageType>
  class ShortestPathCostFunction : public Object
  {
  public:

    /** Standard class typedefs. */
    typedef ShortestPathCostFunction Self;
    typedef Object  Superclass;
    typedef SmartPointer<Self>   Pointer;
    typedef SmartPointer<const Self>  ConstPointer;
    typedef ShapedNeighborhoodIterator< TInputImageType >   ShapedNeighborhoodIteratorType;

    /** Run-time type information (and related methods). */
    itkTypeMacro(ShortestPathCostFunction, Object);

    /** Type definition for the input image. */
    typedef TInputImageType  ImageType;

    // More typdefs for convenience
    typedef typename TInputImageType::Pointer               ImagePointer;
    typedef typename TInputImageType::ConstPointer          ImageConstPointer;
    typedef typename TInputImageType::PixelType             PixelType;
    typedef typename TInputImageType::IndexType             IndexType;

    // \brief Set the input image.
    itkSetConstObjectMacro(Image,TInputImageType);

    // \brief Calculate the cost for going from pixel p1 to pixel p2
    virtual double GetCost( IndexType p1, IndexType p2) = 0;

    // \brief Return the minimal possible cost (needed for A*)
    virtual double GetMinCost() = 0;

    // \brief Initialize the metric
    virtual void Initialize () = 0;

    // \brief Set the starting index of a path
    void SetStartIndex (const IndexType & index);

    // \brief Set the ending index of a path
    void SetEndIndex(const IndexType & index);

  protected:

    ShortestPathCostFunction() {};
    virtual ~ShortestPathCostFunction() {};
    void PrintSelf(std::ostream& os, Indent indent) const override;
    ImageConstPointer    m_Image;
    IndexType m_StartIndex, m_EndIndex;

  private:

    ShortestPathCostFunction(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

  };

} // end namespace itk


#include "itkShortestPathCostFunction.txx"

#endif /* __itkShortestPathCostFunction_h */
