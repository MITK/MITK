set( VPX_LIB "" CACHE FILEPATH "Search for the vpx library file" )
set( VPX_HEADER_DIR "" CACHE PATH "The path containing vpy headers" )
set( VPX_FOUND FALSE )

if(VPX_LIB AND VPX_HEADER_DIR)
  list(APPEND ALL_LIBRARIES ${VPX_LIB})
  list(APPEND ALL_INCLUDE_DIRECTORIES ${VPX_HEADER_DIR})
  set( VPX_FOUND TRUE )
endif()
