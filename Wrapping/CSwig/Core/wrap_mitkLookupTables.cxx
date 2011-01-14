#include "mitkLookupTables.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="LookupTables";
     namespace wrappers
     {
         //typedef mitk::BoolLookupTable BoolLookupTable; //cswig crashes; D:\CableSwig\CableSwig\SWIG\Source\Modules\lang.cxx line 1717 symname = NULL; no check for this case
         typedef mitk::FloatLookupTable FloatLookupTable;
         typedef mitk::IntLookupTable IntLookupTable;
         typedef mitk::StringLookupTable StringLookupTable;
     }
}

#endif

