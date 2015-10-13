/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
  \brief Helper for VectorProperty to determine a good ITK ClassName.

  This template is specialized for special instantiations that need
  a serializer of this VectorProperty.
*/
template <typename D>
struct VectorPropertyDataType {
  static const char* prefix() { return "Invalid"; }
};

/**
  \brief Providing a std::vector as property.

  Templated over the data type of the std::vector that is held
  by this class. Nothing special about data handling, setting and
  getting of std::vectors is implemented by-value.

  When checking the declaration of this class, you'll notice
  that it does not use the mitkClassMacro but instead writes all
  of its definition manually.
  This is in order to specifically override the GetNameOfClass()
  method without having to inherit again from the template (see
  comments in code).
*/
template <typename DATATYPE>
class MITKCORE_EXPORT VectorProperty : public BaseProperty
{
public:

  typedef std::vector<DATATYPE> VectorType;

  // Manually expand most of mitkClassMacro:
  //   mitkClassMacro(VectorProperty<DATATYPE>, mitk::BaseProperty);
  // This manual expansion is done to override explicitly
  // the GetNameOfClass() and GetStaticNameOfClass() methods
  typedef VectorProperty<DATATYPE> Self;
  typedef BaseProperty SuperClass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;
  virtual std::vector<std::string> GetClassHierarchy() const override { return mitk::GetClassHierarchy<Self>(); }

  /// This function must return different
  /// strings in function of the template parameter!
  /// Serialization depends on this feature.
  static const char* GetStaticNameOfClass()
  {
    // concatenate a prefix dependent on the template type and our own classname
    static std::string nameOfClass = std::string( VectorPropertyDataType<DATATYPE>::prefix()) + "VectorProperty";
    return nameOfClass.c_str();
  }

  virtual const char* GetNameOfClass() const override
  {
    return this->GetStaticNameOfClass();
  }

  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self)

  /// Returns the property value as a std::string.
  ///
  /// Since VectorProperty potentially holds many
  /// elements, it implements this function in a way
  /// that only the first and the last couple of
  /// elements really appear in the string.
  /// Missing central elements are indicated by
  /// an ellipsis ("...")
  virtual std::string GetValueAsString() const override;

  /// returns a const reference to the contained vector
  virtual const VectorType& GetValue() const;

  /// sets the content vector
  virtual void SetValue(const VectorType& parameter_vector);

private:

  /// purposely not implemented
  VectorProperty& operator=(const Self&);

  /// creates a copy of it self
  virtual itk::LightObject::Pointer InternalClone() const override;

  /// compares two properties.
  virtual bool IsEqual(const BaseProperty& an_other_property) const override;

  /// assigns the content of an_other_property to this
  virtual bool Assign(const BaseProperty& an_other_property) override;

  /// property content
  VectorType m_PropertyContent;
};

/// This should be used in .h files.
#define MITK_DECLARE_VECTOR_PROPERTY(TYPE, PREFIX) \
 \
typedef VectorProperty<TYPE> PREFIX ## VectorProperty; \
 \
template <> \
struct VectorPropertyDataType< TYPE > \
{ \
  static const char* prefix() { return #PREFIX; } \
};

/// This should be used in a .cpp file
#define MITK_DEFINE_VECTOR_PROPERTY(TYPE) \
template class VectorProperty<TYPE>;

MITK_DECLARE_VECTOR_PROPERTY(double, Double)
MITK_DECLARE_VECTOR_PROPERTY(int, Int)

} // namespace

#endif

