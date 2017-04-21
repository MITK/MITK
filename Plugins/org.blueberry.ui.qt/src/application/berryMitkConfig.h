#pragma once

namespace berry
{
  struct MitkConfig
  {
    MitkConfig();

    ~MitkConfig();

    MitkConfig(const MitkConfig&) = delete;
    MitkConfig& operator=(const MitkConfig&) = delete;
  };
}