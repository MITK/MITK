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
                         1,
                         itk::CreateObjectFunction<IOAdapter<PlanarFigureReader> >::New());
}

PlanarFigureIOFactory::~PlanarFigureIOFactory()
{
}

const char* PlanarFigureIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PlanarFigureIOFactory::GetDescription() const
{
  return "PlanarFigure IO Factory, allows the loading of .pf files";
}

} // end namespace mitk
