#pragma once

#include <memory>
#include <functional>

#include <boost/noncopyable.hpp>

#include <QString>

#include <MitkConfigExports.h>

#ifndef INTERNAL_PARAM_PREFIX
  #define INTERNAL_PARAM_PREFIX "__"
#endif

namespace config
{
  struct IParamSaver : private boost::noncopyable
  {
    virtual ~IParamSaver() {}

    virtual void Put(const char* key, const QString& value) = 0;
    virtual void Put(const char* key, double value) = 0;
    virtual void Put(const char* key, int value) = 0;
    virtual void Put(const char* key, bool value) = 0;
  };

  class MITKCONFIG_EXPORT ConfigManager : private boost::noncopyable
  {
  public:
    static ConfigManager& GetInstance();

    void PutString(const char* key, const QString& value);
    void PutDouble(const char* key, double value);
    void PutInt(const char* key, int value);
    void PutBool(const char* key, bool value);

    const QString& GetString(const char* key, const QString& value) const;
    double GetDouble(const char* key, double value) const;
    int GetInt(const char* key, int value) const;
    bool GetBool(const char* key, bool value) const;

    template <typename TValue>
    void PutEnum(const char* key, TValue value)
    {
      return PutInt(key, static_cast<int>(value));
    }

    template <typename TValue>
    TValue GetEnum(const char* key, TValue value) const
    {
      return static_cast<TValue>(GetInt(key, static_cast<int>(value)));
    }

    void Delete(const char* key);

    void SaveParams(IParamSaver& saver) const;

  private:
    class Impl;

    ConfigManager();
    ~ConfigManager();

    std::unique_ptr<Impl> m_impl;
  };
}
