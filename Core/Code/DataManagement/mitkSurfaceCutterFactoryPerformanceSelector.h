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

#ifndef mitkSurfaceCutterFactoryPerformanceSelector_h
#define mitkSurfaceCutterFactoryPerformanceSelector_h

#include "mitkISurfaceCutterFactory.h"

namespace mitk
{
  /**
    * \brief The factory for surface cutter which selects one with highest performance out of all available cutters
    * \ingroup Data
    */
      class MITK_CORE_EXPORT SurfaceCutterFactoryPerformanceSelector : public ISurfaceCutterFactory
      {
      public:
          SurfaceCutterFactoryPerformanceSelector();
          virtual ~SurfaceCutterFactoryPerformanceSelector();

          virtual ISurfaceCutter* createSurfaceCutter() const;
      };
} // namespace mitk

#endif // mitkSurfaceCutterFactoryPerformanceSelector_h
