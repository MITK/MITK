set(BetData_DIR ${MITK_SUPERBUILD_BINARY_DIR}/ep/src/BetData)

if(EXISTS ${BetData_DIR})
  set(BetData_FOUND TRUE)
else()
  set(BetData_FOUND FALSE)
endif()
