/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13. Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPlanarFigureSOURCE_H_HEADER_INCLUDED
#define MITKPlanarFigureSOURCE_H_HEADER_INCLUDED

#include "mitkPlanarFigure.h"
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
  class MITKEXT_CORE_EXPORT PlanarFigureSource : public mitk::BaseProcess
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
