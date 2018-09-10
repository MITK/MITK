# Set Diffusion specific CPack options

set(CPACK_PACKAGE_EXECUTABLES "MitkDiffusion;MITK Diffusion")
set(CPACK_PACKAGE_NAME "MITK-Diffusion")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "MITK application for processing of MR diffusion imaging data.")
# Major version is the year of release
set(CPACK_PACKAGE_VERSION_MAJOR "2018")
# Minor version is the month of release
set(CPACK_PACKAGE_VERSION_MINOR "09")
# Patch versioning is not intended to be done
set(CPACK_PACKAGE_VERSION_PATCH "99")

# this should result in names like 2011.09, 2012.06, ...
# version has to be set explicitly to avoid such things as CMake creating the install directory "MITK Diffusion 2011.."
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
#set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")#.${CPACK_PACKAGE_VERSION_PATCH}")

set(CPACK_PACKAGE_FILE_NAME "MITK-Diffusion-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCH}")

