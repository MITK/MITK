MACRO(MITK_WRAP_OBJECT HEADERFILES CLASSNAME WRAPPERNAME DIRECTORY)
FILE(WRITE ${DIRECTORY}/wrap_${CLASSNAME}.cxx "#include "mitkCSwigMacros.h"\n")
FOREACH(f ${HEADERFILES})
	FILE(APPEND ${DIRECTORY}/wrap_${CLASSNAME}.cxx "#include "${f}"\n")
ENDFOREACH()
FILE(APPEND ${DIRECTORY}/wrap_${CLASSNAME}.cxx "#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="${CLASSNAME}";
     namespace wrappers
     {
	MITK_WRAP_OBJECT(${CLASSNAME})
     }
}

#endif")
ENDMACRO()
