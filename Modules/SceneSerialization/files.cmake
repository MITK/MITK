# Do not glob files in the Testing directory
file(GLOB SRCS
  *.cpp
  BaseDataSerializer/*.cpp
  )

set(CPP_FILES ${SRCS})

