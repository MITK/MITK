# CMakeLists.txt for Breakpad
# - supports builds on Windows and Linux only
# - needs to keep updated when updating the SVN revision in Breakpad.cmake
# - imitates the library structure of google's build system
# - does not build libraries which are not required by the MITK breakpad module

cmake_minimum_required(VERSION 2.8)

project (BreakpadForMITK CXX)

if (WIN32)

  include_directories(src)

 # Windows...
 set(exceptionhandler_CPPs
   src/client/windows/handler/exception_handler.cc
   )

 set(server_CPPs
   src/client/windows/crash_generation/client_info.cc
   src/client/windows/crash_generation/crash_generation_server.cc
   src/client/windows/crash_generation/minidump_generator.cc
   )

 set(client_CPPs
   src/client/windows/crash_generation/crash_generation_client.cc
   )

 set(common_CPPs
   src/common/convert_UTF.c
   src/common/language.cc
   src/common/string_conversion.cc
   src/common/module.cc
   src/common/windows/http_upload.cc
   src/common/windows/string_utils.cc
   src/common/windows/guid_string.cc
   )

  # this was copied from http://svn.code.sf.net/p/safir/svn/safir_sdk_core/trunk/src/lluf/lluf_crash_reporter.ss/src/google-breakpad/build-breakpad-windows.cmake
  # check if this is actually required or not
  add_definitions(-DBREAKPAD_NO_TERMINATE_THREAD)
  add_definitions(-DUNICODE -D_UNICODE)
  add_definitions(/wd4127 /wd4245)

  set_source_files_properties(src/common/convert_UTF.c PROPERTIES LANGUAGE CXX) # seems more simple than tell CMake about two compilers
  # following line is inspired by breakpad issue 465 where the following definition was suggested as a patch for MSVC 2005/8
  # TODO: do we need a if MSVC2005/8? (check MITK requirements)
  set_source_files_properties(src/client/windows/handler/exception_handler.cc PROPERTIES COMPILE_DEFINITIONS "STATUS_INVALID_PARAMETER=((DWORD)0xC000000DL)")

  include_directories(src)

  add_library(common STATIC ${common_CPPs})
  add_library(crash_generation_client STATIC ${client_CPPs})
  add_library(crash_generation_server STATIC ${server_CPPs})
  add_library(exception_handler STATIC ${exceptionhandler_CPPs})

else()

 # Linux
 # commented code is not required right now and depends on other libraries (e.g. glog)
 set(client_CPPs
   src/client/minidump_file_writer.cc
   src/client/linux/handler/minidump_descriptor.cc
   src/client/linux/handler/exception_handler.cc
   src/client/linux/log/log.cc
   src/client/linux/minidump_writer/linux_core_dumper.cc
   src/client/linux/minidump_writer/linux_ptrace_dumper.cc
   src/client/linux/minidump_writer/linux_dumper.cc
   src/client/linux/minidump_writer/minidump_writer.cc
   src/client/linux/crash_generation/crash_generation_server.cc
   src/client/linux/crash_generation/crash_generation_client.cc
   )

 set(common_CPPs
   src/common/convert_UTF.c
   src/common/md5.cc
   src/common/language.cc
   src/common/dwarf_cfi_to_module.cc
   src/common/dwarf_line_to_module.cc
   src/common/string_conversion.cc
   src/common/module.cc
   src/common/dwarf_cu_to_module.cc
   src/common/stabs_to_module.cc
   src/common/linux/libcurl_wrapper.cc
   src/common/linux/safe_readlink.cc
   src/common/linux/http_upload.cc
   src/common/linux/google_crashdump_uploader.cc
   src/common/linux/elf_symbols_to_module.cc
   src/common/linux/guid_creator.cc
   src/common/linux/memory_mapped_file.cc
   src/common/linux/linux_libc_support.cc
   src/common/linux/dump_symbols.cc
   src/common/linux/file_id.cc
   src/common/linux/synth_elf.cc
   src/common/linux/elfutils.cc
   src/common/linux/elf_core_dump.cc
   )

 # commented code is not required right now and depends on other libraries (e.g. glog)
 set_source_files_properties(src/common/convert_UTF.c PROPERTIES LANGUAGE CXX) # seems more simple than tell CMake about two compilers

 include_directories(src)
 add_definitions(-fPIC)

 add_library(breakpad_client STATIC
   ${client_CPPs}
   ${common_CPPs}
   )

endif()
