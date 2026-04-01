/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPreferences_h
#define mitkPreferences_h

#include <mitkIPreferences.h>

#include <memory>
#include <optional>
#include <unordered_map>

namespace mitk
{
  class IPreferencesStorage;

  /**
   * \brief Implementation of the IPreferences interface.
   *
   * Stores preferences as string key-value pairs in a tree structure. Each node
   * has a name, an absolute path, a parent, and zero or more children. Values
   * are stored as strings and converted on access (int, bool, float, double, byte
   * array). Supports overrides that shadow regular property values without
   * modifying the persisted data.
   *
   * Only used through the IPreferences interface.
   *
   * \sa IPreferences
   * \sa IPreferencesStorage
   * \sa PreferencesService
   */
  class Preferences : public IPreferences
  {
  public:
    /** \brief Property map type: string keys to string values. */
    using Properties = std::unordered_map<std::string, std::string>;

    /**
     * \brief Construct a preferences node.
     *
     * If parent is non-null, this node is automatically added as a child of the parent.
     * The absolute path is computed from the parent's path and the given name.
     *
     * \param[in] properties Initial key-value properties for this node.
     * \param[in] name The name of this preferences node.
     * \param[in] parent The parent node, or nullptr for a root node.
     * \param[in] storage The backing storage. Must not be nullptr.
     * \throw mitk::Exception if storage is nullptr.
     */
    Preferences(const Properties& properties, const std::string& name, Preferences* parent, IPreferencesStorage* storage);

    ~Preferences() override;

    Preferences(const Preferences&) = delete;
    Preferences& operator=(const Preferences&) = delete;

    Preferences(Preferences&&) = default;
    Preferences& operator=(Preferences&&) = default;

    std::string Get(const std::string& key, const std::string& def) const override;
    void Put(const std::string& key, const std::string& value) override;

    int GetInt(const std::string& key, int def) const override;
    void PutInt(const std::string& key, int value) override;

    bool GetBool(const std::string& key, bool def) const override;
    void PutBool(const std::string& key, bool value) override;

    float GetFloat(const std::string& key, float def) const override;
    void PutFloat(const std::string& key, float value) override;

    double GetDouble(const std::string& key, double def) const override;
    void PutDouble(const std::string& key, double value) override;

    std::vector<std::byte> GetByteArray(const std::string& key, const std::byte* def, size_t size) const override;
    void PutByteArray(const std::string& key, const std::byte* array, size_t size) override;

    void Override(const std::string& key, const std::string& value) override;
    void OverrideInt(const std::string& key, int value) override;
    void OverrideBool(const std::string& key, bool value) override;
    void OverrideFloat(const std::string& key, float value) override;
    void OverrideDouble(const std::string& key, double value) override;
    void OverrideByteArray(const std::string& key, const std::byte* array, size_t size) override;

    bool IsOverridden(const std::string& key) const override;
    void RemoveOverride(const std::string& key) override;
    void ClearOverrides() override;

    bool Remove(const std::string& key, bool forceRemoval = false) override;
    void Clear(bool includeOverrides = false) override;
    std::vector<std::string> Keys(bool includeOverrides = false) const override;
    std::vector<std::string> ChildrenNames() const override;
    IPreferences* Parent() override;
    const IPreferences* Parent() const override;
    IPreferences* Root() override;
    const IPreferences* Root() const override;
    IPreferences* Node(const std::string& path) override;
    void RemoveNode() override;
    std::string Name() const override;
    std::string AbsolutePath() const override;
    void Flush() override;

    const Properties& GetProperties() const;
    const std::vector<std::unique_ptr<Preferences>>& GetChildren() const;

  private:
    template<typename T>
    void SetProperty(Properties& target, const std::string& key, const T& value, const std::function<std::string(const T&)>& toString)
    {
      const auto oldValue = target[key];
      const auto newValue = toString(value);
      target[key] = newValue;

      if (oldValue != newValue)
      {
        this->OnChanged(this);
        this->OnPropertyChanged(ChangeEvent(this, key, oldValue, newValue));
      }
    }

    std::optional<std::string> FindValue(const std::string& key) const;

    Properties m_Properties;
    Properties m_Overrides;
    std::vector<std::unique_ptr<Preferences>> m_Children;
    std::string m_Path;
    std::string m_Name;
    Preferences* m_Parent;
    Preferences* m_Root;
    IPreferencesStorage* m_Storage;
  };
}

#endif
