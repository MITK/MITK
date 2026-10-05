# Applied as the PATCH_COMMAND of the sentry external project.
#
# On Linux, crashpad's util CMakeLists hard-requires libcurl for its HTTP
# upload transport (find_package(CURL REQUIRED)); on Windows it uses WinHTTP
# and needs nothing. MITK starts the crash handler with an empty upload URL
# (local-only dumps: SENTRY_TRANSPORT=none, no DSN), so the transport is
# compiled but never invoked at runtime. This rewrite selects crashpad's
# curl-free socket transport and compiles TLS out, removing the libcurl
# (and the socket path's optional OpenSSL) build dependency entirely.
#
# Editing the file on Windows is harmless: the affected lines live inside an
# if(LINUX) block that CMake never enters there, so the source tree stays
# identical across platforms.
#
# CRASHPAD_UTIL_CMAKE must be the absolute path to
# <sentry-source>/external/crashpad/util/CMakeLists.txt.

if(NOT DEFINED CRASHPAD_UTIL_CMAKE)
  message(FATAL_ERROR "[MITK] CRASHPAD_UTIL_CMAKE not set")
endif()

if(NOT EXISTS "${CRASHPAD_UTIL_CMAKE}")
  message(FATAL_ERROR "[MITK] crashpad util CMakeLists not found: ${CRASHPAD_UTIL_CMAKE}")
endif()

file(READ "${CRASHPAD_UTIL_CMAKE}" contents)

# Our own marker doubles as the idempotency signal: re-running the patch step
# (clean rebuilds re-run it) must be a no-op rather than a second rewrite.
if(contents MATCHES "\\[MITK\\] curl-free crash transport")
  message(STATUS "[MITK] crashpad curl transport already patched out")
  return()
endif()

set(orig "${contents}")

# Drop the hard curl requirement and the curl link; the socket transport
# needs neither. TLS stays off (CRASHPAD_USE_BORINGSSL undefined), which is
# the whole point - a local-only handler never opens a socket.
string(REPLACE
  "find_package(CURL REQUIRED)"
  "# [MITK] curl-free crash transport: libcurl requirement removed"
  contents "${contents}")

string(REPLACE
  "target_link_libraries(crashpad_util PRIVATE CURL::libcurl)"
  "# [MITK] curl-free crash transport: no libcurl link"
  contents "${contents}")

string(REPLACE
  "SET(HTTP_TRANSPORT_IMPL net/http_transport_libcurl.cc)"
  "SET(HTTP_TRANSPORT_IMPL net/http_transport_socket.cc)"
  contents "${contents}")

# A vendored-crashpad bump can move these anchors. Fail loudly at configure
# rather than silently leaving the curl dependency in place (which would only
# resurface as the original CI error, minus any hint of why).
if(contents STREQUAL orig)
  message(FATAL_ERROR
    "[MITK] crashpad curl-transport patch matched nothing. The vendored "
    "crashpad likely changed - update CMakeExternals/PatchSentryCrashpad.cmake "
    "to match util/CMakeLists.txt.")
endif()

file(WRITE "${CRASHPAD_UTIL_CMAKE}" "${contents}")
message(STATUS "[MITK] crashpad set to curl-free socket transport (TLS off) on Linux")
