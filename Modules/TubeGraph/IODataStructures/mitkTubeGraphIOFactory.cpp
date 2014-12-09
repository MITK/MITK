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

#include "mitkTubeGraphIOFactory.h"
#include "mitkIOAdapter.h"
#include "mitkTubeGraphReader.h"

#include "itkVersion.h"


namespace mitk
{

TubeGraphIOFactory::TubeGraphIOFactory()
{
  typedef TubeGraphReader TubeGraphReaderType;
  this->RegisterOverride("mitkIOAdapter", //beibehalten
                         "mitkTubeGraphReader", //umbenennen
                         "Tube Graph IO", //angezeigter name
                         1,
                         itk::CreateObjectFunction<IOAdapter<TubeGraphReaderType> >::New());
}

TubeGraphIOFactory::~TubeGraphIOFactory()
{
}

const char* TubeGraphIOFactory::GetITKSourceVersion() const
{
  return ITK_SOURCE_VERSION;
}

const char* TubeGraphIOFactory::GetDescription() const
{
  return "TubeGraphIOFactory, allows the loading of Tube Graphs";
}

} // end namespace mitk
