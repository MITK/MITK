#include "ConfigManager.h"

#include <unordered_map>
#include <unordered_set>
#include <string>

#include <boost/variant.hpp>

namespace config
{
  class ConfigManager::Impl
  {
  public:
    template <typename TValue>
    void Add(const char* key, TValue&& value);

    template <typename TValue>
    TValue Get(const char* key, TValue&& value) const;

    void Delete(const char* key);

    void Enumirate(IParamSaver& visitor) const;

  private:
    class Visitor;

    std::unordered_map<std::string, boost::variant<bool, int, double, QString>> m_data;
    std::unordered_set<std::string> m_deleted;
  };

  class ConfigManager::Impl::Visitor : public boost::static_visitor<>
  {
  public:
    Visitor(const char* key, IParamSaver& saver)
      : m_key(key)
      , m_saver(saver)
    {
    }

    template <typename TValue>
    void operator()(TValue&& value) const
    {
      m_saver.Put(m_key, value);
    }

  private:
    const char* m_key;
    IParamSaver& m_saver;
  };

  void ConfigManager::Impl::Delete(const char* key)
  {
    m_deleted.emplace(key);
  }

  template <typename TValue>
  void ConfigManager::Impl::Add(const char* key, TValue&& value)
  {
    if (!key) {
      return;
    }
    m_deleted.erase(key);
    const auto res = m_data.emplace(key, value);
    if (!res.second) {
      res.first->second = value;
    }
  }

  template <typename TValue>
  TValue ConfigManager::Impl::Get(const char* key, TValue&& value) const
  {
    if (!key) {
      return value;
    }
    const auto iter = m_data.find(key);
    return m_deleted.end() != m_deleted.find(key) || m_data.end() == iter ? value : boost::get<std::remove_reference<TValue>::type>(iter->second);
  }

  void ConfigManager::Impl::Enumirate(IParamSaver& visitor) const
  {
    for (auto& item : m_data) {
      if (m_deleted.end() == m_deleted.find(item.first)) {
        boost::apply_visitor(Visitor(item.first.c_str(), visitor), item.second);
      }
    }
  }

  ConfigManager& ConfigManager::GetInstance()
  {
    static ConfigManager instance;
    return instance;
  }

  ConfigManager::ConfigManager()
    : m_impl(new Impl())
  {
  }

  ConfigManager::~ConfigManager()
  {
  }

  void ConfigManager::PutString(const char* key, const QString& value)
  {
    m_impl->Add(key, value);
  }

  void ConfigManager::PutDouble(const char* key, double value)
  {
    m_impl->Add(key, value);
  }

  void ConfigManager::PutInt(const char* key, int value)
  {
    m_impl->Add(key, value);
  }

  void ConfigManager::PutBool(const char* key, bool value)
  {
    m_impl->Add(key, value);
  }

  const QString& ConfigManager::GetString(const char* key, const QString& value) const
  {
    return m_impl->Get(key, value);
  }

  double ConfigManager::GetDouble(const char* key, double value) const
  {
    return m_impl->Get(key, value);
  }

  int ConfigManager::GetInt(const char* key, int value) const
  {
    return m_impl->Get(key, value);
  }

  bool ConfigManager::GetBool(const char* key, bool value) const
  {
    return m_impl->Get(key, value);
  }

  void ConfigManager::SaveParams(IParamSaver& saver) const
  {
    m_impl->Enumirate(saver);
  }

  void ConfigManager::Delete(const char* key)
  {
    m_impl->Delete(key);
  }
}
