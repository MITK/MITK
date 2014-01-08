project(QuickRender)
find_package(MITK REQUIRED)

mitk_create_executable(${PROJECT_NAME} DEPENDS QmlMitk)
