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


#ifndef MITKPlanarFigureSOURCE_H_HEADER_INCLUDED
#define MITKPlanarFigureSOURCE_H_HEADER_INCLUDED

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"
#include "mitkCommon.h"
#include "mitkBaseProcess.h"

namespace mitk
{
/**
 * @brief Base class for all filters which have an object of type
 * mitk::PlanarFigure as output
 *
 * Base class for all filters which have an object of type mitk::PlanarFigure
 * as output. mitk::PlanarFigureSources do not provide support
 * for streaming, that is, that the requested region is always the largest
 * possible region.
 * @ingroup Process
 */
class PlanarFigure_EXPORT PlanarFigureSource : public mitk::BaseProcess
{
public:
    mitkClassMacro( PlanarFigureSource, BaseProcess );
    itkNewMacro( Self );

    typedef mitk::PlanarFigure OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef itk::DataObject::Pointer DataObjectPointer;

    /**
     * Allocates a new output object and returns it.
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
     * Allows to set the output of the planar figure source. According to the itk documentation
     * this method is outdated and should not be used. Instead GraftOutput(...)
     * should be used.
     * @param output the intended output of the planar figure source
     */
    void SetOutput( OutputType* output );

    /**
     * Graft output 0 to the parameter graft
     */
    virtual void GraftOutput(itk::DataObject *graft);
    /**
    * Graft output idx to the parameter graft
    */
    virtual void GraftNthOutput(unsigned int idx, itk::DataObject *graft);

    /**
     * Returns the output with index 0 of the planar figure source
     * @returns the output
     */
    OutputType* GetOutput();

    /**
     * Returns the nth output of the planar figure source
     * @param idx the index of the wanted output
     * @returns the output with index idx.
     */
    OutputType* GetOutput ( unsigned int idx );

protected:
    PlanarFigureSource();
    virtual ~PlanarFigureSource();
};
} // namespace mitk
#endif
