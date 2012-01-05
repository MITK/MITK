
# suppress some warnings in VC8 about using unsafe/deprecated c functions
function(SUPPRESS_VC_DEPRECATED_WARNINGS)
  if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
  endif()
endfunction()

function(SUPPRESS_ALL_WARNINGS)
  if(MSVC)
    string(REGEX REPLACE "/W[0-9]" "" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})   
    string(REGEX REPLACE "/W[0-9]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})   
    add_definitions(/w)
    # suppress also poco linker warnings
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4217")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /ignore:4217")
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-w)
  endif()
endfunction(SUPPRESS_ALL_WARNINGS)

