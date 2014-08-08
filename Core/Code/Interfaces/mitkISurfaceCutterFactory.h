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

#ifndef mitkISurfaceCutterFactory_h
#define mitkISurfaceCutterFactory_h

#include "MitkCoreExports.h"
#include "mitkISurfaceCutter.h"

#include <usServiceInterface.h>

namespace mitk
{
  /**
    * \brief Interface for surface cutting algorithms.
    * \ingroup Data
    */
    class MITK_CORE_EXPORT ISurfaceCutterFactory
    {
    public:
        virtual ~ISurfaceCutterFactory() {}

        /** Create the surface cutter instance
         */
        virtual ISurfaceCutter* createSurfaceCutter() const = 0;
     };
}

US_DECLARE_SERVICE_INTERFACE(mitk::ISurfaceCutterFactory, "org.mitk.ISurfaceCutterFactory")

#endif
