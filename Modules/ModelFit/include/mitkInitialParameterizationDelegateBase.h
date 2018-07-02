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

    virtual ~InitialParameterizationDelegateBase();

  private:

    //No copy constructor allowed
    InitialParameterizationDelegateBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __MODEL_PARAMETERIZER_BASE_H
