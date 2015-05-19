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


#ifndef MITKLOOKUPTABLESOURCE_H_HEADER_INCLUDED
#define MITKLOOKUPTABLESOURCE_H_HEADER_INCLUDED

#include "mitkLookupTable.h"
#include "MitkDataTypesExtExports.h"
#include "mitkCommon.h"

#include "itkProcessObject.h"

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
class MITKDATATYPESEXT_EXPORT LookupTableSource : public itk::ProcessObject
{
public:

    mitkClassMacroItkParent( LookupTableSource, itk::ProcessObject )
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef mitk::LookupTable OutputType;

    typedef OutputType::Pointer OutputTypePointer;

    typedef itk::DataObject::Pointer DataObjectPointer;

    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual itk::DataObject::Pointer MakeOutput ( DataObjectPointerArraySizeType idx ) override;

    /**
     * This is a default implementation to make sure we have something.
     * Once all the subclasses of ProcessObject provide an appopriate
     * MakeOutput(), then ProcessObject::MakeOutput() can be made pure
     * virtual.
     */
    virtual itk::DataObject::Pointer MakeOutput(const DataObjectIdentifierType &name) override;

    /**
     * Generates the input requested region simply by calling the equivalent
     * method of the superclass.
     */
    void GenerateInputRequestedRegion() override;

    /**
     * Replacement of the SetOutput method. I think it is not yet correcly
     * implemented, so you should better not use it.
     * @todo provide a more usefule implementation
     * @param output the intended output of the lookup table source.
     */
    virtual void GraftOutput( OutputType* output );

    virtual OutputType* GetOutput();
    virtual const OutputType* GetOutput() const;
    virtual OutputType* GetOutput(DataObjectPointerArraySizeType idx);
    virtual const OutputType* GetOutput(DataObjectPointerArraySizeType idx) const;

protected:
    LookupTableSource();
    virtual ~LookupTableSource();

};

} // namespace mitk



#endif
