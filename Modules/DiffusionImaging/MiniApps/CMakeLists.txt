option(BUILD_DiffusionMiniApps "Build commandline tools for diffusion" OFF)

if(BUILD_DiffusionMiniApps OR MITK_BUILD_ALL_APPS)

  # needed include directories
  include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )


#    #TractogramAngularError.cpp


  mitk_create_executable(DwiDenoising
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES DwiDenoising.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(ImageResampler
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES ImageResampler.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(NetworkCreation
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking MitkConnectomics
    PACKAGE_DEPENDS ITK
    FILES NetworkCreation.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(NetworkStatistics
    DEPENDS MitkCore MitkDiffusionCore MitkConnectomics
    PACKAGE_DEPENDS ITK
    FILES NetworkStatistics.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(ExportShImage
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES ExportShImage.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(Fiberfox
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES Fiberfox.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(MultishellMethods
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES MultishellMethods.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(PeaksAngularError
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES PeaksAngularError.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(PeakExtraction
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES PeakExtraction.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(FiberDirectionExtraction
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES FiberDirectionExtraction.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(LocalDirectionalFiberPlausibility
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES LocalDirectionalFiberPlausibility.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(StreamlineTracking
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES StreamlineTracking.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(GibbsTracking
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES GibbsTracking.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(CopyGeometry
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES CopyGeometry.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(DiffusionIndices
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES DiffusionIndices.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(TractometerMetrics
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking
    PACKAGE_DEPENDS ITK
    FILES TractometerMetrics.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(QballReconstruction
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES QballReconstruction.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(Registration
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES mitkRegistration.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(FileFormatConverter
    DEPENDS MitkCore MitkDiffusionCore MitkFiberTracking MitkDiffusionIO
    PACKAGE_DEPENDS ITK
    FILES mitkFileFormatConverter.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(TensorReconstruction
    DEPENDS MitkCore MitkDiffusionCore MitkDiffusionIO
    PACKAGE_DEPENDS ITK
    FILES TensorReconstruction.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(TensorDerivedMapExtraction
    DEPENDS MitkCore MitkDiffusionCore MitkDiffusionIO
    PACKAGE_DEPENDS ITK
    FILES TensorDerivedMapsExtraction.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(DICOMLoader
    DEPENDS MitkCore MitkDiffusionCore
    PACKAGE_DEPENDS ITK
    FILES DICOMLoader.cpp mitkCommandLineParser.cpp
  )

  mitk_create_executable(Dicom2Nrrd
    DEPENDS MitkCore
    FILES Dicom2Nrrd.cpp mitkCommandLineParser.cpp
  )

  if(EXECUTABLE_IS_ENABLED)
    MITK_INSTALL_TARGETS(EXECUTABLES ${EXECUTABLE_TARGET})
  endif()

endif()
