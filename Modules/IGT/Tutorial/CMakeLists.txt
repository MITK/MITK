add_executable(MitkIGTTutorialStep1 mitkIGTTutorialStep1.cpp)
mitk_use_modules(TARGET MitkIGTTutorialStep1 MODULES MitkIGT MitkDataTypesExt)

add_executable(MitkIGTTutorialStep2 mitkIGTTutorialStep2.cpp)
mitk_use_modules(TARGET MitkIGTTutorialStep2 MODULES MitkIGT MitkDataTypesExt)

set_property(TARGET MitkIGTTutorialStep1 MitkIGTTutorialStep2 PROPERTY FOLDER "${MITK_ROOT_FOLDER}/Modules/Executables")
