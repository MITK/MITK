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

#include "mitkGeometryDataWriterService.h"
#include "mitkGeometryDataReaderService.h"
#include "mitkIOMimeTypes.h"

#include <tinyxml.h>

#include <iostream>
#include <fstream>
#include <locale>

#include <mitkLocaleSwitch.h>

mitk::GeometryDataWriterService::GeometryDataWriterService()
  : AbstractFileWriter(GeometryData::GetStaticNameOfClass(),
                       GeometryDataReaderService::GEOMETRY_DATA_MIMETYPE(),
                       "MITK Geometry Data Writer")
{
  RegisterService();
}

mitk::GeometryDataWriterService::GeometryDataWriterService(const mitk::GeometryDataWriterService& other)
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

  if ( !out.good() )
  {
    mitkThrow() << "Stream not good.";
  }
  
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  // Open XML file using TinyXML,
  // loop over all inputs,
  //   call appropriate serializing functions

  TiXmlDocument doc;

  TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", ""); // TODO what to write here? encoding? standalone would mean that we provide a DTD somewhere...
  doc.LinkEndChild(decl);
  
  TiXmlElement* rootNode = new TiXmlElement("GeometryData");
  doc.LinkEndChild(rootNode);

  // note version info
  TiXmlElement* version = new TiXmlElement("Version");
  version->SetAttribute("Writer", __FILE__);
  version->SetAttribute("FileVersion", 1);
  rootNode->LinkEndChild(version);

  const GeometryData* data = static_cast<const GeometryData*>( this->GetInput() );

  const Geometry3D* geom3D(NULL);
  if ( geom3D = dynamic_cast<const Geometry3D*>( data->GetGeometry() ) )
  {
      WriteSpecific(rootNode, geom3D);
  }

  // Write out document
  out << doc;
}

mitk::GeometryDataWriterService*mitk::GeometryDataWriterService::Clone() const
{
  return new GeometryDataWriterService(*this);
}

void mitk::GeometryDataWriterService::WriteSpecific(TiXmlNode* rootNode, const Geometry3D* geom3D)
{
  assert(rootNode);
  assert(geom3D);

  // really serialize
  const AffineTransform3D* transform = geom3D->GetIndexToWorldTransform();

  // get transform parameters that would need to be serialized
  AffineTransform3D::MatrixType matrix = transform->GetMatrix();
  AffineTransform3D::OffsetType offset = transform->GetOffset();
  
  Point3D origin = geom3D->GetOrigin();
  Vector3D spacing = geom3D->GetSpacing();
  bool isImageGeometry = geom3D->GetImageGeometry();
  BaseGeometry::BoundsArrayType bounds = geom3D->GetBounds();

  // create XML file
  // construct XML tree describing the geometry
  TiXmlElement* geomElem = new TiXmlElement("Geometry3D");
  geomElem->SetAttribute("ImageGeometry", isImageGeometry ? "true" : "false" );
  geomElem->SetAttribute("FrameOfReferenceID", geom3D->GetFrameOfReferenceID());

  rootNode->LinkEndChild(geomElem);

  // IndexToWorldTransform: matrix, offset, center, translation, scale? (which ones are redundant?? unit tests, search slides) 
  // BoundingBox
  // Origin
  // ImageGeometry
  // Spacing
  // FrameOfReference

  // coefficients are matrix[row][column]!
  TiXmlElement* matrixElem = new TiXmlElement("IndexToWorld");
  matrixElem->SetAttribute("type", "Matrix3x3");
  matrixElem->SetDoubleAttribute("m_0_0", matrix[0][0]);
  matrixElem->SetDoubleAttribute("m_0_1", matrix[0][1]);
  matrixElem->SetDoubleAttribute("m_0_2", matrix[0][2]);
  matrixElem->SetDoubleAttribute("m_1_0", matrix[1][0]);
  matrixElem->SetDoubleAttribute("m_1_1", matrix[1][1]);
  matrixElem->SetDoubleAttribute("m_1_2", matrix[1][2]);
  matrixElem->SetDoubleAttribute("m_2_0", matrix[2][0]);
  matrixElem->SetDoubleAttribute("m_2_1", matrix[2][1]);
  matrixElem->SetDoubleAttribute("m_2_2", matrix[2][2]);
  geomElem->LinkEndChild(matrixElem);
  
  TiXmlElement* offsetElem = new TiXmlElement("Offset");
  offsetElem->SetAttribute("type", "Vector3D");
  offsetElem->SetDoubleAttribute("x", offset[0]);
  offsetElem->SetDoubleAttribute("y", offset[1]);
  offsetElem->SetDoubleAttribute("z", offset[2]);
  geomElem->LinkEndChild(offsetElem);

  TiXmlElement* boundsElem = new TiXmlElement("Bounds");
  TiXmlElement* boundsMinElem = new TiXmlElement("Min");
  boundsMinElem->SetAttribute("type", "Vector3D");
  boundsMinElem->SetDoubleAttribute("x", bounds[0]);
  boundsMinElem->SetDoubleAttribute("y", bounds[2]);
  boundsMinElem->SetDoubleAttribute("z", bounds[4]);
  boundsElem->LinkEndChild(boundsMinElem);
  TiXmlElement* boundsMaxElem = new TiXmlElement("Max");
  boundsMaxElem->SetAttribute("type", "Vector3D");
  boundsMaxElem->SetDoubleAttribute("x", bounds[1]);
  boundsMaxElem->SetDoubleAttribute("y", bounds[3]);
  boundsMaxElem->SetDoubleAttribute("z", bounds[5]);
  boundsElem->LinkEndChild(boundsMaxElem);
  geomElem->LinkEndChild(boundsElem);

  TiXmlElement* originElem = new TiXmlElement("Origin");
  originElem->SetAttribute("type", "Point3D");
  originElem->SetDoubleAttribute("x", origin[0]);
  originElem->SetDoubleAttribute("y", origin[1]);
  originElem->SetDoubleAttribute("z", origin[2]);
  geomElem->LinkEndChild(originElem);

  TiXmlElement* spacingElem = new TiXmlElement("Spacing");
  spacingElem->SetAttribute("type", "Vector3D");
  spacingElem->SetDoubleAttribute("x", spacing[0]);
  spacingElem->SetDoubleAttribute("y", spacing[1]);
  spacingElem->SetDoubleAttribute("z", spacing[2]);
  geomElem->LinkEndChild(spacingElem);
}