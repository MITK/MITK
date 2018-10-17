
set(TOOL_CPPS "")

# temporary suppress warnings in the following files until image accessors are fully integrated.
set_source_files_properties( src/DataManagement/mitkImage.cpp COMPILE_FLAGS -DMITK_NO_DEPRECATED_WARNINGS )
set_source_files_properties( src/Controllers/mitkSliceNavigationController.cpp COMPILE_FLAGS -DMITK_NO_DEPRECATED_WARNINGS )

MITK_CREATE_MODULE(
  INCLUDE_DIRS
    PUBLIC ${MITK_BINARY_DIR}
    PRIVATE src/Algorithms src/Controllers src/DataManagement src/Interactions src/IO src/Rendering ${OPENGL_INCLUDE_DIR}
  DEPENDS PUBLIC mbilog CppMicroServices
  PACKAGE_DEPENDS
    PRIVATE tinyxml OpenGL
    PUBLIC  ITK|ITKTransform+ITKImageGrid+ITKImageFeature+ITKIOImageBase+ITKIOHDF5+ITKIOLSM+ITKIOMRC+ITKIOBioRad+ITKIOGE+ITKIOStimulate+ITKIOBruker+ITKIOMINC
    # We privately use/link all ITK modules in order to support all IO, Transform, etc.
    # factories from ITK which are registered "automatically" via a factory manager.
    PRIVATE ITK
    PUBLIC  VTK|vtkFiltersTexture+vtkFiltersParallel+vtkImagingStencil+vtkImagingMath+vtkInteractionStyle+vtkRenderingOpenGL2+vtkRenderingContextOpenGL2+vtkRenderingVolumeOpenGL2+vtkRenderingFreeType+vtkRenderingLabel+vtkInteractionWidgets+vtkIOGeometry+vtkIOXML
    PUBLIC  Boost|boost
  SUBPROJECTS MITK-Core
  # Do not automatically create CppMicroServices initialization code.
  # Because the VTK 6 "auto-init" functionality injects file-local static
  # initialization code in every cpp file which includes a VTK header,
  # static initialization order becomes an issue again. For the Mitk
  # core library, we need to ensure that the VTK static initialization stuff
  # happens before the CppMicroServices initialization, since the latter
  # might already use VTK code which needs to access VTK object factories.
  # Hence, CppMicroServices initialization code is placed manually within
  # the mitkCoreActivator.cpp file.
  NO_INIT
)
if(NOT TARGET ${MODULE_TARGET})
  message(SEND_ERROR "Core target ${MODULE_TARGET} does not exist")
endif()
function(_itk_create_factory_register_manager)
  # In MITK_ITK_Config.cmake, we do *not* include ITK_USE_FILE, which
  # prevents multiple registrations/unregistrations of ITK IO factories
  # during library loading/unloading (of MITK libraries). However, we need
  # "one" place where the IO factories are registered at
  # least once. This could be the application executable, but every executable would
  # need to take care of that itself. Instead, we allow the auto registration in the
  # Mitk Core library.
  set(NO_DIRECTORY_SCOPED_ITK_COMPILE_DEFINITION 1)
  find_package(ITK)
  include(${ITK_USE_FILE})

  if(NOT ITK_NO_IO_FACTORY_REGISTER_MANAGER)
    # We manually add the define which will be of target scope. MITK
    # patches ITK_USE_FILE to remove the directory scoped compile
    # definition since it would be propagated to other targets in the
    # same directory scope but these targets might want to *not*
    # use the ITK factory manager stuff.
    target_compile_definitions(${MODULE_TARGET} PRIVATE ITK_IO_FACTORY_REGISTER_MANAGER)
  endif()
endfunction()
_itk_create_factory_register_manager()

if(MSVC_IDE OR MSVC_VERSION)
  target_link_libraries(${MODULE_TARGET} PRIVATE psapi.lib)
endif()

add_subdirectory(TestingHelper)

add_subdirectory(test)
