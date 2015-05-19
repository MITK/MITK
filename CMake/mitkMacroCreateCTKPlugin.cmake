macro(MACRO_CREATE_MITK_CTK_PLUGIN)

  message(WARNING "The MACRO_CREATE_MITK_CTK_PLUGIN macro is deprecated since 2015.03. Use mitk_create_plugin instead.")

  mitk_create_plugin(${ARGN})

endmacro()
