/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkAIFBasedModelParameterizerBase_h
#define mitkAIFBasedModelParameterizerBase_h

#include <mitkConcreteModelParameterizerBase.h>
#include <mitkAIFParametrizerHelper.h>
#include <mitkAIFBasedModelBase.h>

namespace mitk
{
  /** \class AIFBasedModelParameterizerBase
   * \brief Base class for parameterizers of pharmacokinetic models that use an Arterial Input Function (AIF).
   *
   * This template class stores the AIF concentration values and corresponding time grid, and
   * provides them as global static parameters when configuring model instances for fitting.
   *
   * \tparam TAIFBasedModel The concrete AIF-based model type (must derive from AIFBasedModelBase).
   * \sa AIFBasedModelBase, ConcreteModelParameterizerBase
   */
  template <class TAIFBasedModel>
  class MITKPHARMACOKINETICS_EXPORT AIFBasedModelParameterizerBase : public ConcreteModelParameterizerBase
    <TAIFBasedModel>
  {
  public:
    typedef AIFBasedModelParameterizerBase<TAIFBasedModel> Self;
    typedef ConcreteModelParameterizerBase<TAIFBasedModel> Superclass;
    typedef itk::SmartPointer< Self >                            Pointer;
    typedef itk::SmartPointer< const Self >                      ConstPointer;

    itkTypeMacro(AIFBasedModelParameterizerBase, ConcreteModelParameterizerBase);

    typedef typename Superclass::ModelBaseType ModelBaseType;
    typedef typename Superclass::ModelBasePointer ModelBasePointer;

    typedef typename Superclass::ModelType ModelType;
    typedef typename Superclass::ModelPointer ModelPointer;

    typedef typename Superclass::StaticParameterValueType StaticParameterValueType;
    typedef typename Superclass::StaticParameterValuesType StaticParameterValuesType;
    typedef typename Superclass::StaticParameterMapType StaticParameterMapType;


    typedef typename Superclass::IndexType IndexType;

    /** \brief Sets the Arterial Input Function concentration values.
     *  \param[in] _arg AIF values as an itk::Array of double. */
    itkSetMacro(AIF, mitk::AIFBasedModelBase::AterialInputFunctionType);
    /** \brief Returns the currently set AIF concentration values.
     *  \return Const reference to the AIF array. */
    itkGetConstReferenceMacro(AIF, mitk::AIFBasedModelBase::AterialInputFunctionType);

    /** \brief Sets the time grid corresponding to the AIF values.
     *  \param[in] _arg Time grid as an itk::Array of double (in seconds). */
    itkSetMacro(AIFTimeGrid, mitk::ModelBase::TimeGridType);
    /** \brief Returns the currently set AIF time grid.
     *  \return Const reference to the AIF time grid array. */
    itkGetConstReferenceMacro(AIFTimeGrid, mitk::ModelBase::TimeGridType);


    /** \brief Returns the global static parameters for the model.
     *
     * This default implementation provides only the AIF and its time grid as static parameters.
     * Reimplement in derived classes to add additional static parameters.
     * \return Map of static parameter names to their value vectors. */
    StaticParameterMapType GetGlobalStaticParameters() const override
    {
      StaticParameterMapType result;
      StaticParameterValuesType valuesAIF = mitk::convertArrayToParameter(this->m_AIF);
      StaticParameterValuesType valuesAIFGrid = mitk::convertArrayToParameter(this->m_AIFTimeGrid);

      result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_AIF, valuesAIF));
      result.insert(std::make_pair(ModelType::NAME_STATIC_PARAMETER_AIFTimeGrid, valuesAIFGrid));

      return result;
    };


  protected:

    AIFBasedModelParameterizerBase()
    {};

    ~AIFBasedModelParameterizerBase() override
    {};


    mitk::AIFBasedModelBase::AterialInputFunctionType m_AIF;
    mitk::ModelBase::TimeGridType m_AIFTimeGrid;


  private:

    //No copy constructor allowed
    AIFBasedModelParameterizerBase(const Self& source);
    void operator=(const Self&);  //purposely not implemented
  };

}
#endif
