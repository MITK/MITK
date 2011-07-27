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
MACRO(MITK_MULTIPLEX_PICTYPE file)
  STRING(REPLACE "," ";" TYPES "${MITK_ACCESSBYITK_PIXEL_TYPES}")
  FOREACH(TYPE ${TYPES})
    # create filename for destination
    STRING(REPLACE " " "_" quoted_type "${TYPE}")
    STRING(REPLACE TYPE ${quoted_type} quoted_file ${file})
    CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}/${quoted_file} @ONLY)
    SET(CPP_FILES_GENERATED ${CPP_FILES_GENERATED} ${CMAKE_CURRENT_BINARY_DIR}/${quoted_file})
  ENDFOREACH(TYPE)
ENDMACRO(MITK_MULTIPLEX_PICTYPE)
