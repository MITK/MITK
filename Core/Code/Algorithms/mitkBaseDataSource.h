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


#ifndef _MITK_BASE_DATA_SOURCE_H
#define _MITK_BASE_DATA_SOURCE_H

#include "mitkBaseProcess.h"
#include "mitkBaseData.h"

namespace mitk
{

/**
 * @brief Superclass of all classes generating base data (instances of class
 * mitk::BaseData) as output. 
 *
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. This is also true for subclasses of mitk::BaseProcess and thus
 * for mitk::BaseDataSource.
 * @ingroup Process
 */
class MITK_CORE_EXPORT BaseDataSource : public BaseProcess
{
public:
    mitkClassMacro( BaseDataSource, BaseProcess );

    typedef mitk::BaseData OutputType;

    typedef OutputType::Pointer OutputTypePointer;
    
    /**
     * Allocates a new output object and returns it. This function
     * is pure virtual because mitk::BaseData contains pure virtual functions
     * and can not directly be instantiated. This must be done by subclasses, which
     * know which subclasses of mitk::BaseData they want to instantiate. Thus, these 
     * classes have to set their output themselves via this->SetOutput(MakeOutput(0))
     * in the constructor!
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual itk::DataObject::Pointer MakeOutput ( unsigned int idx ) = 0;

    /**
     * Allows to set the output of the base data source. 
     * @param output the intended output of the base data source
     */
    virtual void SetOutput( OutputType* output );
    
    /**
     * Allows to set the n-th output of the base data source. 
     * @param idx The index of the output 
     * @param output The intended output of the base data source
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

    BaseDataSource();
    
    virtual ~BaseDataSource();

};

}
#endif // #define _MITK_BASE_DATA_SOURCE_H
