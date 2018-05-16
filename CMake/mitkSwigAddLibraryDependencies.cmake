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

    endforeach()

    # In addition include python dependencies:
    include_directories( ${PYTHON_INCLUDE_DIR})

endfunction()
