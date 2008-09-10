#ifndef __itkConfidenceDiffusionTensor3D_cpp
#define __itkConfidenceDiffusionTensor3D_cpp

#include "itkConfidenceDiffusionTensor3D.h"

namespace itk  
{ 

  /*
  * Default Constructor 
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>
    ::ConfidenceDiffusionTensor3D()
  {
  }


  /*
  * Constructor with initialization
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>
    ::ConfidenceDiffusionTensor3D( const Self & r ):DiffusionTensor3D<T>(r)
  {
    m_Confidence = r.m_Confidence;
  }



  /*
  * Constructor with initialization
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>
    ::ConfidenceDiffusionTensor3D( const Superclass & r ):DiffusionTensor3D<T>(r)
  {
    m_Confidence = 0;
  }



  /*
  * Constructor with initialization
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>
    ::ConfidenceDiffusionTensor3D( const ComponentType & r ):DiffusionTensor3D<T>(r)
  {
    m_Confidence = 0;
  }




  /*
  * Constructor with initialization
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>
    ::ConfidenceDiffusionTensor3D( const ComponentArrayType r ):DiffusionTensor3D<T>(r)
  {
    m_Confidence = 0;
  }


  /*
  * Assignment Operator
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>&
    ConfidenceDiffusionTensor3D<T>
    ::operator= (const Self& r)
  {
    Superclass::operator=(r);
    return *this;
  }



  /*
  * Assignment Operator
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>&
    ConfidenceDiffusionTensor3D<T>
    ::operator= (const ComponentType & r)
  {
    Superclass::operator=(r);
    return *this;
  }



  /*
  * Assignment Operator
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>&
    ConfidenceDiffusionTensor3D<T>
    ::operator= (const ComponentArrayType r)
  {
    Superclass::operator=(r);
    return *this;
  }



  /*
  * Assignment Operator
  */
  template<class T>
  ConfidenceDiffusionTensor3D<T>&
    ConfidenceDiffusionTensor3D<T>
    ::operator= (const Superclass & r)
  {
    Superclass::operator=(r);
    return *this;
  }

} // end namespace itk 

#endif /*__itkConfidenceDiffusionTensor3D_cpp*/

