#ifndef _MITK_STL_VECTOR_CONTAINER_TXX_
#define _MITK_STL_VECTOR_CONTAINER_TXX_

#include "mitkSTLVectorContainer.h"

template<typename T>
unsigned int mitk::STLVectorContainer<T>::GetSize()
{
    return m_Component.get_size();    
}

template<typename T>
void mitk::STLVectorContainer<T>::Clear()
{
    return m_Component.clear();    
    this->Modified();
}

/*
 * Resizes the underlying stl vector to the given size.
 */
template<typename T>
void mitk::STLVectorContainer<T>::Resize( unsigned int size )
{
    m_Component.resize( size );    
    this->Modified();
}

/*
 * sets the n'th element of the underluing stl vector. 
 * if the index is out of range, an assertion is raised
 */
template<typename T>
void mitk::STLVectorContainer<T>::SetElement( const unsigned int& index, const mitk::STLVectorContainer<T>::ValueType& element )
{
    assert( index < m_Component.size() );
    m_Component[index] = element;
    this->Modified();
}

/*
 * returns the n'th element of the undelying stl vector
 * If the index is out of range, an assertion is raised!
 */
template<typename T>
mitk::STLVectorContainer<T>::ValueType& mitk::STLVectorContainer<T>::GetElement( const unsigned int& index )
{
    assert( index < m_Component.size() );
    return m_Component[index];
    
}
        

#endif
