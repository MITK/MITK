/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIPreferences_h
#define mitkIPreferences_h

#include <mitkMessage.h>

#include <MitkCoreExports.h>

#include <cstddef>
#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Interface to application preferences.
   *
   * %Preferences are implemented as nodes in a tree structure. Each preference node may consist of
   * properties, i.e. key-value pairs, and child preferences nodes. The lifetime of nodes is managed
   * by their respective parent node.
   *
   * Use CoreServices::GetPreferencesService() and IPreferencesService::GetSystemPreferences() to
   * access the application preferences, i.e. the preferences root node.
   *
   * Use the Node() method to traverse through the tree.
   *
   * Getters and setters for various data types, e.g. GetInt() and PutInt(), are provided to conveniently
   * access properties as their intended data types. All preferences are ultimately stored as strings.
   *
   * Call Flush() to enforce the persistence of the whole (!) preferences tree. Preferences are flushed
   * automatically at least on a regular application shutdown.
   *
   * Changes of a preference node can be observed via IPreferences::OnChanged.
   * Property changes of a preference node can be observed additionally via IPreferences::OnPropertyChanged.
   *
   * \sa CoreServices::GetPreferencesService()
   * \sa IPreferencesService
   */
  class MITKCORE_EXPORT IPreferences
  {
  public:
    /**
     * \brief Event object sent on IPreferences::OnPropertyChanged events.
     */
    class MITKCORE_EXPORT ChangeEvent
    {
    public:
      ChangeEvent(IPreferences* source, const std::string& property, const std::string& oldValue, const std::string& newValue);

      IPreferences* GetSource() const;
      std::string GetProperty() const;
      std::string GetOldValue() const;
      std::string GetNewValue() const;

    private:
      IPreferences* m_Source;
      std::string m_Property;
      std::string m_OldValue;
      std::string m_NewValue;
    };

    virtual ~IPreferences();

    /**
     * \brief Get a property value as string.
     *
     * If the property cannot be found, return the provided default value instead.
     *
     * \param key Name of the property
     * \param def Default value
     *
     * \return Property value or default value
     */
    virtual std::string Get(const std::string& key, const std::string& def) const = 0;

    /**
     * \brief Set a property value.
     *
     * Create the property if not yet existent. Otherwise change its value.
     *
     * Trigger an IPreferences::OnPropertyChanged event if the new value is different
     * or if a new property has been created, except the string value is empty in
     * the latter case.
     */
    virtual void Put(const std::string& key, const std::string& value) = 0;

    /**
     * \brief Get a property value as \c int.
     *
     * Convenience wrapper of Get() undertaking string to integer conversion.
     *
     * \throw Exception The property value is not a valid string representation of an \c int value.
     */
    virtual int GetInt(const std::string& key, int def) const = 0;

    /**
     * \brief Set an \c int property value.
     *
     * Convenience wrapper of Put() undertaking integer to string conversion.
     */
    virtual void PutInt(const std::string& key, int value) = 0;

    /**
     * \brief Get a property value as \c bool.
     *
     * Convenience wrapper of Get() checking if the string value literally equals \c "true".
     * Returns false otherwise. The comparison is case-insensitive.
     */
    virtual bool GetBool(const std::string& key, bool def) const = 0;

    /**
     * \brief Set a \c bool property value.
     *
     * Convenience wrapper of Put() using the literal strings \c "true" and \c "false" to
     * encode the \c bool value.
     */
    virtual void PutBool(const std::string& key, bool value) = 0;

    /**
     * \brief Get a property value as \c float.
     *
     * Convenience wrapper of Get() undertaking string to \c float conversion.
     *
     * \throw Exception The property value is not a valid string representation of a \c float value.
     */
    virtual float GetFloat(const std::string& key, float def) const = 0;

    /**
     * \brief Set a \c float property value.
     *
     * Convenience wrapper of Put() undertaking \c float to string conversion.
     */
    virtual void PutFloat(const std::string& key, float value) = 0;

    /**
     * \brief Get a property value as \c double.
     *
     * Convenience wrapper of Get() undertaking string to \c double conversion.
     *
     * \throw Exception The property value is not a valid string representation of a \c double value.
     */
    virtual double GetDouble(const std::string& key, double def) const = 0;

    /**
     * \brief Set a \c double property value.
     *
     * Convenience wrapper of Put() undertaking \c double to string conversion.
     */
    virtual void PutDouble(const std::string& key, double value) = 0;

    /**
     * \brief Get a property value as typeless binary representation.
     *
     * Decode a Base64-encoded property value into a typeless binary representation.
     * Use \c reinterpret_cast to restore the original type:
     *
     * \code{.cpp}
     * // Restore position and size of a widget from preferences
     * std::vector<std::byte> geometry = preferences->GetByteArray("widget geometry", nullptr, 0);
     * widget->restoreGeometry(QByteArray(reinterpret_cast<const char*>(geometry.data())), geometry.size());
     * \endcode
     *
     * \param key Name of the property
     * \param def Raw pointer to the default binary data or \c nullptr
     * \param size Size of the default binary data in bytes or \c 0 in case of \c nullptr
     *
     * \return Decoded binary data encapsulated in a vector of bytes. Use \c std::vector::data() to get a raw pointer to the data.
     */
    virtual std::vector<std::byte> GetByteArray(const std::string& key, const std::byte* def, size_t size) const = 0;

    /**
     * \brief Put binary data into a property value.
     *
     * The Base64 algorithm is used to encode binary data into a valid text representation.
     * Use \c reinterpret_cast to pass a raw pointer to binary data and specify its total
     * number of bytes:
     *
     * \code{.cpp}
     * // Store position and size of a widget in preferences
     * QByteArray geometry = widget->saveGeometry();
     * preferences->PutByteArray("widget geometry", reinterpret_cast<const std::byte*>(geometry.data()), geometry.size());
     * \endcode
     *
     * \param key Name of the property
     * \param array Raw pointer to binary data
     * \param size Size of the binary data in bytes
     */
    virtual void PutByteArray(const std::string& key, const std::byte* array, size_t size) = 0;

    /**
     * \brief Write all (!) preferences to disk.
     *
     * Enforce the persistence of the whole preferences tree.
     *
     * \note Preferences are flushed automatically at least on a regular application shutdown.
     *
     * \sa IPreferencesStorage::Flush()
     */
    virtual void Flush() = 0;

    /**
     * \brief Remove a property from this preferences node.
     *
     * \param key Name of the property
     */
    virtual void Remove(const std::string& key) = 0;

    /**
     * \brief Remove all properties from this preferences node.
     */
    virtual void Clear() = 0;

    /**
     * \brief Get the names of all properties of this preferences node.
     */
    virtual std::vector<std::string> Keys() const = 0;

    /**
     * \brief Get the name of this preferences node.
     *
     * The name can be used in an absolute or relative path separated by forward slashes to address a specific properties node.
     *
     * \sa AbsolutePath() const
     * \sa Node()
     */
    virtual std::string Name() const = 0;

    /**
     * \brief Get the absolute path (relative to the root node) of this preferences node.
     *
     * \sa Name() const
     * \sa Node()
     */
    virtual std::string AbsolutePath() const = 0;

    /**
     * \brief Get the names of all direct child preference nodes.
     */
    virtual std::vector<std::string> ChildrenNames() const = 0;

    /**
     * \brief Get the parent preferences node or \c nullptr in case of the root node.
     */
    virtual IPreferences* Parent() = 0;

    /**
     * \sa Parent()
     */
    virtual const IPreferences* Parent() const = 0;

    /**
     * \brief Get the root preferences node.
     */
    virtual IPreferences* Root() = 0;

    /**
     * \sa Root()
     */
    virtual const IPreferences* Root() const = 0;

    /**
     * \brief Get (and possibly create) the preferences node specified by the given path.
     *
     * The path is considered to be absolute if it starts with a forward slash. It is relative to
     * this preferences node otherwise. If any preferences node does not yet exist during traversal,
     * it is created on the fly.
     *
     * \sa Name() const
     * \sa AbsolutePath() const
     */
    virtual IPreferences* Node(const std::string& path) = 0;

    /**
     * \brief Remove this preferences node from its parent node.
     *
     * \note Since preferences nodes are owned by their parents, this preferences node
     * is immediately deleted after removal.
     */
    virtual void RemoveNode() = 0;

    Message1<const IPreferences*> OnChanged; /**< \brief Notify on node changes. */
    Message1<const ChangeEvent&> OnPropertyChanged; /**< \brief Notify on property changes. */
  };
}

#endif
