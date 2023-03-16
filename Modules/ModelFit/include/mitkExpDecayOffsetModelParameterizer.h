/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExpDecayOffsetModelParameterizer_h
#define mitkExpDecayOffsetModelParameterizer_h

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkExpDecayOffsetModel.h"
#include "MitkModelFitExports.h"

namespace mitk
{

  class MITKMODELFIT_EXPORT ExpDecayOffsetModelParameterizer : public
    mitk::ConcreteModelParameterizerBase<mitk::ExpDecayOffsetModel>
  {
  public:
    typedef ExpDecayOffsetModelParameterizer Self;
    typedef mitk::ConcreteModelParameterizerBase<mitk::ExpDecayOffsetModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ExpDecayOffsetModelParameterizer,
      mitk::ConcreteModelParameterizerBase<mitk::ExpDecayOffsetModel>);
    itkFactorylessNewMacro(Self);

    typedef Superclass::ModelBaseType ModelBaseType;
    typedef Superclass::ModelBasePointer ModelBasePointer;

    typedef Superclass::ModelType ModelType;
    typedef ModelType::Pointer ModelPointer;

    typedef Superclass::StaticParameterValueType StaticParameterValueType;
    typedef Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef  Superclass::StaticParameterMapType StaticParameterMapType;

    typedef Superclass::IndexType IndexType;

    ParametersType GetDefaultInitialParameterization() const override;

  protected:
    ExpDecayOffsetModelParameterizer();

    ~ExpDecayOffsetModelParameterizer() override;

  private:

    //No copy constructor allowed
    ExpDecayOffsetModelParameterizer(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif
