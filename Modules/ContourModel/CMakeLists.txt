MITK_CREATE_MODULE( ContourModel
  INCLUDE_DIRS Algorithms DataManagement IO Rendering
  DEPENDS Mitk
  PACKAGE_DEPENDS ITK|ITKReview
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
