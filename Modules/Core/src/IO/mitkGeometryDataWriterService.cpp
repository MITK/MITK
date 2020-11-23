/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGeometryDataWriterService.h"
#include "mitkIOMimeTypes.h"

#include "mitkProportionalTimeGeometryToXML.h"

#include <tinyxml2.h>

#include <mitkLocaleSwitch.h>

mitk::GeometryDataWriterService::GeometryDataWriterService()
  : AbstractFileWriter(
      GeometryData::GetStaticNameOfClass(), IOMimeTypes::GEOMETRY_DATA_MIMETYPE(), "MITK Geometry Data Writer")
{
  RegisterService();
}

mitk::GeometryDataWriterService::GeometryDataWriterService(const mitk::GeometryDataWriterService &other)
  : AbstractFileWriter(other)
{
}

mitk::GeometryDataWriterService::~GeometryDataWriterService()
{
}

void mitk::GeometryDataWriterService::Write()
{
  /* using the stream interface produces files without line breaks or indentation..
     But before changing to file interface, need to understand the new I/O classes */
  OutputStream out(this);

  if (!out.good())
  {
    mitkThrow() << "Stream not good.";
  }

  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  // Open XML file using TinyXML,
  // loop over all inputs,
  //   call appropriate serializing functions

  tinyxml2::XMLDocument doc;
  doc.InsertEndChild(doc.NewDeclaration());

  auto* rootNode = doc.NewElement("GeometryData");
  doc.InsertEndChild(rootNode);

  // note version info
  auto *version = doc.NewElement("Version");
  version->SetAttribute("Writer", __FILE__);
  version->SetAttribute("FileVersion", 1);
  rootNode->InsertEndChild(version);

  const auto *data = static_cast<const GeometryData *>(this->GetInput());

  const ProportionalTimeGeometry *timeGeometry(nullptr);
  if ((timeGeometry = dynamic_cast<const ProportionalTimeGeometry *>(data->GetTimeGeometry())))
  {
    auto *timeGeometryElement = ProportionalTimeGeometryToXML::ToXML(doc, timeGeometry);
    rootNode->InsertEndChild(timeGeometryElement);
  }
  else
  {
    MITK_WARN << "Serializing GeometryData that does not have a valid ProportionalTimeGeometry! Not implemented!";
  }

  // Write out document
  tinyxml2::XMLPrinter printer;
  doc.Print(&printer);
  out << printer.CStr();
}

mitk::GeometryDataWriterService *mitk::GeometryDataWriterService::Clone() const
{
  return new GeometryDataWriterService(*this);
}
