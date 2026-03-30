/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkWeakPointerProperty_h
#define mitkWeakPointerProperty_h

#include <itkWeakPointer.h>
#include <mitkBaseProperty.h>
#include <MitkCoreExports.h>

namespace mitk
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4522)
#endif

  /**
   * \brief Property containing a weak (non-owning) pointer to an itk::Object.
   *
   * Unlike SmartPointerProperty, this property does not prevent the referenced
   * object from being deleted. The weak pointer becomes invalid if the
   * referenced object is destroyed.
   *
   * \note This property does not support JSON serialization. ToJSON() and
   * FromJSON() return \c false.
   *
   * \ingroup DataManagement
   *
   * \sa BaseProperty
   * \sa SmartPointerProperty
   */
  class MITKCORE_EXPORT WeakPointerProperty : public BaseProperty
  {
  public:
    mitkClassMacro(WeakPointerProperty, BaseProperty);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);
    mitkNewMacro1Param(WeakPointerProperty, itk::Object*);

    ~WeakPointerProperty() override;

    /** \brief The type of the value stored by this property. */
    typedef itk::WeakPointer<itk::Object> ValueType;

    /**
     * \brief Get the stored weak pointer.
     * \return The itk::WeakPointer value.
     */
    ValueType GetWeakPointer() const;

    /**
     * \brief Get the stored weak pointer (alias for GetWeakPointer()).
     * \return The itk::WeakPointer value.
     */
    ValueType GetValue() const;

    /**
     * \brief Set the stored weak pointer.
     *
     * \param[in] pointer The itk::Object to point to, or \c nullptr.
     */
    void SetWeakPointer(itk::Object *pointer);

    /**
     * \brief Set the stored weak pointer (alias for SetWeakPointer()).
     *
     * \param[in] value The itk::WeakPointer value to set.
     */
    void SetValue(const ValueType &value);

    /**
     * \brief Return the pointer address as a string.
     * \return A string representation of the pointer address.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief JSON serialization is not supported for WeakPointerProperty.
     *
     * \param[out] j Unused.
     * \return Always \c false.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief JSON deserialization is not supported for WeakPointerProperty.
     *
     * \param[in] j Unused.
     * \return Always \c false.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    itk::WeakPointer<itk::Object> m_WeakPointer;

    WeakPointerProperty(const WeakPointerProperty &);

    WeakPointerProperty(itk::Object *pointer = nullptr);

    mitkCloneMacro(WeakPointerProperty);

  private:
    // purposely not implemented
    WeakPointerProperty &operator=(const WeakPointerProperty &);

    bool IsEqual(const BaseProperty &property) const override;
    bool Assign(const BaseProperty &property) override;
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif

} // namespace mitk

#endif
