/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MODEL_PARAMETERIZER_BASE_H
#define __MODEL_PARAMETERIZER_BASE_H

#include <itkObject.h>
#include <itkIndex.h>

#include "mitkModelBase.h"
#include "mitkInitialParameterizationDelegateBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /** Base class for all model parameterizers
   * Model parameterizers are used to offer a generic api to generate/bind
   * models with specific global static and local static parameters in order to
   * utilize the model correctly.\n
   * - Global static parameter: parameters that are the same for a model independent
   * of the spatial position (in image space) that should be modeled (e.g. parameter "Tau"
   * for the DescriptivePharmacokineticBrixModel)
   * - Local static parameter: parameters that are specific for the spatial position (in image space)
   * that should be modeled (e.g. parameter "S0" for the DescriptivePharmacokineticBrixModel)
   */
  class MITKMODELFIT_EXPORT ModelParameterizerBase : public itk::Object, public ModelTraitsInterface
  {
  public:
    typedef ModelParameterizerBase Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(ModelParameterizerBase, itk::Object);

    typedef ModelBase ModelBaseType;
    typedef ModelBaseType::Pointer ModelBasePointer;

    typedef ModelBaseType::ParametersType ParametersType;
    typedef ModelBaseType::StaticParameterValueType StaticParameterValueType;
    typedef ModelBaseType::StaticParameterValuesType StaticParameterValuesType;
    typedef ModelBaseType::StaticParameterMapType StaticParameterMapType;
    typedef ModelBaseType::TimeGridType TimeGridType;

    typedef ::itk::Index<3> IndexType;

    virtual StaticParameterMapType GetGlobalStaticParameters() const = 0;
    virtual StaticParameterMapType GetLocalStaticParameters(const IndexType& currentPosition) const = 0;

    /** Returns the parameterization (e.g. initial parametrization for fitting) that should be used.
     If no ParameterizationDelegate is set (see SetInitialParameterizationDelegate()) it will just return
     the result of GetInitialParameterization().*/
    ParametersType GetInitialParameterization() const;
    ParametersType GetInitialParameterization(const IndexType& currentPosition) const;

    /** This function returns the default parameterization (e.g. initial parametrization for fitting)
     defined by the model developer for  for the given model.*/
    virtual ParametersType GetDefaultInitialParameterization() const = 0;

    /** Possibility to set a custom strategy for defining the initial parameterization via a delegate.*/
    void SetInitialParameterizationDelegate(const InitialParameterizationDelegateBase* delegate);

    virtual ModelBasePointer GenerateParameterizedModel(const IndexType& currentPosition) const = 0;
    /** Generate model instance, only with global static parametrization.
     * Any local static parameter stay default.*/
    virtual ModelBasePointer GenerateParameterizedModel() const = 0;

    itkSetMacro(DefaultTimeGrid, TimeGridType);
    itkGetConstReferenceMacro(DefaultTimeGrid, TimeGridType);

  protected:


    ModelParameterizerBase();

    ~ModelParameterizerBase() override;

    InitialParameterizationDelegateBase::ConstPointer m_InitialDelegate;

    /** The default time grid that should be set to generated models.*/
    TimeGridType m_DefaultTimeGrid;
  private:

    //No copy constructor allowed
    ModelParameterizerBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };
}

#endif // __MODEL_PARAMETERIZER_BASE_H
