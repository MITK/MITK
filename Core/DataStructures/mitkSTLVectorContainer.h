#ifndef _MITK_STL_VECTOR_H_
#define _MITK_STL_VECTOR_H_

#include <vector>
#include "mitkCommon.h"
#include <itkDataObject.h>
#include <itkObjectFactory.h>
#include <itkSimpleDataObjectDecorator.h>    

namespace mitk
{
    
typedef itk::SimpleDataObjectDecorator<std::vector>
    
template <typename T>
class STLVectorContainer : public itk::SimpleDataObjectDecorator< std::vector<T> > 
{
public:

    mitkClassMacro( STLVectorContainer, itk::SimpleDataObjectDecorator );

    itkNewMacro( Self );
    
    typedef T ValueType;
    
    typedef std::vector<ValueType> STLVectorContainerType;
    
    /**
     * Returns the number of elements contained in the
     * underlying stl-vector
     */
    unsigned int GetSize();
    
    /**
     * Emptys the undelying stl-vector. That means, that
     * after a call to this function, GetSize() returns 0.
     */
    void Clear();
    
    /**
     * Resizes the underlying stl vector to the given size.
     */
    void Resize( unsigned int size );
    
    /**
     * sets the n'th element of the underluing stl vector. 
     * if the index is out of range, an assertion is raised
     */
    void SetElement( const unsigned int& index, const ValueType& element );
    
    /**
     * returns the n'th element of the undelying stl vector
     * If the index is out of range, an assertion is raised!
     */
    ValueType& GetElement( const unsigned int& index );
    

};

typedef STLVectorContainer<int> IntVectorContainer;
typedef STLVectorContainer<unsigned int> UnsignedIntVectorContainer;
typedef STLVectorContainer<short int> ShortIntVectorContainer;
typedef STLVectorContainer<short unsigned int> ShortUnsignedIntVectorContainer;
typedef STLVectorContainer<float> FloatVectorContainer;
typedef STLVectorContainer<double> DoubleVectorContainer;


}

#include "mitkSTLVector.txx"

#endif
