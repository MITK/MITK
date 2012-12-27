# - Find Eclipse
# The following variables are set if Eclipse is found.
#
#  Eclipse_DIR      - the path to the Eclipse base directory.

if(WIN32)
  set(_eclipse_dir_search
    "C:/Eclipse"
    "D:/Eclipse"
    "C:/Program Files/Eclipse"
    "C:/Programme/Eclipse"
    "D:/Program Files/Eclipse"
    "D:/Programme/Eclipse"
  )
endif(WIN32)

#
# Look for an installation.
#
find_path(Eclipse_DIR eclipse PATHS

  # Look in other places.
  ${_eclipse_dir_search}

  # Help the user find it if we cannot.
  DOC "The base directory of your Eclipse installation"
)

