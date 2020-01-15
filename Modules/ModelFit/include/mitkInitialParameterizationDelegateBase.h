/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __INITIAL_PARAMETERIZATION_DELEGATE_BASE_H
#define __INITIAL_PARAMETERIZATION_DELEGATE_BASE_H

#include <itkObject.h>
#include <itkIndex.h>

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /** Base class for all initial parametrization delegates
   * These delegates are used to define custom strategies, which are used by model parameterizers
   * to determine the initial parametrization of a model (e.g. starting parameter for fitting).
   */
  class MITKMODELFIT_EXPORT InitialParameterizationDelegateBase : public itk::Object
  {
  public:
    typedef InitialParameterizationDelegateBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(InitialParameterizationDelegateBase, itk::Object);

    typedef ModelBase ModelBaseType;

    typedef ModelBaseType::ParametersType ParametersType;

    typedef ::itk::Index<3> IndexType;

    /** Returns the parameterization (e.g. initial parametrization for fitting) that should be used.
     If no ParameterizationDelegate is set (see SetInitialParameterizationDelegate()) it will just return
     the result of GetInitialParameterization().*/
    virtual ParametersType GetInitialParameterization() const = 0;
    virtual ParametersType GetInitialParameterization(const IndexType& currentPosition) const = 0;

  protected:

    InitialParameterizationDelegateBase();

    ~InitialParameterizationDelegateBase() override;

  private:

    //No copy constructor allowed
    InitialParameterizationDelegateBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __MODEL_PARAMETERIZER_BASE_H
