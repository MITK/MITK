#[[ Given the path to a file (typically a .lib), this function searches
    upward in the directory hierarchy to locate the nearest 'lib' directory,
    then checks for a corresponding 'bin' directory.

    If a 'bin' directory is found, its path is returned in out_bin_dir.
    If not found, out_bin_dir is set to an empty string.

    The following example will set ssl_bin to "C:/Program Files/OpenSSL-Win64/bin":
      mitkFindClosestBinDirectory(
        "C:/Program Files/OpenSSL-Win64/lib/VC/x64/MD/libssl.lib"
        ssl_bin
      )
 ]]
function(mitkFindClosestBinDirectory input_path out_bin_dir)
  # Start from the directory containing the given input file
  get_filename_component(input_dir "${input_path}" DIRECTORY)
  file(REAL_PATH "${input_dir}" input_dir)

  # Walk up directories until we find a 'lib' directory
  set(search_dir "${input_dir}")
  while(NOT "${search_dir}" STREQUAL "" AND EXISTS "${search_dir}")
    get_filename_component(dir_name "${search_dir}" NAME)
    if(dir_name STREQUAL "lib")
      # One level up from 'lib' folder
      get_filename_component(parent_dir "${search_dir}" DIRECTORY)
      set(candidate_bin "${parent_dir}/bin")
      if(EXISTS "${candidate_bin}")
        set(${out_bin_dir} "${candidate_bin}" PARENT_SCOPE)
        return()
      endif()
    endif()
    # Move one level up
    get_filename_component(search_dir "${search_dir}" DIRECTORY)
  endwhile()

  # Not found
  set(${out_bin_dir} "" PARENT_SCOPE)
endfunction()
