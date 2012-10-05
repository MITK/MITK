# Set Diffusion specific CPack options

set(CPACK_PACKAGE_EXECUTABLES "mitkDiffusion;MITK Diffusion")
set(CPACK_PACKAGE_NAME "MITK-Diffusion")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MITK application for processing of MR diffusion imaging data.")
# Major version is the year of release
set(CPACK_PACKAGE_VERSION_MAJOR "2012")
# Minor version is assigned depending on the number of releases that year, order is "", "a", "b"..., "z"
set(CPACK_PACKAGE_VERSION_MINOR "")
# Patch versioning is not intended to be done
set(CPACK_PACKAGE_VERSION_PATCH "")

# this should result in names like 2011, 2011a, ...
# version has to be set explicitly to avoid such things as CMake creating the install directory "MITK Diffusion 2011.."
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}${CPACK_PACKAGE_VERSION_MINOR}")

set(CPACK_PACKAGE_FILE_NAME "MITK-Diffusion-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCH}")

