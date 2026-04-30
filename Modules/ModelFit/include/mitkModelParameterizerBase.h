/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkModelParameterizerBase_h
#define mitkModelParameterizerBase_h

#include <itkObject.h>
#include <itkIndex.h>

#include <mitkModelBase.h>
#include <mitkInitialParameterizationDelegateBase.h>

#include <MitkModelFitExports.h>

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

    /**
     * \brief Returns the global static parameters that are the same for all positions.
     * \return A map of global static parameter names to their values.
     */
    virtual StaticParameterMapType GetGlobalStaticParameters() const = 0;

    /**
     * \brief Returns the local static parameters specific to the given image position.
     * \param[in] currentPosition The 3D index position in image space.
     * \return A map of local static parameter names to their values.
     */
    virtual StaticParameterMapType GetLocalStaticParameters(const IndexType& currentPosition) const = 0;

    /**
     * \brief Returns the initial parameterization for fitting.
     *
     * If an InitialParameterizationDelegate has been set, it will be used.
     * Otherwise, GetDefaultInitialParameterization() is returned.
     *
     * \return The initial parameter values.
     */
    ParametersType GetInitialParameterization() const;

    /**
     * \brief Returns the initial parameterization for a specific image position.
     * \param[in] currentPosition The 3D index position in image space.
     * \return The initial parameter values for the given position.
     */
    ParametersType GetInitialParameterization(const IndexType& currentPosition) const;

    /**
     * \brief Returns the default initial parameterization defined by the model developer.
     * \return The default initial parameter values for fitting.
     */
    virtual ParametersType GetDefaultInitialParameterization() const = 0;

    /**
     * \brief Sets a custom strategy for defining the initial parameterization.
     * \param[in] delegate Pointer to the delegate that provides custom initial values.
     */
    void SetInitialParameterizationDelegate(const InitialParameterizationDelegateBase* delegate);

    /**
     * \brief Generates a fully parameterized model instance for the given position.
     *
     * The model is configured with both global and local static parameters,
     * and the default time grid.
     *
     * \param[in] currentPosition The 3D index position in image space.
     * \return Smart pointer to the fully parameterized model.
     */
    virtual ModelBasePointer GenerateParameterizedModel(const IndexType& currentPosition) const = 0;

    /**
     * \brief Generates a model instance with only global static parameterization.
     *
     * Local static parameters remain at their default values.
     *
     * \return Smart pointer to the globally parameterized model.
     */
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

#endif
