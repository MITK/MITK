
function(mitkFunctionCheckCxx0xUnorderedSet var_has_feature)

  CHECK_CXX_SOURCE_COMPILES(
    "
    #include <unordered_set>
    int main() { std::unordered_set<int> us; return 0; }
    "
    ${var_has_feature}
  )

  set(${var_has_feature} "${${var_has_feature}}" PARENT_SCOPE)
endfunction()
