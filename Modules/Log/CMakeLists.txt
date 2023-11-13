option(MITK_ENABLE_DEBUG_MESSAGES "Enable extra debug log output" OFF)
mark_as_advanced(MITK_ENABLE_DEBUG_MESSAGES)

configure_file(
  mitkLogConfig.h.in
  mitkLogConfig.h
)

mitk_create_module(
  NO_INIT
)
