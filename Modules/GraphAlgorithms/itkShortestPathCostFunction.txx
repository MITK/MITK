#ifndef __itkShortestPathCostFunction_cpp
#define __itkShortestPathCostFunction_cpp


namespace itk
{ 

  // Constructor
  template<class TInputImageType>
  ShortestPathCostFunction<TInputImageType>
    ::ShortestPathCostFunction()
  {
  }


  template<class TInputImageType>
  void
    ShortestPathCostFunction<TInputImageType>
    ::PrintSelf( std::ostream& os, Indent indent ) const
  {
    Superclass::PrintSelf(os,indent);
  }

  
  template <class TInputImageType>
  void ShortestPathCostFunction<TInputImageType>::
    SetStartIndex (const typename TInputImageType::IndexType &StartIndex)
  {
    for (unsigned int i=0;i<TInputImageType::ImageDimension;++i)
    {
      m_StartIndex[i] = StartIndex[i];
    }
  }


  template <class TInputImageType>
  void ShortestPathCostFunction<TInputImageType>::
    SetEndIndex (const typename TInputImageType::IndexType &EndIndex)
  {
    for (unsigned int i=0;i<TInputImageType::ImageDimension;++i)
    {
      m_EndIndex[i] = EndIndex[i];
    }
  }


} // end namespace itk

#endif // __itkShortestPathCostFunction_cpp
