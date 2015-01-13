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


#ifndef MITKPlanarFigureToPlanarFigureFilter_H_HEADER_INCLUDED
#define MITKPlanarFigureToPlanarFigureFilter_H_HEADER_INCLUDED

#include "mitkPlanarFigure.h"
#include <MitkPlanarFigureExports.h>
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
 * @ingroup MitkPlanarFigureModule
 */
class MITKPLANARFIGURE_EXPORT PlanarFigureToPlanarFigureFilter : public mitk::PlanarFigureSource
{
public:
    mitkClassMacro( PlanarFigureToPlanarFigureFilter, PlanarFigureSource );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef PlanarFigure InputType;
    typedef InputType::Pointer InputTypePointer;
    typedef itk::DataObject::Pointer DataObjectPointer;

    using Superclass::SetInput;

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
