
function(mitkFunctionCheckItkHashString var_has_feature)

  CHECK_CXX_SOURCE_COMPILES(
    "
    #include <itk_hash_map.h>
    #include <string>
    int main() { itk::hash<std::string> h; return 0; }
    "
    ${var_has_feature}
  )

  set(${var_has_feature} "${${var_has_feature}}" PARENT_SCOPE)
endfunction()
