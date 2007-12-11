/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef _MITK_STL_VECTOR_H_
#define _MITK_STL_VECTOR_H_

#include <vector>
#include <itkSimpleDataObjectDecorator.h>    

namespace mitk
{
    
    
template <typename T>
class STLVectorContainer : public itk::SimpleDataObjectDecorator< std::vector<T> > 
{
public:

    typedef STLVectorContainer<T> Self; 
    typedef itk::SimpleDataObjectDecorator< std::vector<T> > Superclass; 
    typedef itk::SmartPointer<Self> Pointer; 
    typedef itk::SmartPointer<const Self>  ConstPointer; 
    itkTypeMacro(STLVectorContainer,itk::SimpleDataObjectDecorator)
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
typedef STLVectorContainer<long> LongVectorContainer;
typedef STLVectorContainer<unsigned long> UnsignedLongVectorContainer;
typedef STLVectorContainer<short int> ShortIntVectorContainer;
typedef STLVectorContainer<short unsigned int> ShortUnsignedIntVectorContainer;
typedef STLVectorContainer<float> FloatVectorContainer;
typedef STLVectorContainer<double> DoubleVectorContainer;


} //end of namespace mitk

#include "mitkSTLVectorContainer.txx"

#endif
