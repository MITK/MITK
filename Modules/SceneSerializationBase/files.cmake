# Do not glob files in the Testing directory
file(GLOB SRCS
  *.cpp
  BasePropertySerializer/*.cpp
  )

set(CPP_FILES ${SRCS})

