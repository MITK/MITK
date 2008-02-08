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


#ifndef MITKLOOKUPTABLESOURCE_H_HEADER_INCLUDED
#define MITKLOOKUPTABLESOURCE_H_HEADER_INCLUDED

#include "mitkLookupTable.h"
#include "mitkCommon.h"
#include "mitkBaseProcess.h"

namespace mitk
{

/**
 * @brief Base class for all objects which have an object of type
 * mitkLookupTable as output
 *
 * Base class for all objects which have an object of type mitkLookupTable
 * as output. It is assumed, that mitkLookupTableSources do not provide support
 * for streaming, that is, that the requested region is always the largest
 * possible region.
 * @ingroup Process
 */
class MITK_CORE_EXPORT LookupTableSource : public itk::ProcessObject
{
public:

    mitkClassMacro( LookupTableSource, BaseProcess );

    itkNewMacro( Self );

    typedef mitk::LookupTable OutputType;

    typedef OutputType::Pointer OutputTypePointer;

    typedef itk::DataObject::Pointer DataObjectPointer;

    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual DataObjectPointer MakeOutput ( unsigned int idx );

    /**
     * Generates the input requested region simply by calling the equivalent
     * method of the superclass.
     */
    void GenerateInputRequestedRegion();

    /**
     * Allows to set the output of the lookup-table source. According to the itk documentation
     * this method is outdated and should not be used. Instead GraftOutput(...)
     * should be used.
     * @param output the intended output of the lookup table source
     */
    void SetOutput( OutputType* output );

    /**
     * Replacement of the SetOutput method. I think it is not yet correcly 
     * implemented, so you should better not use it.
     * @todo provide a more usefule implementation
     * @param output the intended output of the lookup table source.
     */
    virtual void GraftOutput( OutputType* output );

    /**
     * Returns the output with index 0 of the lookup table source
     * @returns the output
     */
    OutputType* GetOutput();

    /**
     * Returns the n'th output of the lookup table source
     * @param idx the index of the wanted output
     * @returns the output with index idx.
     */
    OutputType* GetOutput ( unsigned int idx );

protected:
    LookupTableSource();
    virtual ~LookupTableSource();

};

} // namespace mitk



#endif 


