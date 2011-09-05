if(MSVC10)
  set(MITK_USE_C++0x 1)
else()
  option(MITK_USE_C++0x "Enable C++0x features in MITK. This is experimental." OFF)
  mark_as_advanced(MITK_USE_C++0x)
endif()

# Begin checking fo C++0x features
if(MITK_USE_C++0x)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_REQUIRED_FLAGS "-std=c++0x")
  endif()

  include(mitkFunctionCheckC++0xHashString)
  mitkFunctionCheckCxx0xHashString(MITK_HAS_HASH_STRING_0x)

  include(mitkFunctionCheckC++0xHashSizeT)
  mitkFunctionCheckCxx0xHashSizeT(MITK_HAS_HASH_SIZE_T_0x)

  include(mitkFunctionCheckC++0xUnorderedMap)
  mitkFunctionCheckCxx0xUnorderedMap(MITK_HAS_UNORDERED_MAP_H_0x)

  include(mitkFunctionCheckC++0xUnorderedSet)
  mitkFunctionCheckCxx0xUnorderedMap(MITK_HAS_UNORDERED_SET_H_0x)

  set(MITK_HAS_VAR_SUFFIX "_0x")
  set(CMAKE_REQUIRED_FLAGS "")

else()

  set(CMAKE_REQUIRED_FLAGS "")

  include(mitkFunctionCheckItkHashString)
  mitkFunctionCheckItkHashString(MITK_HAS_HASH_STRING_def)

  include(mitkFunctionCheckItkHashSizeT)
  mitkFunctionCheckItkHashSizeT(MITK_HAS_HASH_SIZE_T_def)

  set(MITK_HAS_UNORDERED_MAP_H_def )
  set(MITK_HAS_UNORDERED_SET_H_def )

  set(MITK_HAS_VAR_SUFFIX "_def")

endif()

foreach(var
        MITK_HAS_HASH_STRING
        MITK_HAS_HASH_SIZE_T
        MITK_HAS_UNORDERED_MAP_H
        MITK_HAS_UNORDERED_SET_H
       )
  set(${var} ${${var}${MITK_HAS_VAR_SUFFIX}})
endforeach()

# End checking C++0x features
