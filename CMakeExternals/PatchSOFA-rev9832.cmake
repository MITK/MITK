file(STRINGS "framework/sofa/helper/vector_device.h" vector_device_h NEWLINE_CONSUME)
string(REPLACE "#ifndef SOFA_NO_OPENGL\n    GLu" "    unsigned " vector_device_h ${vector_device_h})
string(REPLACE "#endif\n    mutable i" "    mutable i" vector_device_h ${vector_device_h})
string(REPLACE "        if (MemoryManager::SUPPORT_GL_BUFFER && bufferObject)\n" "#ifndef SOFA_NO_OPENGL\n        if (MemoryManager::SUPPORT_GL_BUFFER && bufferObject)\n" vector_device_h ${vector_device_h})
string(REPLACE "//         else {" "#endif\n//         else {" vector_device_h ${vector_device_h})
string(REPLACE "P ( GLu" "P ( unsigned " vector_device_h ${vector_device_h})
set(CONTENTS ${vector_device_h})
configure_file(${TEMPLATE_FILE} "framework/sofa/helper/vector_device.h" @ONLY)

file(STRINGS "sofa-dependencies.cmake" sofaDependencies_cmake NEWLINE_CONSUME)
string(REPLACE "add_subdirectory(\"\${SOFA_A" "#add_subdirectory(\"\${SOFA_A" sofaDependencies_cmake "${sofaDependencies_cmake}")
file(WRITE "sofa-dependencies.cmake" "${sofaDependencies_cmake}")

file(STRINGS "modules/sofa/component/SofaUserInteraction/CMakeLists.txt" CMakeLists_txt NEWLINE_CONSUME)
string(REPLACE "../collision/AddFramePerformer." "#../collision/AddFramePerformer." CMakeLists_txt "${CMakeLists_txt}")
file(WRITE "modules/sofa/component/SofaUserInteraction/CMakeLists.txt" "${CMakeLists_txt}")

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
"add_definitions(-DSOFA_NO_OPENGL;-DSOFA_XML_PARSER_TINYXML;-DTIXML_USE_STL;-DMINI_FLOWVR)

set(SOFA_INCLUDE_DIRS \"@SOFA_EXTLIBS_DIR@/miniBoost;@SOFA_EXTLIBS_DIR@/miniFlowVR/include;@SOFA_EXTLIBS_DIR@/newmat;@SOFA_EXTLIBS_DIR@/tinyxml;@SOFA_SRC_DIR@/framework;@SOFA_SRC_DIR@/modules\")

set(SOFA_LIBRARY_DIRS \"@SOFA_LIB_DIR@\")

set(SOFA_LIBRARIES
  debug SofaBaseAnimationLoop_1_0d optimized SofaBaseAnimationLoop_1_0
  debug SofaBaseCollision_1_0d optimized SofaBaseCollision_1_0
  debug SofaBaseLinearSolver_1_0d optimized SofaBaseLinearSolver_1_0
  debug SofaBaseMechanics_1_0d optimized SofaBaseMechanics_1_0
  debug SofaBaseTopology_1_0d optimized SofaBaseTopology_1_0
  debug SofaBaseVisual_1_0d optimized SofaBaseVisual_1_0
  debug SofaBoundaryCondition_1_0d optimized SofaBoundaryCondition_1_0
  debug SofaComponentAdvanced_1_0d optimized SofaComponentAdvanced_1_0
  debug SofaComponentBase_1_0d optimized SofaComponentBase_1_0
  debug SofaComponentCommon_1_0d optimized SofaComponentCommon_1_0
  debug SofaComponentGeneral_1_0d optimized SofaComponentGeneral_1_0
  debug SofaComponentMain_1_0d optimized SofaComponentMain_1_0
  debug SofaComponentMisc_1_0d optimized SofaComponentMisc_1_0
  debug SofaConstraint_1_0d optimized SofaConstraint_1_0
  debug SofaCore_1_0d optimized SofaCore_1_0
  debug SofaDefaultType_1_0d optimized SofaDefaultType_1_0
  debug SofaDeformable_1_0d optimized SofaDeformable_1_0
  debug SofaDenseSolver_1_0d optimized SofaDenseSolver_1_0
  debug SofaEngine_1_0d optimized SofaEngine_1_0
  debug SofaEulerianFluid_1_0d optimized SofaEulerianFluid_1_0
  debug SofaExplicitOdeSolver_1_0d optimized SofaExplicitOdeSolver_1_0
  debug SofaExporter_1_0d optimized SofaExporter_1_0
  debug SofaGraphComponent_1_0d optimized SofaGraphComponent_1_0
  debug SofaHaptics_1_0d optimized SofaHaptics_1_0
  debug SofaHelper_1_0d optimized SofaHelper_1_0
  debug SofaImplicitOdeSolver_1_0d optimized SofaImplicitOdeSolver_1_0
  debug SofaLoader_1_0d optimized SofaLoader_1_0
  debug SofaMeshCollision_1_0d optimized SofaMeshCollision_1_0
  debug SofaMisc_1_0d optimized SofaMisc_1_0
  debug SofaMiscCollision_1_0d optimized SofaMiscCollision_1_0
  debug SofaMiscEngine_1_0d optimized SofaMiscEngine_1_0
  debug SofaMiscFem_1_0d optimized SofaMiscFem_1_0
  debug SofaMiscForceField_1_0d optimized SofaMiscForceField_1_0
  debug SofaMiscMapping_1_0d optimized SofaMiscMapping_1_0
  debug SofaMiscSolver_1_0d optimized SofaMiscSolver_1_0
  debug SofaMiscTopology_1_0d optimized SofaMiscTopology_1_0
  debug SofaNonUniformFem_1_0d optimized SofaNonUniformFem_1_0
  debug SofaObjectInteraction_1_0d optimized SofaObjectInteraction_1_0
  debug SofaPreconditioner_1_0d optimized SofaPreconditioner_1_0
  debug SofaRigid_1_0d optimized SofaRigid_1_0
  debug SofaSimpleFem_1_0d optimized SofaSimpleFem_1_0
  debug SofaSimulationCommon_1_0d optimized SofaSimulationCommon_1_0
  debug SofaSimulationTree_1_0d optimized SofaSimulationTree_1_0
  debug SofaSphFluid_1_0d optimized SofaSphFluid_1_0
  debug SofaTopologyMapping_1_0d optimized SofaTopologyMapping_1_0
  debug SofaUserInteraction_1_0d optimized SofaUserInteraction_1_0
  debug SofaValidation_1_0d optimized SofaValidation_1_0
  debug SofaVolumetricData_1_0d optimized SofaVolumetricData_1_0
  debug tinyxml_1_0d optimized tinyxml_1_0
)")
