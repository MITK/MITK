/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkModelFitException_h
#define mitkModelFitException_h

#include "mitkExceptionMacro.h"

namespace mitk
{
    namespace modelFit
    {
        /**
         *  @brief  Class-specific exception for modelfits.
         */
        class MITKMODELFIT_EXPORT ModelFitException : public Exception
        {
        public:
            mitkExceptionClassMacro(ModelFitException, Exception);
        };
    }
}

#endif // mitkModelFitException_h
