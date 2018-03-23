#! This CMake macro adds the necessary library and incllude
#! directories to a swig-project.
#!
#! params:
#!  swig_module : Name of the SWIG module, for example pyMITK
#!  library_names : Semicolon separated list of the libraries that are included, for example "MitkCore;mbilog"
#!


# function inspired by
# https://stackoverflow.com/questions/37205274/swig-and-cmake-make-use-of-information-provided-by-target-include-directories
# This function tells cmake which additional dependencies are existing
# especially with respect to the linker dependencies.
function(mitkSwigAddLibraryDependencies swig_module library_names)
    foreach(library_name ${library_names})
        # Adding each library as a linker dependency:
        swig_link_libraries(${swig_module} ${library_name})
        # Extracting all include directories from each given project and
        # then including these directories to the newly created swig project.
        get_property(LIBRARY_INCLUDES
                     TARGET ${library_name}
                     PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
        # Checking each given librarie to include all includes from this library.
        foreach(INCLUDE_PATH ${LIBRARY_INCLUDES})
            file(GLOB_RECURSE header_files "${INCLUDE_PATH}/*.h")
            list(APPEND SWIG_MODULE_${swig_module}_EXTRA_DEPS ${header_files})
            # export variable to parent scope
            set(SWIG_MODULE_${swig_module}_EXTRA_DEPS
              ${SWIG_MODULE_${swig_module}_EXTRA_DEPS} PARENT_SCOPE)
        endforeach()
    endforeach()

    # In addition include python dependencies:
    include_directories( ${PYTHON_INCLUDE_DIR})
    list(APPEND SWIG_MODULE_${swig_module}_EXTRA_DEPS ${PYTHON_INCLUDE_DIR})
    #swig_link_libraries(${swig_module} ${PYTHON_LIBRARIES} )

    # Add additional include paths, for example to the common files:
    list(APPEND SWIG_MODULE_${swig_module}_EXTRA_DEPS ${SWIG_EXTRA_DEPS})

    set(SWIG_MODULE_${swig_module}_EXTRA_DEPS
      ${SWIG_MODULE_${swig_module}_EXTRA_DEPS} PARENT_SCOPE)
endfunction()
