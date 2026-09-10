/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVectorProperty_h
#define mitkVectorProperty_h

// MITK
#include <MitkCoreExports.h>
#include <mitkBaseProperty.h>

// STL
#include <vector>

namespace mitk
{
  /**
   * \brief Helper trait for VectorProperty to determine a good ITK class name prefix.
   *
   * This template is specialized for specific instantiations that need
   * a serializer for the VectorProperty. The default returns "Invalid"
   * to indicate an unsupported type.
   *
   * \tparam D The data type of the vector elements.
   *
   * \sa VectorProperty
   */
  template <typename D>
  struct VectorPropertyDataType
  {
    /** \brief Return the prefix string for the class name.
     *  \return "Invalid" for unspecialized types. */
    static const char *prefix() { return "Invalid"; }
  };

  /**
   * \brief Property for storing a std::vector of values.
   *
   * Templated over the data type of the std::vector that is held by this class.
   * Setting and getting of std::vectors is implemented by-value.
   *
   * The class manually expands most of mitkClassMacro to specifically override
   * GetNameOfClass() and GetStaticNameOfClass() in order to return type-dependent
   * class names (e.g., "DoubleVectorProperty", "IntVectorProperty"). This is
   * essential for serialization.
   *
   * \tparam DATATYPE The element type of the stored vector.
   *
   * \sa BaseProperty
   * \sa VectorPropertyDataType
   */
  template <typename DATATYPE>
  class MITKCORE_EXPORT VectorProperty : public BaseProperty
  {
  public:
    /** \brief The type of the std::vector stored by this property. */
    typedef std::vector<DATATYPE> VectorType;

    /** \brief The type of the value stored by this property. */
    typedef VectorType ValueType;

    // Manually expand most of mitkClassMacro:
    //   mitkClassMacro(VectorProperty<DATATYPE>, mitk::BaseProperty);
    // This manual expansion is done to override explicitly
    // the GetNameOfClass() and GetStaticNameOfClass() methods
    typedef VectorProperty<DATATYPE> Self;
    typedef BaseProperty SuperClass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    std::vector<std::string> GetClassHierarchy() const override { return mitk::GetClassHierarchy<Self>(); }

    /**
     * \brief Return the static class name including the type-dependent prefix.
     *
     * This function returns different strings depending on the template parameter,
     * which is required for serialization to work correctly.
     *
     * \return The class name string (e.g., "DoubleVectorProperty").
     */
    static const char *GetStaticNameOfClass()
    {
      // concatenate a prefix dependent on the template type and our own classname
      static std::string nameOfClass = std::string(VectorPropertyDataType<DATATYPE>::prefix()).append("VectorProperty");
      return nameOfClass.c_str();
    }

    const char *GetNameOfClass() const override { return this->GetStaticNameOfClass(); }
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(Self, const VectorType &);

    /**
     * \brief Return the property value as a human-readable string.
     *
     * Since VectorProperty potentially holds many elements, only the first
     * and last few elements appear in the string. Missing central elements
     * are indicated by an ellipsis ("...").
     *
     * \return A string representation showing the first and last elements.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Get a const reference to the contained vector.
     * \return A const reference to the stored vector.
     */
    virtual const VectorType &GetValue() const;

    /**
     * \brief Set the content vector.
     *
     * Calls Modified() only if the new vector differs from the current content.
     *
     * \param[in] parameter_vector The new vector to store.
     */
    virtual void SetValue(const VectorType &parameter_vector);

    /**
     * \brief Serialize the vector to JSON as a JSON array.
     *
     * \param[out] j The JSON value to write into.
     * \return Always \c true.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief Deserialize the vector from a JSON array.
     *
     * \param[in] j The JSON array to read from.
     * \return Always \c true.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    VectorProperty() = default;
    explicit VectorProperty(const VectorType &value) : m_PropertyContent(value) {}
    VectorProperty(const Self &other) : BaseProperty(other), m_PropertyContent(other.m_PropertyContent) {}

    mitkCloneMacro(Self);

  private:
    /// purposely not implemented
    VectorProperty &operator=(const Self &);

    /// compares two properties.
    bool IsEqual(const BaseProperty &an_other_property) const override;

    /// assigns the content of an_other_property to this
    bool Assign(const BaseProperty &an_other_property) override;

    /// property content
    VectorType m_PropertyContent;
  };

/// This should be used in .h files.
#define MITK_DECLARE_VECTOR_PROPERTY(TYPE, PREFIX)                                                                     \
                                                                                                                       \
  typedef VectorProperty<TYPE> PREFIX##VectorProperty;                                                                 \
                                                                                                                       \
  template <>                                                                                                          \
                                                                                                                       \
  struct VectorPropertyDataType<TYPE>                                                                                  \
  {                                                                                                                    \
    static const char *prefix() { return #PREFIX; }                                                                    \
  };

/// This should be used in a .cpp file
#define MITK_DEFINE_VECTOR_PROPERTY(TYPE) template class VectorProperty<TYPE>;

  MITK_DECLARE_VECTOR_PROPERTY(double, Double)
  MITK_DECLARE_VECTOR_PROPERTY(int, Int)

} // namespace

#endif
