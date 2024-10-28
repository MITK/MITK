#ifndef MITKHALFLIFECONSTANTS_H
#define MITKHALFLIFECONSTANTS_H

#include <string>
#include "MitkPETExports.h"

namespace mitk
{
struct MITKPET_EXPORT HALFLIFECONSTANTS
{
    static const std::string NAME_18F;
    /**half life of 18F in [sec]*/
    static const double VALUE_18F;
    static const std::string NAME_68Ga;
    /**half life of 68Ga in [sec]*/
    static const double VALUE_68Ga;
    static const std::string NAME_11C;
    /**half life of 11C in [sec]*/
    static const double VALUE_11C;
    static const std::string NAME_15O;
    /**half life of 15O in [sec]*/
    static const double VALUE_15O;
};

}

#endif // HALFLIFECONSTANTS_H
