/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11. Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
