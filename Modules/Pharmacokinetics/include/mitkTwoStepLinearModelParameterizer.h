/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTwoStepLinearModelParameterizer_h
#define mitkTwoStepLinearModelParameterizer_h

#include "mitkConcreteModelParameterizerBase.h"
#include "mitkTwoStepLinearModel.h"

namespace mitk
{

class MITKPHARMACOKINETICS_EXPORT TwoStepLinearModelParameterizer : public
  mitk::ConcreteModelParameterizerBase<mitk::TwoStepLinearModel>
{
public:
  typedef TwoStepLinearModelParameterizer Self;
  typedef mitk::ConcreteModelParameterizerBase<mitk::TwoStepLinearModel> Superclass;
  typedef itk::SmartPointer< Self >                            Pointer;
  typedef itk::SmartPointer< const Self >                      ConstPointer;

  itkTypeMacro(TwoStepLinearModelParameterizer, ConcreteModelParameterizerBase);
  itkFactorylessNewMacro(Self);

  typedef typename Superclass::ModelBaseType ModelBaseType;
  typedef typename Superclass::ModelBasePointer ModelBasePointer;

  typedef typename Superclass::ModelType ModelType;
  typedef typename Superclass::ModelPointer ModelPointer;

  typedef typename Superclass::StaticParameterValueType StaticParameterValueType;
  typedef typename Superclass::StaticParameterValuesType StaticParameterValuesType;
  typedef typename Superclass::StaticParameterMapType StaticParameterMapType;

  typedef typename Superclass::IndexType IndexType;

  /** This function returns the default parameterization (e.g. initial parametrization for fitting)
   defined by the model developer for  for the given model.*/
  virtual ParametersType GetDefaultInitialParameterization() const;

protected:
  TwoStepLinearModelParameterizer(){};

  virtual ~TwoStepLinearModelParameterizer(){};

private:

  //No copy constructor allowed
  TwoStepLinearModelParameterizer(const Self& source);
  void operator=(const Self&);  //purposely not implemented
};}


#endif
