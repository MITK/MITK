
function(mitkFunctionCheckCxx0xHashSizeT var_has_feature)

  CHECK_CXX_SOURCE_COMPILES(
    "
    #include <unordered_map>
    int main() { std::hash<std::size_t> h; return 0; }
    "
    ${var_has_feature}
  )

  set(${var_has_feature} "${${var_has_feature}}" PARENT_SCOPE)
endfunction()
