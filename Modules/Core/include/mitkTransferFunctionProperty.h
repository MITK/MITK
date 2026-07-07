/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkTransferFunctionProperty_h
#define mitkTransferFunctionProperty_h

#include <mitkBaseProperty.h>
#include <mitkTransferFunction.h>

namespace mitk
{
  /**
   * \brief Property for storing a mitk::TransferFunction.
   *
   * Holds a transfer function that maps scalar values to colors and opacities
   * for volume rendering or other rendering modes. The transfer function
   * consists of scalar opacity, gradient opacity, and color transfer function
   * components.
   *
   * \note If you want to use this property for an mitk::Image, make sure
   * to set the mitk::RenderingModeProperty to a mode which supports transfer
   * functions (e.g. COLORTRANSFERFUNCTION_COLOR). See the documentation of
   * mitk::RenderingModeProperty. For a code example see
   * mitkImageVtkMapper2DTransferFunctionTest.cpp in Core/Code/Testing.
   *
   * \sa BaseProperty
   * \sa TransferFunction
   * \sa RenderingModeProperty
   */
  class MITKCORE_EXPORT TransferFunctionProperty : public BaseProperty
  {
  public:
    /** \brief The type of the value stored by this property. */
    typedef mitk::TransferFunction::Pointer ValueType;

    mitkClassMacro(TransferFunctionProperty, BaseProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
      mitkNewMacro1Param(TransferFunctionProperty, mitk::TransferFunction::Pointer);

    /**
     * \brief Set the transfer function.
     * \param[in] _arg The new transfer function smart pointer.
     */
    itkSetMacro(Value, mitk::TransferFunction::Pointer);

    /**
     * \brief Get the transfer function.
     * \return The stored transfer function smart pointer.
     */
    itkGetConstMacro(Value, mitk::TransferFunction::Pointer);

    /**
     * \brief Return the transfer function as a string (address representation).
     * \return A string representation of the transfer function pointer.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Serialize the transfer function to JSON.
     *
     * Serializes scalar opacity points, gradient opacity points, and color
     * transfer function nodes into a structured JSON object.
     *
     * \param[out] j The JSON object to write the value into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the transfer function from JSON.
     *
     * Reads scalar opacity points, gradient opacity points, and color transfer
     * function nodes from a structured JSON object and creates a new
     * TransferFunction instance.
     *
     * \param[in] j The JSON object containing the transfer function data.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    mitk::TransferFunction::Pointer m_Value;

    TransferFunctionProperty();
    TransferFunctionProperty(const TransferFunctionProperty &other);

    TransferFunctionProperty(mitk::TransferFunction::Pointer value);

    mitkCloneMacro(TransferFunctionProperty);

  private:
    // purposely not implemented
    TransferFunctionProperty &operator=(const TransferFunctionProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };


} // namespace mitk

#endif
