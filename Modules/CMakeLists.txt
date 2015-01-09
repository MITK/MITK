set(MITK_DEFAULT_SUBPROJECTS MITK-Modules)
set(MITK_LEGACY_EXPORT_MACRO_NAME 1)

include(ModuleList.cmake)
mitkFunctionWhitelistModules(MITK mitk_modules)
