MITK_USE_MODULE(MitkIGT MitkDataTypesExt)

add_executable(mitkIGTTutorialStep1 mitkIGTTutorialStep1.cpp)
target_link_libraries(mitkIGTTutorialStep1 ${ALL_LIBRARIES})

add_executable(mitkIGTTutorialStep2 mitkIGTTutorialStep2.cpp)
target_link_libraries(mitkIGTTutorialStep2 ${ALL_LIBRARIES})
