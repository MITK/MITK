/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlanarFigureIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkPlanarFigureReader.h"

#include "itkVersion.h"

namespace mitk
{
  PlanarFigureIOFactory::PlanarFigureIOFactory()
  {
    this->RegisterOverride("mitkIOAdapter",
                           "mitkPlanarFigureReader",
                           "mitk PlanarFigure IO",
                           true,
                           itk::CreateObjectFunction<IOAdapter<PlanarFigureReader>>::New());
  }

  PlanarFigureIOFactory::~PlanarFigureIOFactory() {}
  const char *PlanarFigureIOFactory::GetITKSourceVersion() const { return ITK_SOURCE_VERSION; }
  const char *PlanarFigureIOFactory::GetDescription() const
  {
    return "PlanarFigure IO Factory, allows the loading of .pf files";
  }

} // end namespace mitk
