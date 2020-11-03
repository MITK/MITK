/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __VALUE_BASED_PARAMETERIZATION_DELEGATE_H
#define __VALUE_BASED_PARAMETERIZATION_DELEGATE_H

#include <itkObject.h>
#include <itkIndex.h>

#include "mitkInitialParameterizationDelegateBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /** Parameterization delegate that always use parameters defined by the users.
   */
  class MITKMODELFIT_EXPORT ValueBasedParameterizationDelegate : public
    InitialParameterizationDelegateBase
  {
  public:
    typedef ValueBasedParameterizationDelegate Self;
    typedef InitialParameterizationDelegateBase Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    itkTypeMacro(ValueBasedParameterizationDelegate, InitialParameterizationDelegateBase);

    typedef Superclass::ModelBaseType ModelBaseType;

    typedef Superclass::ParametersType ParametersType;

    typedef Superclass::IndexType IndexType;

    void SetInitialParameterization(ParametersType params);

    /** Returns the parameterization (e.g. initial parametrization for fitting) that should be used.
     If no ParameterizationDelegate is set (see SetInitialParameterizationDelegate()) it will just return
     the result of GetInitialParameterization().*/
    ParametersType GetInitialParameterization() const override;
    ParametersType GetInitialParameterization(const IndexType& currentPosition) const override;

  protected:

    ParametersType m_Parameterization;

    ValueBasedParameterizationDelegate();

    ~ValueBasedParameterizationDelegate() override;

  private:

    //No copy constructor allowed
    ValueBasedParameterizationDelegate(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __MODEL_PARAMETERIZER_BASE_H
