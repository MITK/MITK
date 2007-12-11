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


#ifndef _MITK_DATA_TREE_NODE_SOURCE_H
#define _MITK_DATA_TREE_NODE_SOURCE_H

#include "itkProcessObject.h"
#include "mitkDataTreeNode.h"

namespace mitk
{
/**
 * @brief Superclass of all classes generating data tree nodes (instances of class
 * mitk::DataTreeNode) as output. 
 *
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. This is also true for subclasses of mitk::BaseProcess and thus
 * for mitk::DataTreeNodeSource.
 * @ingroup Process
 */
  class DataTreeNodeSource : public itk::ProcessObject
{
public:
    mitkClassMacro( DataTreeNodeSource, itk::ProcessObject );

    itkNewMacro( Self );

    typedef mitk::DataTreeNode OutputType;

    typedef OutputType::Pointer OutputTypePointer;
    
    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual itk::DataObject::Pointer MakeOutput ( unsigned int idx );

    /**
     * Allows to set the output of the base data source. 
     * @param output the intended output of the base data source
     */
    virtual void SetOutput( OutputType* output );
    
    /**
     * Allows to set the n-th output of the base data source. 
     * @param output the intended output of the base data source
     */
    virtual void SetOutput( unsigned int idx, OutputType* output );
    
    /**
     * Returns the output with index 0 of the base data source
     * @returns the output
     */
    virtual OutputType* GetOutput();
    
    /**
     * Returns the n'th output of the base data source
     * @param idx the index of the wanted output
     * @returns the output with index idx.
     */
    virtual OutputType* GetOutput ( unsigned int idx );
    
protected:

    DataTreeNodeSource();
    
    virtual ~DataTreeNodeSource();

};

}
#endif // #define _MITK_BASE_DATA_SOURCE_H
