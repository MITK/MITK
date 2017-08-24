#pragma once

#include <vector>

#include <MitkUtilitiesExports.h>

namespace mitk {

class MITKUTILITIES_EXPORT TranslationUtilities
{
public:
  static std::vector<std::string> getTranslatedTransferFunctionPresets();
  static std::string getDefaultUserPresetName();

private:
  TranslationUtilities() {};
  ~TranslationUtilities() {};

  TranslationUtilities(TranslationUtilities&) = delete;
  TranslationUtilities& operator =(TranslationUtilities&) = delete;
};

}
