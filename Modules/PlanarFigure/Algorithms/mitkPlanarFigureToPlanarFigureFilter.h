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


#ifndef MITKPlanarFigureToPlanarFigureFilter_H_HEADER_INCLUDED
#define MITKPlanarFigureToPlanarFigureFilter_H_HEADER_INCLUDED

#include "mitkPlanarFigure.h"
#include "PlanarFigureExports.h"
#include "mitkCommon.h"
#include "mitkPlanarFigureSource.h"

namespace mitk
{
/**
 * @brief Base class for all filters which have an object of type
 * mitk::PlanarFigure as input and output
 *
 * Base class for all filters which have an object of type mitk::PlanarFigure
 * as input and output. 
 * @ingroup PlanarFigure_EXPORT
 */
class PlanarFigure_EXPORT PlanarFigureToPlanarFigureFilter : public mitk::PlanarFigureSource
{
public:
    mitkClassMacro( PlanarFigureToPlanarFigureFilter, PlanarFigureSource );
    itkNewMacro( Self );

    typedef PlanarFigure InputType;
    typedef InputType::Pointer InputTypePointer;
    typedef itk::DataObject::Pointer DataObjectPointer;

    virtual void SetInput( const InputType* figure );

    virtual void SetInput( unsigned int idx, const InputType* figure );

    virtual const InputType* GetInput();

    virtual const InputType* GetInput( unsigned int idx );

    virtual void CreateOutputsForAllInputs();

protected:
    PlanarFigureToPlanarFigureFilter();
    virtual ~PlanarFigureToPlanarFigureFilter();
};
} // namespace mitk
#endif 
