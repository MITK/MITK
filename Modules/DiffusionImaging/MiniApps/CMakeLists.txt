option(BUILD_DiffusionMiniApps "Build commandline tools for diffusion" OFF)

if(BUILD_DiffusionMiniApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

#  project( MitkDiffusionMiniApps )

#  # fill in the standalone executables here
#  set(DIFFUSIONMINIAPPS
#    mitkDiffusionMiniApps
#  )

  # set additional files here
#  set(DIFFUSIONCORE_ADDITIONAL_FILES

#    TensorReconstruction.cpp
#    QballReconstruction.cpp
#    DiffusionIndices.cpp
#    CopyGeometry.cpp
#    GibbsTracking.cpp
#    StreamlineTracking.cpp
#    FiberProcessing.cpp
#    LocalDirectionalFiberPlausibility.cpp
#    #TractogramAngularError.cpp
#    FiberDirectionExtraction.cpp
#    PeakExtraction.cpp
#    PeaksAngularError.cpp
#    MultishellMethods.cpp
#    Fiberfox.cpp
#    ExportShImage.cpp
#    NetworkCreation.cpp
#    NetworkStatistics.cpp
#    DwiDenoising.cpp
#    TractometerMetrics.cpp
#  )


  mitk_create_executable(Registration
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES_CMAKE Registration.cmake
  )


  mitk_create_executable(FileFormatConverter
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking MitkDiffusionIO
    PACKAGE_DEPENDS ITK
    FILES_CMAKE FileFormat.cmake
  )

  mitk_create_executable(TensorReconstruction
    DEPENDS MitkCore MitkDiffusionCore MitkDiffusionIO
    PACKAGE_DEPENDS ITK
    FILES_CMAKE TensorReconstruction.cmake
  )

  mitk_create_executable(TensorDerivedMapExtraction
    DEPENDS MitkCore MitkDiffusionCore MitkDiffusionIO
    PACKAGE_DEPENDS ITK
    FILES_CMAKE TensorDerivedMapsExtraction.cmake
  )

  mitk_create_executable(DICOMLoader
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES_CMAKE DICOMLoader.cmake
  )


  mitk_create_executable(Dicom2Nrrd
    DEPENDS MitkCore
    FILES_CMAKE Dicom2Nrrd.cmake
  )

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES ${EXECUTABLE_TARGET})
  endif()

endif()
