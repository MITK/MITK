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

#include "mitkSurfaceCutterFactoryPerformanceSelector.h"

#include "mitkSurfaceCutterVtk.h"
#include "mitkSurfaceCutterCGAL.h"

namespace mitk {
    SurfaceCutterFactoryPerformanceSelector::SurfaceCutterFactoryPerformanceSelector()
    {
        // TODO: perform test and choose cutter
    }

    SurfaceCutterFactoryPerformanceSelector::~SurfaceCutterFactoryPerformanceSelector()
    {

    }

    ISurfaceCutter* SurfaceCutterFactoryPerformanceSelector::createSurfaceCutter() const
    {
        return new SurfaceCutterCGAL();
    }

} // namespace mitk
