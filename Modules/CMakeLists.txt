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
# bug-8977-IpPicFree : Deactivating temporarily ToF
#   ToFHardware
#   ToFProcessing
#   ToFUI
)

SET(MITK_DEFAULT_SUBPROJECTS MITK-Modules)

FOREACH(module_dir ${module_dirs})
  ADD_SUBDIRECTORY(${module_dir})
ENDFOREACH()
