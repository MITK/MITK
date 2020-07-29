include(${CMAKE_CURRENT_LIST_DIR}/Minimal.cmake)

list(APPEND enabled_modules
  AlgorithmsExt
  Annotation
  CommandLine
  ContourModel
  CoreCmdApps
  DataTypesExt
  DICOMPM
  DICOMPMIO
  DICOMQI
  DICOM
  DICOMImageIO
  RT
  DICOMRTIO
  DICOMSegIO
  ModelFit
  Multilabel
  MultilabelIO
  LegacyGL
  SceneSerialization
  SceneSerializationBase
)

