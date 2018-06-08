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

#include "mitkSimpleVolumeDICOMSeriesReaderService.h"
#include "mitkDICOMReaderConfigurator.h"
#include <mitkDICOMITKSeriesGDCMReader.h>

#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleResourceStream.h>
#include <usModule.h>
#include <usModuleRegistry.h>

namespace mitk {

  SimpleVolumeDICOMSeriesReaderService::SimpleVolumeDICOMSeriesReaderService()
  : BaseDICOMReaderService("MITK Simple 3D Volume Importer")
{
  this->RegisterService();
}

DICOMFileReader::Pointer SimpleVolumeDICOMSeriesReaderService::GetReader(const mitk::StringList& relevantFiles) const
{
  mitk::StringList files = relevantFiles;
  std::string descr;

  us::ModuleResource resource = 
    us::ModuleRegistry::GetModule("MitkDICOMReader")->GetResource("configurations/3D/simpleinstancenumber_soft.xml");

  if ( resource.IsValid() )
  {
    us::ModuleResourceStream stream(resource);

    stream.seekg(0, std::ios::end);
    descr.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);

    descr.assign((std::istreambuf_iterator<char>(stream)),
      std::istreambuf_iterator<char>());
  }

  DICOMReaderConfigurator::Pointer configurator = DICOMReaderConfigurator::New();
  DICOMFileReader::Pointer reader = configurator->CreateFromUTF8ConfigString(descr);

  return reader.GetPointer();
};


  SimpleVolumeDICOMSeriesReaderService* SimpleVolumeDICOMSeriesReaderService::Clone() const
  {
    return new SimpleVolumeDICOMSeriesReaderService(*this);
  }

}
