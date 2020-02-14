message(STATUS "Creating install manifest")

file(GLOB installed_binaries "lib/boost_*.*" "lib/libboost_*.*" "bin/boost_*.*")
file(GLOB_RECURSE installed_headers "include/boost/*")

set(install_manifest "src/Boost-build/install_manifest.txt")
file(WRITE ${install_manifest} "")

foreach(f ${installed_binaries} ${installed_headers})
  file(APPEND ${install_manifest} "${f}\n")
endforeach()
