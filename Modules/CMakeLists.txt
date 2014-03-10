
# Modules must be listed according to their dependencies
set(module_dirs
    DataTypesExt
    AlgorithmsExt
    MapperExt
    IOExt
    Qt4Qt5TestModule
    SceneSerializationBase
    PlanarFigure
    ImageDenoising
    ImageExtraction
    ImageStatistics
    LegacyAdaptors
    IpPicSupport
    Ext
    SceneSerialization
    GraphAlgorithms
    ContourModel
    SurfaceInterpolation
    Segmentation
    PlanarFigureSegmentation
    OpenViewCore
    QmlItems
    QtWidgets
    QtWidgetsExt
    SegmentationUI
    DiffusionImaging
    GPGPU
    IGTBase
    IGT
    CameraCalibration
    IGTUI
    RigidRegistration
    RigidRegistrationUI
    DeformableRegistration
    DeformableRegistrationUI
    OpenCL
    OpenCVVideoSupport
    Overlays
    InputDevices
    ToFHardware
    ToFProcessing
    ToFUI
    US
    ClippingTools
    USUI
    DicomUI
    Simulation
    Remeshing
    Python
    Persistence
    SurfaceRegistration
)

if(MITK_ENABLE_PIC_READER)
  list(APPEND module_dirs IpPicSupportIO)
endif()

set(MITK_DEFAULT_SUBPROJECTS MITK-Modules)

foreach(module_dir ${module_dirs})
  add_subdirectory(${module_dir})
endforeach()

if(MITK_PRIVATE_MODULES)
  file(GLOB all_subdirs RELATIVE ${MITK_PRIVATE_MODULES} ${MITK_PRIVATE_MODULES}/*)
  foreach(subdir ${all_subdirs})
    string(FIND ${subdir} "." _result)
    if(_result EQUAL -1)
      if(EXISTS ${MITK_PRIVATE_MODULES}/${subdir}/CMakeLists.txt)
        message(STATUS "Found private module ${subdir}")
        add_subdirectory(${MITK_PRIVATE_MODULES}/${subdir} private_modules/${subdir})
    endif()
    endif()
  endforeach()
endif(MITK_PRIVATE_MODULES)

