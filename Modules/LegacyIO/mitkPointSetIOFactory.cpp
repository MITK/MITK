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

#include "mitkPointSetIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkPointSetReader.h"

#include "itkVersion.h"


namespace mitk
{
PointSetIOFactory::PointSetIOFactory()
{
  this->RegisterOverride("mitkIOAdapter",
                         "mitkPointSetReader",
                         "mitk PointSet IO",
                         1,
                         itk::CreateObjectFunction<IOAdapter<PointSetReader> >::New());
}

PointSetIOFactory::~PointSetIOFactory()
{
}

const char* PointSetIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* PointSetIOFactory::GetDescription() const
{
  return "PointSet IO Factory, allows the loading of MITK pointsets";
}

} // end namespace mitk
