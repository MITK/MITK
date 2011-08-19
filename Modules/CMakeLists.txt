SET(LIBPOSTFIX "Ext")

SET(module_dirs
    SceneSerializationBase
    PlanarFigure
    ImageExtraction
    ImageStatistics
    MitkExt
    SceneSerialization
    QmitkExt
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
)

SET(MITK_DEFAULT_SUBPROJECTS MITK-Modules)

FOREACH(module_dir ${module_dirs})
  ADD_SUBDIRECTORY(${module_dir})
ENDFOREACH()
