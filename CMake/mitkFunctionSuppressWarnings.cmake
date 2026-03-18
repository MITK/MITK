function(SUPPRESS_VC_DEPRECATED_WARNINGS)
  message(DEPRECATION "SUPPRESS_VC_DEPRECATED_WARNINGS() is deprecated. "
    "CRT deprecation warnings are suppressed via the MitkCompilerFlags target.")
endfunction()

function(SUPPRESS_ALL_WARNINGS)
  message(DEPRECATION "SUPPRESS_ALL_WARNINGS() is deprecated. "
    "Use add_compile_options(-w) or add_compile_options(/W0) at directory scope instead.")
endfunction()
