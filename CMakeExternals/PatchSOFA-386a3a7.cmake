# Changes of cmake/dependencies.cmake are commented separately below

set(path "cmake/dependencies.cmake")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)

# Comment out add_subdirectory() commands that reference the SOFA applications
# directory

string(REPLACE "add_subdirectory(\"\${SOFA_A" "#add_subdirectory(\"\${SOFA_A" CONTENTS ${CONTENTS})

# Rename tinyxml to SofaTinyXml since MITK has its own version of tinyxml and
# the same name lead to linker or runtime errors in the past

string(REPLACE "\"tinyxml\"" "\"SofaTinyXml\"" CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Adjust tinyxml references (see above)

set(path "framework/sofa/helper/CMakeLists.txt")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

set(path "modules/sofa/component/SofaBaseVisual/CMakeLists.txt")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

set(path "modules/sofa/component/SofaLoader/CMakeLists.txt")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

set(path "modules/sofa/simulation/common/CMakeLists.txt")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Changes of cmake/environment.cmake are commented separately below

set(path "cmake/environment.cmake")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)

# Allow setting SOFA_APPLICATIONS_PLUGINS_DIR from outside

string(REPLACE "S_DIR}/plugins\" CACHE INTERNAL" "S_DIR}/plugins\" CACHE PATH" CONTENTS ${CONTENTS})

# Place binaries and libraries in different subfolders according to the build
# configuration

string(REPLACE "DEBUG \"\${directory}" "DEBUG \"\${directory}/Debug" CONTENTS ${CONTENTS})
string(REPLACE "RELEASE \"\${directory}" "RELEASE \"\${directory}/Release" CONTENTS ${CONTENTS})
string(REPLACE "RELWITHDEBINFO \"\${directory}" "RELWITHDEBINFO \"\${directory}/RelWithDebInfo" CONTENTS ${CONTENTS})
string(REPLACE "MINSIZEREL \"\${directory}" "MINSIZEREL \"\${directory}/MinSizeRel" CONTENTS ${CONTENTS})

configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Binary directories in add_subdirectory() regarding external plugins have to be explicitly specified

set(path "cmake/functions.cmake")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)
string(REPLACE "add_subdirectory(\"\${GLOBAL_PROJECT_PATH_\${projectName}}\")"
"if(\${GLOBAL_PROJECT_PATH_\${projectName}} MATCHES ^\${CMAKE_SOURCE_DIR}.*)
                      add_subdirectory(\"\${GLOBAL_PROJECT_PATH_\${projectName}}\")
                    else()
                      string(LENGTH \"\${SOFA_APPLICATIONS_PLUGINS_DIR}\" begin)
                      string(SUBSTRING \"\${GLOBAL_PROJECT_PATH_\${projectName}}\" \${begin} -1 subdir)
                      add_subdirectory(\"\${GLOBAL_PROJECT_PATH_\${projectName}}\" \"\${CMAKE_BINARY_DIR}/applications/plugins\${subdir}\")
                    endif()"
CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Remove SOFA's FindGLEW.cmake since we need the official FindGLEW.cmake to
# inject the MITK superbuild version of GLEW into SOFA

file(REMOVE "cmake/Modules/FindGLEW.cmake")

# Changes of cmake/externals.cmake are commented separately below

set(path "cmake/externals.cmake")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)

# Enhance GLUT and GLEW finding routine to regard externally injected GLUT and
# GLEW libraries

string(REPLACE "set(GLUT_LIBRARIES \"freeglut\")" "find_package(GLUT REQUIRED)" CONTENTS ${CONTENTS})
string(REPLACE "set(GLEW_LIBRARIES \"glew32\")" "find_package(GLEW REQUIRED)" CONTENTS ${CONTENTS})
string(REPLACE "else()\n    set(O" "    list(APPEND GLOBAL_INCLUDE_DIRECTORIES \"\${GLUT_INCLUDE_DIRS}\" \"\${GLEW_INCLUDE_DIRS}\")\nelse()\n    set(O" CONTENTS ${CONTENTS})
string(REPLACE "GLUT REQUIRED" "GLUT REQUIRED CONFIG" CONTENTS ${CONTENTS})
string(REPLACE "GLEW REQUIRED" "GLEW REQUIRED CONFIG" CONTENTS ${CONTENTS})

# Remove Boost binary graph dependency because it is not straight forward to
# compile on all plattforms and its header-only part is enough

string(REPLACE "graph " "" CONTENTS ${CONTENTS})
configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Changes of CMakeLists.txt are commented separately below

set(path "CMakeLists.txt")
file(STRINGS ${path} CONTENTS NEWLINE_CONSUME)

# Set CMake policies to prevent configure warnings

string(REPLACE "set(SOLUTION_NAME" "if(POLICY CMP0039)
  cmake_policy(SET CMP0039 OLD)
endif()

if(POLICY CMP0043)
  cmake_policy(SET CMP0043 OLD)
  endif()

if(POLICY CMP0054)
  cmake_policy(SET CMP0054 OLD)
endif()

set(SOLUTION_NAME"
CONTENTS ${CONTENTS})

# Use configure file (see below)

string(REPLACE "\nendif()" "\nendif()\n\nconfigure_file(SOFAConfig.cmake.in SOFAConfig.cmake @ONLY)" CONTENTS ${CONTENTS})

configure_file(${TEMPLATE_FILE} ${path} @ONLY)

# Create SOFAConfig.cmake.in file to make SOFA findable through the config mode
# of find_package()

file(WRITE "SOFAConfig.cmake.in"
"add_definitions(-DSOFA_XML_PARSER_TINYXML;-DTIXML_USE_STL;-DMINI_FLOWVR;-DSOFA_HAVE_BOOST;-DSOFA_HAVE_CSPARSE;-DSOFA_HAVE_DAG;-DSOFA_HAVE_EIGEN2;-DSOFA_HAVE_FREEGLUT;-DSOFA_HAVE_GLEW;-DSOFA_HAVE_METIS)

set(SOFA_INCLUDE_DIRS \"@SOFA_EXTLIBS_DIR@/csparse;@SOFA_EXTLIBS_DIR@/eigen-3.2.1;@SOFA_EXTLIBS_DIR@/metis-5.1.0/include;@SOFA_EXTLIBS_DIR@/miniFlowVR/include;@SOFA_EXTLIBS_DIR@/newmat;@SOFA_EXTLIBS_DIR@/tinyxml;@SOFA_SRC_DIR@/framework;@SOFA_SRC_DIR@/modules\")

if(WIN32)
  set(SOFA_LIBRARY_DIRS \"@SOFA_LIB_DIR@\")
  set(version \"_1_0\")
else()
  set(SOFA_LIBRARY_DIRS \"@SOFA_LIB_DIR@/@CMAKE_BUILD_TYPE@\")
  set(version \"\")
endif()

set(SOFA_LIBRARIES
  debug SofaBaseAnimationLoop\${version}d optimized SofaBaseAnimationLoop\${version}
  debug SofaBaseCollision\${version}d optimized SofaBaseCollision\${version}
  debug SofaBaseLinearSolver\${version}d optimized SofaBaseLinearSolver\${version}
  debug SofaBaseMechanics\${version}d optimized SofaBaseMechanics\${version}
  debug SofaBaseTopology\${version}d optimized SofaBaseTopology\${version}
  debug SofaBaseVisual\${version}d optimized SofaBaseVisual\${version}
  debug SofaBoundaryCondition\${version}d optimized SofaBoundaryCondition\${version}
  debug SofaComponentAdvanced\${version}d optimized SofaComponentAdvanced\${version}
  debug SofaComponentBase\${version}d optimized SofaComponentBase\${version}
  debug SofaComponentCommon\${version}d optimized SofaComponentCommon\${version}
  debug SofaComponentGeneral\${version}d optimized SofaComponentGeneral\${version}
  debug SofaComponentMain\${version}d optimized SofaComponentMain\${version}
  debug SofaComponentMisc\${version}d optimized SofaComponentMisc\${version}
  debug SofaConstraint\${version}d optimized SofaConstraint\${version}
  debug SofaCore\${version}d optimized SofaCore\${version}
  debug SofaDefaultType\${version}d optimized SofaDefaultType\${version}
  debug SofaDeformable\${version}d optimized SofaDeformable\${version}
  debug SofaDenseSolver\${version}d optimized SofaDenseSolver\${version}
  debug SofaEigen2Solver\${version}d optimized SofaEigen2Solver\${version}
  debug SofaEngine\${version}d optimized SofaEngine\${version}
  debug SofaEulerianFluid\${version}d optimized SofaEulerianFluid\${version}
  debug SofaExplicitOdeSolver\${version}d optimized SofaExplicitOdeSolver\${version}
  debug SofaExporter\${version}d optimized SofaExporter\${version}
  debug SofaGraphComponent\${version}d optimized SofaGraphComponent\${version}
  debug SofaHaptics\${version}d optimized SofaHaptics\${version}
  debug SofaHelper\${version}d optimized SofaHelper\${version}
  debug SofaImplicitOdeSolver\${version}d optimized SofaImplicitOdeSolver\${version}
  debug SofaLoader\${version}d optimized SofaLoader\${version}
  debug SofaMeshCollision\${version}d optimized SofaMeshCollision\${version}
  debug SofaMisc\${version}d optimized SofaMisc\${version}
  debug SofaMiscCollision\${version}d optimized SofaMiscCollision\${version}
  debug SofaMiscEngine\${version}d optimized SofaMiscEngine\${version}
  debug SofaMiscFem\${version}d optimized SofaMiscFem\${version}
  debug SofaMiscForceField\${version}d optimized SofaMiscForceField\${version}
  debug SofaMiscMapping\${version}d optimized SofaMiscMapping\${version}
  debug SofaMiscSolver\${version}d optimized SofaMiscSolver\${version}
  debug SofaMiscTopology\${version}d optimized SofaMiscTopology\${version}
  debug SofaNonUniformFem\${version}d optimized SofaNonUniformFem\${version}
  debug SofaObjectInteraction\${version}d optimized SofaObjectInteraction\${version}
  debug SofaOpenglVisual\${version}d optimized SofaOpenglVisual\${version}
  debug SofaPreconditioner\${version}d optimized SofaPreconditioner\${version}
  debug SofaRigid\${version}d optimized SofaRigid\${version}
  debug SofaSimpleFem\${version}d optimized SofaSimpleFem\${version}
  debug SofaSimulationCommon\${version}d optimized SofaSimulationCommon\${version}
  debug SofaSimulationGraph\${version}d optimized SofaSimulationGraph\${version}
  debug SofaSimulationTree\${version}d optimized SofaSimulationTree\${version}
  debug SofaSparseSolver\${version}d optimized SofaSparseSolver\${version}
  debug SofaSphFluid\${version}d optimized SofaSphFluid\${version}
  debug SofaTopologyMapping\${version}d optimized SofaTopologyMapping\${version}
  debug SofaUserInteraction\${version}d optimized SofaUserInteraction\${version}
  debug SofaValidation\${version}d optimized SofaValidation\${version}
  debug SofaVolumetricData\${version}d optimized SofaVolumetricData\${version}
  debug SofaTinyXml\${version}d optimized SofaTinyXml\${version}
)")
