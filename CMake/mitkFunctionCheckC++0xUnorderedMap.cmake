
function(mitkFunctionCheckCxx0xUnorderedMap var_has_feature)

  CHECK_CXX_SOURCE_COMPILES(
    "
    #include <unordered_map>
    int main() { std::unordered_map<int,int> um; return 0; }
    "
    ${var_has_feature}
  )

  set(${var_has_feature} "${${var_has_feature}}" PARENT_SCOPE)
endfunction()
