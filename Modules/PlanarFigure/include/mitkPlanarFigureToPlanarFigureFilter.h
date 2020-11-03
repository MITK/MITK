/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPlanarFigureToPlanarFigureFilter_H_HEADER_INCLUDED
#define MITKPlanarFigureToPlanarFigureFilter_H_HEADER_INCLUDED

#include "mitkCommon.h"
#include "mitkPlanarFigure.h"
#include "mitkPlanarFigureSource.h"
#include <MitkPlanarFigureExports.h>

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
    mitkClassMacro(PlanarFigureToPlanarFigureFilter, PlanarFigureSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef PlanarFigure InputType;
    typedef InputType::Pointer InputTypePointer;
    typedef itk::DataObject::Pointer DataObjectPointer;

    using Superclass::SetInput;

    virtual void SetInput(const InputType *figure);

    virtual void SetInput(unsigned int idx, const InputType *figure);

    virtual const InputType *GetInput();

    virtual const InputType *GetInput(unsigned int idx);

    virtual void CreateOutputsForAllInputs();

  protected:
    PlanarFigureToPlanarFigureFilter();
    ~PlanarFigureToPlanarFigureFilter() override;
  };
} // namespace mitk
#endif
