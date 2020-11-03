/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CURVE_PARAMETER_FUNCTOR_H
#define CURVE_PARAMETER_FUNCTOR_H

#include "mitkCurveDescriptionParameterBase.h"
#include "mitkSimpleFunctorBase.h"
#include "MitkPharmacokineticsExports.h"

namespace mitk
{

  /**Functor for the curve description values by using the itkMulitOutputNaryImageFilter.
   * You may register any number of CurveDescriptionParamterBase instances to the functor.
   * The Functor will compute all values.
   * @warning the functor must be threadsafe and so must be the registered CurveDescriptionParamterBase instances.*/
  class MITKPHARMACOKINETICS_EXPORT CurveParameterFunctor : public SimpleFunctorBase
  {
  public:
    typedef CurveParameterFunctor Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

      itkFactorylessNewMacro(Self);
      itkTypeMacro(CurveParameterFunctor, SimpleFunctorBase);

      typedef CurveDescriptionParameterBase::CurveDescriptionParameterNameType ParameterNameType;
      typedef CurveDescriptionParameterBase::DescriptionParameterNamesType ParameterNamesType;
      using GridArrayType = SimpleFunctorBase::GridArrayType;

    SimpleFunctorBase::OutputPixelVectorType Compute(const InputPixelVectorType & value) const override;

    unsigned int GetNumberOfOutputs() const override;

    GridArrayType GetGrid() const override;
    itkSetMacro(Grid, GridArrayType);

    ParameterNamesType GetDescriptionParameterNames() const;

    /**@warning Teh function is currently not thread safe.
     @todo reimplement with shareable lock to allow other class methods to be used parallel but lock this one exclusively.*/
    void ResetDescriptionParameters();
    /**@warning Teh function is currently not thread safe.
    @todo reimplement with shareable lock to allow other class methods to be used parallel but lock this one exclusively.*/
    void RegisterDescriptionParameter(const ParameterNameType& parameterName, CurveDescriptionParameterBase* parameterFunction);
    /**@warning Teh function is currently not thread safe.
    @todo reimplement with shareable lock to allow other class methods to be used parallel but lock this one exclusively.*/
    const CurveDescriptionParameterBase* GetDescriptionParameterFunction(const ParameterNameType& parameterName) const;

  protected:
    CurveParameterFunctor();
    ~CurveParameterFunctor() override;

  private:
    typedef std::map<ParameterNameType, CurveDescriptionParameterBase::Pointer> DescriptionParameterMapType;
    DescriptionParameterMapType m_DescriptorMap;
    GridArrayType m_Grid;
  };

}

#endif // CURVE_PARAMETER_FUNCTOR_H
