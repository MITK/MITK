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

#ifndef ModelBasedValueFunctorBase_H
#define ModelBasedValueFunctorBase_H

#include "mitkIndexedValueFunctorBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{

  /**Functor base class for functors that are some how based on an model.
   This abstract class adds the possibility to query the signal grid
   of the model.*/
  class MITKMODELFIT_EXPORT ModelBasedValueFunctorBase: public IndexedValueFunctorBase
  {
  public:
    typedef ModelBasedValueFunctorBase Self;
    typedef IndexedValueFunctorBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ModelBasedValueFunctorBase, IndexedValueFunctorBase);

    typedef Superclass::InputImagePixelType InputImagePixelType;
    typedef Superclass::InputPixelVectorType InputPixelVectorType;
    typedef Superclass::OutputPixelVectorType OutputPixelVectorType;
    typedef Superclass::IndexType IndexType;

    typedef itk::Array<double> GridArrayType;

    virtual GridArrayType GetGrid() const = 0;

  protected:

    ModelBasedValueFunctorBase()
    {};

    ~ModelBasedValueFunctorBase()
    {};

  };

}


#endif // ModelBasedValueFunctorBase_H
