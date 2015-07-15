project(org_mitk_gui_qt_diffusionimaging)

# Enable OpenMP support
find_package(OpenMP)
if(NOT OPENMP_FOUND)
   message("Diffusion Plugin: OpenMP is not available.")
endif()
if(OPENMP_FOUND)
  message(STATUS "Diffusion Plugin: Found OpenMP.")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

mitk_create_plugin(
  EXPORT_DIRECTIVE DIFFUSIONIMAGING_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkConnectomics MitkFiberTracking MitkDiffusionCore MitkQuantification
  PACKAGE_DEPENDS PUBLIC ITK|ITKDiffusionTensorImage Vigra HDF5
)
