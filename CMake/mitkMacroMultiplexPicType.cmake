#
# MITK_MULTIPLEX_PICTYPE: generate separated source files for different
# data types to reduce memory consumption of compiler during template
# instantiation
#
# Param "file" should be named like mitkMyAlgo-TYPE.cpp
# in the file, every occurence of @TYPE@ is replaced by the
# datatype. For each datatype, a new file mitkMyAlgo-datatype.cpp
# is generated and added to CPP_FILES_GENERATED.
#
macro(MITK_MULTIPLEX_PICTYPE file)
  string(REPLACE "," ";" TYPES "${MITK_ACCESSBYITK_PIXEL_TYPES}")
  foreach(TYPE ${TYPES})
    # create filename for destination
    string(REPLACE " " "_" quoted_type "${TYPE}")
    string(REPLACE TYPE ${quoted_type} quoted_file ${file})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${quoted_file} @ONLY)
    set(CPP_FILES_GENERATED ${CPP_FILES_GENERATED} ${CMAKE_CURRENT_BINARY_DIR}/${quoted_file})
  endforeach(TYPE)
endmacro(MITK_MULTIPLEX_PICTYPE)
