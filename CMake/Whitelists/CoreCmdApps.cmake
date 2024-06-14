include(${CMAKE_CURRENT_LIST_DIR}/Minimal.cmake)

list(APPEND enabled_modules
  AlgorithmsExt
  Annotation
  CommandLine
  ContourModel
  CoreCmdApps
  DataTypesExt
  DICOMPM
  DICOMQI
  DICOM
  RT
  ModelFit
  Multilabel
  LegacyGL
  SceneSerialization
  SceneSerializationBase
  ROI
)
