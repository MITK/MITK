/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelBasedValueFunctorBase_h
#define mitkModelBasedValueFunctorBase_h

#include <mitkIndexedValueFunctorBase.h>

#include <MitkModelFitExports.h>

namespace mitk
{

  /**
   * \class ModelBasedValueFunctorBase
   * \brief Abstract functor base class for model-based indexed value computation.
   *
   * Extends IndexedValueFunctorBase by adding the ability to query the signal
   * grid (time grid) of the underlying model, via GetGrid().
   *
   * \sa IndexedValueFunctorBase, ModelFitInfoSignalGenerationFunctor
   */
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

    ~ModelBasedValueFunctorBase() override
    {};

  };

}


#endif
