macro(MITK_WRAP_OBJECT HEADERFILES CLASSNAME WRAPPERNAME DIRECTORY)
file(WRITE ${DIRECTORY}/wrap_${CLASSNAME}.cxx "#include "mitkCSwigMacros.h"\n")
foreach(f ${HEADERFILES})
  file(APPEND ${DIRECTORY}/wrap_${CLASSNAME}.cxx "#include "${f}"\n")
endforeach()
file(APPEND ${DIRECTORY}/wrap_${CLASSNAME}.cxx "#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="${CLASSNAME}";
     namespace wrappers
     {
  MITK_WRAP_OBJECT(${CLASSNAME})
     }
}

#endif")
endmacro()
