
function(mitkFunctionCheckCxx0xHashString var_has_feature)

  CHECK_CXX_SOURCE_COMPILES(
    "
    #include <string>
    #include <functional>
    int main() { std::hash<std::string> h; return 0; }
    "
    ${var_has_feature}
  )

  set(${var_has_feature} "${${var_has_feature}}" PARENT_SCOPE)
endfunction()
