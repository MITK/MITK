file(STRINGS "framework/sofa/defaulttype/MapMapSparseMatrix.h" mapMapSparseMatrix_h NEWLINE_CONSUME)
string(REPLACE "_pair< KeyT, T >" "_pair" mapMapSparseMatrix_h "${mapMapSparseMatrix_h}")
string(REPLACE "r< KeyType, RowType >" "r" mapMapSparseMatrix_h "${mapMapSparseMatrix_h}")
set(CONTENTS ${mapMapSparseMatrix_h})
configure_file(${TEMPLATE_FILE} "framework/sofa/defaulttype/MapMapSparseMatrix.h" @ONLY)

file(STRINGS "sofa-dependencies.cmake" sofaDependencies_cmake NEWLINE_CONSUME)
string(REPLACE "\"tinyxml\"" "\"SofaTinyXml\"" sofaDependencies_cmake "${sofaDependencies_cmake}")
string(REPLACE "add_subdirectory(\"\${SOFA_A" "#add_subdirectory(\"\${SOFA_A" sofaDependencies_cmake "${sofaDependencies_cmake}")
file(WRITE "sofa-dependencies.cmake" "${sofaDependencies_cmake}")

file(STRINGS "modules/sofa/component/SofaBaseVisual/CMakeLists.txt" CMakeLists_txt NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CMakeLists_txt "${CMakeLists_txt}")
file(WRITE "modules/sofa/component/SofaBaseVisual/CMakeLists.txt" "${CMakeLists_txt}")

file(STRINGS "modules/sofa/component/SofaLoader/CMakeLists.txt" CMakeLists_txt NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CMakeLists_txt "${CMakeLists_txt}")
file(WRITE "modules/sofa/component/SofaLoader/CMakeLists.txt" "${CMakeLists_txt}")

file(STRINGS "modules/sofa/simulation/common/CMakeLists.txt" CMakeLists_txt NEWLINE_CONSUME)
string(REPLACE "tinyxml" "SofaTinyXml" CMakeLists_txt "${CMakeLists_txt}")
file(WRITE "modules/sofa/simulation/common/CMakeLists.txt" "${CMakeLists_txt}")

file(STRINGS "modules/sofa/component/SofaUserInteraction/CMakeLists.txt" CMakeLists_txt NEWLINE_CONSUME)
string(REPLACE "../collision/AddFramePerformer." "#../collision/AddFramePerformer." CMakeLists_txt "${CMakeLists_txt}")
file(WRITE "modules/sofa/component/SofaUserInteraction/CMakeLists.txt" "${CMakeLists_txt}")

file(REMOVE "cmake/FindGLEW.cmake")

file(STRINGS "cmake/externals.cmake" externals_cmake NEWLINE_CONSUME)
string(REPLACE "graph " "" externals_cmake "${externals_cmake}")
string(REPLACE "set(GLUT_LIBRARIES \"freeglut\")" "find_package(GLUT REQUIRED)" externals_cmake "${externals_cmake}")
string(REPLACE "set(GLEW_LIBRARIES \"glew32\")" "find_package(GLEW REQUIRED)" externals_cmake "${externals_cmake}")
string(REPLACE "else()\n\tset(O" "\tlist(APPEND GLOBAL_INCLUDE_DIRECTORIES \"\${GLUT_INCLUDE_DIRS}\" \"\${GLEW_INCLUDE_DIRS}\")\nelse()\n\tset(O" externals_cmake "${externals_cmake}")
string(REPLACE "GLUT REQUIRED" "GLUT REQUIRED CONFIG" externals_cmake "${externals_cmake}")
string(REPLACE "GLEW REQUIRED" "GLEW REQUIRED CONFIG" externals_cmake "${externals_cmake}")
file(WRITE "cmake/externals.cmake" "${externals_cmake}")

file(STRINGS "cmake/preBuildConfig.cmake" preBuildConfig_cmake NEWLINE_CONSUME)
string(REPLACE "DEBUG \"\${SOFA_BIN_DIR}" "DEBUG \"\${SOFA_BIN_DIR}/Debug" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "RELEASE \"\${SOFA_BIN_DIR}" "RELEASE \"\${SOFA_BIN_DIR}/Release" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "RELWITHDEBINFO \"\${SOFA_BIN_DIR}" "RELWITHDEBINFO \"\${SOFA_BIN_DIR}/RelWithDebInfo" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "MINSIZEREL \"\${SOFA_BIN_DIR}" "MINSIZEREL \"\${SOFA_BIN_DIR}/MinSizeRel" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "DEBUG \"\${SOFA_LIB_DIR}" "DEBUG \"\${SOFA_LIB_DIR}/Debug" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "RELEASE \"\${SOFA_LIB_DIR}" "RELEASE \"\${SOFA_LIB_DIR}/Release" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "RELWITHDEBINFO \"\${SOFA_LIB_DIR}" "RELWITHDEBINFO \"\${SOFA_LIB_DIR}/RelWithDebInfo" preBuildConfig_cmake "${preBuildConfig_cmake}")
string(REPLACE "MINSIZEREL \"\${SOFA_LIB_DIR}" "MINSIZEREL \"\${SOFA_LIB_DIR}/MinSizeRel" preBuildConfig_cmake "${preBuildConfig_cmake}")
file(WRITE "cmake/preBuildConfig.cmake" "${preBuildConfig_cmake}")

file(APPEND "CMakeLists.txt" "\n\nconfigure_file(SOFAConfig.cmake.in SOFAConfig.cmake @ONLY)")

file(WRITE "SOFAConfig.cmake.in"
"add_definitions(-DSOFA_XML_PARSER_TINYXML;-DTIXML_USE_STL;-DMINI_FLOWVR;-DSOFA_HAVE_BOOST;-DSOFA_HAVE_FREEGLUT;-DSOFA_HAVE_GLEW)

set(SOFA_INCLUDE_DIRS \"@SOFA_EXTLIBS_DIR@/miniFlowVR/include;@SOFA_EXTLIBS_DIR@/newmat;@SOFA_EXTLIBS_DIR@/tinyxml;@SOFA_SRC_DIR@/framework;@SOFA_SRC_DIR@/modules\")

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
  debug SofaSimulationTree\${version}d optimized SofaSimulationTree\${version}
  debug SofaSphFluid\${version}d optimized SofaSphFluid\${version}
  debug SofaTopologyMapping\${version}d optimized SofaTopologyMapping\${version}
  debug SofaUserInteraction\${version}d optimized SofaUserInteraction\${version}
  debug SofaValidation\${version}d optimized SofaValidation\${version}
  debug SofaVolumetricData\${version}d optimized SofaVolumetricData\${version}
  debug SofaTinyXml\${version}d optimized SofaTinyXml\${version}
)")
