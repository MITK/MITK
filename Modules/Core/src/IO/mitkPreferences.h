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
#include <unordered_map>

namespace mitk
{
  class IPreferencesStorage;

  /**
   * \brief Implementation of the IPreferences interface.
   *
   * Only used through the IPreferences interface.
   *
   * \sa IPreferences
   */
  class Preferences : public IPreferences
  {
  public:
    using Properties = std::unordered_map<std::string, std::string>;

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

    void Remove(const std::string& key) override;
    void Clear() override;
    std::vector<std::string> Keys() const override;
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
    void Put(const std::string& key, const T& value, const std::function<std::string(const T&)>& toString)
    {
      const auto oldValue = m_Properties[key];
      const auto newValue = toString(value);
      m_Properties[key] = newValue;

      if (oldValue != newValue)
      {
        this->OnChanged(this);
        this->OnPropertyChanged(ChangeEvent(this, key, oldValue, newValue));
      }
    }

    Properties m_Properties;
    std::vector<std::unique_ptr<Preferences>> m_Children;
    std::string m_Path;
    std::string m_Name;
    Preferences* m_Parent;
    Preferences* m_Root;
    IPreferencesStorage* m_Storage;
  };
}

#endif
