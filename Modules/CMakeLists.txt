SET(LIBPOSTFIX "Ext")

SET(module_dirs
    SceneSerializationBase
    PlanarFigure
    ImageExtraction
    ImageStatistics
    LegacyAdaptors
    IpPicSupport
    MitkExt
    SceneSerialization
    QmitkExt
    GraphAlgorithms
    DiffusionImaging
    GPGPU
    IGT
    CameraCalibration
    IGTUI
    RigidRegistration
    RigidRegistrationUI
    DeformableRegistration
    DeformableRegistrationUI
    OpenCVVideoSupport
    Overlays
    InputDevices
    ToFHardware
    ToFProcessing
    ToFUI
	ClippingTools
)

SET(MITK_DEFAULT_SUBPROJECTS MITK-Modules)

FOREACH(module_dir ${module_dirs})
  ADD_SUBDIRECTORY(${module_dir})
ENDFOREACH()

IF(MITK_PRIVATE_MODULES)
  FILE(GLOB all_subdirs RELATIVE ${MITK_PRIVATE_MODULES} ${MITK_PRIVATE_MODULES}/*)
  FOREACH(subdir ${all_subdirs})
    STRING(FIND ${subdir} "." _result)
    IF(_result EQUAL -1)
      IF(EXISTS ${MITK_PRIVATE_MODULES}/${subdir}/CMakeLists.txt)
        MESSAGE(STATUS "Found private module ${subdir}")    
        ADD_SUBDIRECTORY(${MITK_PRIVATE_MODULES}/${subdir} private_modules/${subdir})
    ENDIF()
    ENDIF()
  ENDFOREACH()     
ENDIF(MITK_PRIVATE_MODULES)

