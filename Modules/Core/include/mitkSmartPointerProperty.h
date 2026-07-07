/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSmartPointerProperty_h
#define mitkSmartPointerProperty_h

#include <mitkBaseProperty.h>
#include <mitkUIDGenerator.h>
#include <MitkCoreExports.h>

#include <list>
#include <map>
#include <string>

namespace mitk
{
  /**
   * \brief Property containing a smart pointer to an itk::Object.
   *
   * Stores a reference-counted smart pointer to an arbitrary itk::Object.
   * Internally tracks reference counts and UIDs for all pointed-to objects
   * to support XML serialization and deserialization workflows.
   *
   * \note This property does not support JSON serialization. ToJSON() and
   * FromJSON() return \c false.
   *
   * \sa BaseProperty
   * \sa WeakPointerProperty
   */
  class MITKCORE_EXPORT SmartPointerProperty : public BaseProperty
  {
  public:
    mitkClassMacro(SmartPointerProperty, BaseProperty);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    mitkNewMacro1Param(SmartPointerProperty, itk::Object*);

    /** \brief The type of the value stored by this property. */
    typedef itk::Object::Pointer ValueType;

    /**
     * \brief Get the stored smart pointer.
     * \return The itk::Object smart pointer.
     */
    itk::Object::Pointer GetSmartPointer() const;

    /**
     * \brief Get the stored smart pointer (alias for GetSmartPointer()).
     * \return The itk::Object smart pointer.
     */
    ValueType GetValue() const;

    /**
     * \brief Set the stored smart pointer.
     *
     * Updates internal reference counting and UID tracking. Marks the property
     * as modified if the pointer changes.
     *
     * Takes the new itk::Object to point to, or \c nullptr.
     */
    void SetSmartPointer(itk::Object *);

    /**
     * \brief Set the stored smart pointer (alias for SetSmartPointer()).
     *
     * Takes the new itk::Object smart pointer.
     */
    void SetValue(const ValueType &);

    /**
     * \brief Return the UID of the pointed-to object as a string.
     *
     * Returns "nullptr" if the pointer is null.
     *
     * \return The UID string of the referenced object.
     */
    std::string GetValueAsString() const override;

    /**
     * \brief Resolve all smart pointer references after XML deserialization.
     *
     * Iterates over all SmartPointerProperty instances that were read from XML
     * and resolves their pointer targets from the registered target map.
     */
    static void PostProcessXMLReading();

    /**
     * \brief Get the number of SmartPointerProperties referencing the given object.
     *
     * \return The reference count (0 if not referenced by any SmartPointerProperty).
     */
    static unsigned int GetReferenceCountFor(itk::Object *);

    /**
     * \brief Get the reference UID for the given object.
     *
     * \return The UID string, or "invalid" if the object is not tracked.
     */
    static std::string GetReferenceUIDFor(itk::Object *);

    /**
     * \brief Register an object as a potential target for XML deserialization.
     *
     * Takes the target object and the UID string to associate with it.
     */
    static void RegisterPointerTarget(itk::Object *, const std::string uid);

    /**
     * \brief JSON serialization is not supported for SmartPointerProperty.
     *
     * \param[out] j Unused.
     * \return Always \c false.
     */
    bool ToJSON(nlohmann::json& j) const override;

    /**
     * \brief JSON deserialization is not supported for SmartPointerProperty.
     *
     * \param[in] j Unused.
     * \return Always \c false.
     */
    bool FromJSON(const nlohmann::json& j) override;

    using BaseProperty::operator=;

  protected:
    SmartPointerProperty(itk::Object * = nullptr);
    SmartPointerProperty(const SmartPointerProperty &);

    itk::Object::Pointer m_SmartPointer;

    mitkCloneMacro(SmartPointerProperty);

  private:
    // purposely not implemented
    SmartPointerProperty &operator=(const SmartPointerProperty &);

    bool IsEqual(const BaseProperty &) const override;
    bool Assign(const BaseProperty &) override;

    typedef std::map<itk::Object *, unsigned int> ReferenceCountMapType;
    typedef std::map<itk::Object *, std::string> ReferencesUIDMapType;
    typedef std::map<SmartPointerProperty *, std::string> ReadInSmartPointersMapType;
    typedef std::map<std::string, itk::Object *> ReadInTargetsMapType;

    /// for each itk::Object* count how many SmartPointerProperties point to it
    static ReferenceCountMapType m_ReferenceCount;
    static ReferencesUIDMapType m_ReferencesUID;
    static ReadInSmartPointersMapType m_ReadInInstances;
    static ReadInTargetsMapType m_ReadInTargets;

    /// to generate unique IDs for the objects pointed at (during XML writing)
    static UIDGenerator m_UIDGenerator;
  };


} // namespace mitk

#endif
