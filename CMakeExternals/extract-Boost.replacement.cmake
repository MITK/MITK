get_filename_component(sample Boost/boost ABSOLUTE)
if(NOT EXISTS "${sample}")
  include(Boost-stamp/extract-Boost.original.cmake)
endif()
