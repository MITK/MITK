#include "berryMitkConfig.h"

#include <berryPlatform.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>

#include <ConfigManager.h>

namespace
{
  const char MITK_CONFIG_PARAMS[] = "mitk_config_params";
  const char PARAMS_COUNT[] = "count";
  const char PARAM_NODE[] = "param_%1";
  const char PARAM_KEY[] = "key";
  const char PARAM_TYPE[] = "type";
  const char PARAM_VALUE[] = "value";

  enum ParamType
  {
    PARAM_UNKNOWN,
    PARAM_STRING,
    PARAM_DOUBLE,
    PARAM_INTEGER,
    PARAM_BOOLEAN
  };

  struct ConfigSaver : config::IParamSaver
  {
    explicit ConfigSaver(berry::IPreferences::Pointer prefsNode)
      : m_prefsNode(prefsNode)
      , m_count(0)
    {
    }

    int Count() const
    {
      return m_count;
    }

    virtual void Put(const char* key, const QString& value)
    {
      if (auto node = GetNode(key, PARAM_STRING)) {
        node->Put(PARAM_VALUE, value);
      }
    }

    virtual void Put(const char* key, double value)
    {
      if (auto node = GetNode(key, PARAM_DOUBLE)) {
        node->PutDouble(PARAM_VALUE, value);
      }
    }

    virtual void Put(const char* key, int value)
    {
      if (auto node = GetNode(key, PARAM_INTEGER)) {
        node->PutInt(PARAM_VALUE, value);
      }
    }

    virtual void Put(const char* key, bool value)
    {
      if (auto node = GetNode(key, PARAM_BOOLEAN)) {
        node->PutBool(PARAM_VALUE, value);
      }
    }

  private:
    berry::IPreferences::Pointer GetNode(const char* key, ParamType type)
    {
      if (!key) {
        return berry::IPreferences::Pointer();
      }
      auto node = m_prefsNode->Node(QString(PARAM_NODE).arg(m_count++));
      if (node) {
        node->Put(PARAM_KEY, key);
        node->PutInt(PARAM_TYPE, type);
      }
      return node;
    }

    berry::IPreferences::Pointer m_prefsNode;
    int m_count;
  };
}

namespace berry
{
  MitkConfig::MitkConfig()
  {
    if (auto prefService = berry::Platform::GetPreferencesService()) {
      if (auto prefsNode = prefService->GetSystemPreferences()->Node(MITK_CONFIG_PARAMS)) {
        const auto count = prefsNode->GetInt(PARAMS_COUNT, 0);
        auto& manager = config::ConfigManager::GetInstance();
        for (auto i = 0; i < count; ++i) {
          if (auto node = prefsNode->Node(QString(PARAM_NODE).arg(i))) {
            const auto key = node->Get(PARAM_KEY, QString());
            if (key.isEmpty()) {
              continue;
            }
            const auto type = node->GetInt(PARAM_TYPE, PARAM_UNKNOWN);
            switch (type) {
            case PARAM_STRING:
              manager.PutString(key.toStdString().c_str(), node->Get(PARAM_VALUE, QString()));
              break;
            case PARAM_DOUBLE:
              manager.PutDouble(key.toStdString().c_str(), node->GetDouble(PARAM_VALUE, 0));
              break;
            case PARAM_INTEGER:
              manager.PutInt(key.toStdString().c_str(), node->GetInt(PARAM_VALUE, 0));
              break;
            case PARAM_BOOLEAN:
              manager.PutBool(key.toStdString().c_str(), node->GetBool(PARAM_VALUE, 0));
              break;
            }
          }
        }
      }
    }
  }

  MitkConfig::~MitkConfig()
  {
    if (auto prefService = berry::Platform::GetPreferencesService()) {
      if (auto prefsNode = prefService->GetSystemPreferences()->Node(MITK_CONFIG_PARAMS)) {
        ConfigSaver saver(prefsNode);
        config::ConfigManager::GetInstance().SaveParams(saver);
        prefsNode->PutInt(PARAMS_COUNT, saver.Count());
      }
    }
  }
}