# this macro gets a filelist as input and looks
# for corresponding h-files to add them to the project.
# 
# additionally files are grouped in source-groups.
#  
# No parameters explicitly declared here, because
# we want to allow for variable argument lists, which
# are later access by the keyword FOREACH(_file ${ARGV})
#  
# output: after calling the macro, files that were found
# correspondigly to the given files are stored in the
# variable:
# ${CORRESPONDING__H_FILES}
# ${CORRESPONDING__TXX_FILES}
  
# Globbed can be found in the variables
# ${GLOBBED__txx_fileS} (CURRENTLY COMMENTED OUT)
# ${GLOBBED_DOX_FILES}

MACRO(MACRO_ORGANIZE_SOURCES)

  MACRO_PARSE_ARGUMENTS(_ORG "INPUT;GENERATED" "" ${ARGN})

  # glob all txx files and add to subgroup
  #FILE(GLOB_RECURSE GLOBBED__TXX_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.txx)
  
  # include documentation
  FILE(GLOB_RECURSE GLOBBED_DOX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.dox)
  SOURCE_GROUP("Doxygen Files" FILES ${GLOBBED_DOX_FILES})
  
  # include documentation
  FILE(GLOB_RECURSE GLOBBED_UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)
  SOURCE_GROUP("QT UI Files" FILES ${GLOBBED_UI_FILES})
  
  # initialise storage for header-files
  SET(CORRESPONDING__H_FILES "" )
  SET(CORRESPONDING__TXX_FILES "" )
  
  _MACRO_ITERATE_OVER_SOURCES("${CMAKE_CURRENT_SOURCE_DIR}/" ${_ORG_INPUT})
  _MACRO_ITERATE_OVER_SOURCES("" ${_ORG_GENERATED})
  
ENDMACRO(MACRO_ORGANIZE_SOURCES)

MACRO(_MACRO_ITERATE_OVER_SOURCES _path_prefix_)

SET(_path_prefix "${_path_prefix_}")

# iterate through sources
  FOREACH(_file ${ARGN} )#${GLOBBED__TXX_FILES})
    #IF(EXISTS ${_path_prefix}${_file})
      STRING(REGEX MATCH "cxx$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("Generated Source Files" FILES ${_path_prefix}${_file})
      ENDIF(_result)
      
      STRING(REGEX MATCH "cpp$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("== Source Files ==" FILES ${_path_prefix}${_file})
      ENDIF(_result)
      
      STRING(REGEX MATCH "txx$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("== Template Files ==" FILES ${_path_prefix}/${_file})
      ENDIF(_result)
      
      STRING(REGEX MATCH "c$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("== Source Files ==" FILES ${_path_prefix}${_file})
      ENDIF(_result)
      
      STRING(REGEX MATCH "h$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("== Header Files ==" FILES ${_path_prefix}${_file})
      ENDIF(_result)
      
      STRING(REGEX MATCH "ui$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("QT UI Files" FILES ${_path_prefix}${_file})
      ENDIF(_result)

      STRING(REGEX MATCH ".*moc_.*cxx$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("Generated MOC Source Files" FILES ${_path_prefix}${_file})
      ENDIF(_result)

      STRING(REGEX MATCH ".*ui_.*h$" _result ${_file})
      IF(_result)
        SOURCE_GROUP("Generated UI Header Files" FILES ${_path_prefix}${_file})
      ENDIF(_result)
      
      # include header files in visual studio solution
      STRING(REGEX MATCH "(txx|cpp|c|cxx)$" _result ${_file})
      IF(_result)
        STRING(REGEX REPLACE "(txx|cpp|c|cxx)$" "h" _h_file ${_file})
        IF(EXISTS ${_path_prefix}${_h_file})
          SET(CORRESPONDING__H_FILES
            ${CORRESPONDING__H_FILES}
            ${_path_prefix}${_h_file}
          )
          SOURCE_GROUP("== Header Files ==" FILES ${_path_prefix}${_h_file} )
        ENDIF(EXISTS ${_path_prefix}${_h_file})
      ENDIF(_result)
      
      # include txx files in visual studio solution
      STRING(REGEX MATCH "(h|cpp|c|cxx)$" _result ${_file})
      IF(_result)
        STRING(REGEX REPLACE "(h|cpp|c|cxx)$" "txx" _txx_file ${_file})
        IF(EXISTS ${_path_prefix}${_txx_file})
          SET(CORRESPONDING__TXX_FILES
            ${CORRESPONDING__TXX_FILES}
            ${_path_prefix}${_txx_file}
          )
          SOURCE_GROUP("== Template Files ==" FILES ${_path_prefix}${_txx_file} )
        ENDIF(EXISTS ${_path_prefix}${_txx_file})
      ENDIF(_result)
      
    #ENDIF(EXISTS ${_path_prefix}${_file})
  ENDFOREACH(_file)

ENDMACRO(_MACRO_ITERATE_OVER_SOURCES _path_prefix)
