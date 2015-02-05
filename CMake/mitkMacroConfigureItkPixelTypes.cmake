#-----------------------------------
# Configuration of the buid-in MITK pixel types
#-----------------------------------

# create a list of types for template instantiations of itk image access functions
function(_create_type_seq TYPES seq_var seqdim_var)
  set(_seq )
  set(_seq_dim )
  string(REPLACE "," ";" _pixeltypes "${TYPES}")
  foreach(_pixeltype ${_pixeltypes})
    set(_seq "${_seq}(${_pixeltype})")
    set(_seq_dim "${_seq_dim}((${_pixeltype},dim))")
  endforeach()
  set(${seq_var} "${_seq}" PARENT_SCOPE)
  set(${seqdim_var} "${_seq_dim}" PARENT_SCOPE)
endfunction()

macro(mitkMacroConfigureItkPixelTypes)

  set(MITK_ACCESSBYITK_PIXEL_TYPES )
  set(MITK_ACCESSBYITK_PIXEL_TYPES_SEQ )
  set(MITK_ACCESSBYITK_TYPES_DIMN_SEQ )

  # concatenate only the simple pixel types to the MITK_ACCESSBYITK_PIXEL_TYPE_SEQ list
  # see Bug 12682 for detailed information
  foreach(_type INTEGRAL FLOATING)
    set(_typelist "${MITK_ACCESSBYITK_${_type}_PIXEL_TYPES}")
    if(_typelist)
      if(MITK_ACCESSBYITK_PIXEL_TYPES)
        set(MITK_ACCESSBYITK_PIXEL_TYPES "${MITK_ACCESSBYITK_PIXEL_TYPES},${_typelist}")
      else()
        set(MITK_ACCESSBYITK_PIXEL_TYPES "${_typelist}")
      endif()
    endif()

    _create_type_seq("${_typelist}"
                     MITK_ACCESSBYITK_${_type}_PIXEL_TYPES_SEQ
                     MITK_ACCESSBYITK_${_type}_TYPES_DIMN_SEQ)
    set(MITK_ACCESSBYITK_PIXEL_TYPES_SEQ "${MITK_ACCESSBYITK_PIXEL_TYPES_SEQ}${MITK_ACCESSBYITK_${_type}_PIXEL_TYPES_SEQ}")
    set(MITK_ACCESSBYITK_TYPES_DIMN_SEQ "${MITK_ACCESSBYITK_TYPES_DIMN_SEQ}${MITK_ACCESSBYITK_${_type}_TYPES_DIMN_SEQ}")
  endforeach()

  # separate processing of the COMPOSITE list to avoid its concatenation to the global list
  _create_type_seq(${MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES}
                   MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ
                   MITK_ACCESSBYITK_COMPOSITE_TYPES_DIMN_SEQ)

  # separate processing of the VECTOR list to avoid its concatenation to the global list
  _create_type_seq(${MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES}
                   MITK_ACCESSBYITK_VECTOR_PIXEL_TYPES_SEQ
                   MITK_ACCESSBYITK_VECTOR_TYPES_DIMN_SEQ)

  set(MITK_ACCESSBYITK_DIMENSIONS_SEQ )
  string(REPLACE "," ";" _dimensions "${MITK_ACCESSBYITK_DIMENSIONS}")
  foreach(_dimension ${_dimensions})
    set(MITK_ACCESSBYITK_DIMENSIONS_SEQ "${MITK_ACCESSBYITK_DIMENSIONS_SEQ}(${_dimension})")
  endforeach()

endmacro()
