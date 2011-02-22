#
# Extracts the current git revision hash and branch/tag name
# and provides these in
# MITK_WC_REVISION_HASH
# MITK_WC_REVISION_NAME
#
function(mitkFunctionGetVersion source_dir prefix)
  find_package(Git)
  if(GIT_FOUND)
    GIT_WC_INFO(${source_dir} ${prefix})

    if(NOT ${prefix}_WC_REVISION_HASH)
      set(${prefix}_WC_REVISION_HASH "n/a")
    endif()
    
    string(REPLACE " " ";" hash_name ${${prefix}_WC_REVISION_NAME})
    list(GET hash_name 1 name)
    if (NOT name)
      set(name "n/a")
    endif()
    
    set(${prefix}_WC_REVISION_HASH ${${prefix}_WC_REVISION_HASH} PARENT_SCOPE)
    set(${prefix}_WC_REVISION_NAME ${name} PARENT_SCOPE)
  endif()
endfunction()
