/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12. Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkPlanarFigureWriter.h"
#include "mitkBasePropertySerializer.h"

#include <tinyxml.h>


mitk::PlanarFigureWriter::PlanarFigureWriter()
: m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Extension(".pf"),
  m_MimeType("application/MITK.PlanarFigure"), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
  this->SetNumberOfOutputs( 0 );
  //this->SetNthOutput( 0, mitk::PlanarFigure::New().GetPointer() );
}


mitk::PlanarFigureWriter::~PlanarFigureWriter()
{}


void mitk::PlanarFigureWriter::GenerateData()
{
  m_Success = false;

  if (m_FileName.empty())
  {
    MITK_ERROR << "Could not write planar figures. File name is invalid";
    throw std::invalid_argument("file name is empty");
  }
 
  TiXmlDocument document;
  TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
  document.LinkEndChild( decl );

  TiXmlElement* version = new TiXmlElement("Version");
  version->SetAttribute("Writer",  __FILE__ );
  version->SetAttribute("CVSRevision",  "$Revision: 17055 $" );
  version->SetAttribute("FileVersion",  1 );
  document.LinkEndChild(version);

  
  /* create xml element for each input */
  for ( unsigned int i = 0 ; i < this->GetNumberOfInputs(); ++i )
  {
    // Create root element for this PlanarFigure
    InputType::Pointer pf = this->GetInput( i );
    if (pf.IsNull())
      continue;
    TiXmlElement* pfElement = new TiXmlElement("PlanarFigure");
    pfElement->SetAttribute("type", pf->GetNameOfClass());
    document.LinkEndChild(pfElement);

    PlanarFigure::VertexContainerType* vertices = pf->GetControlPoints();
    if (vertices == NULL)
      continue;

    // Serialize property list of PlanarFigure
    mitk::PropertyList::Pointer propertyList = pf->GetPropertyList();
    mitk::PropertyList::PropertyMap::const_iterator it;
    for ( it = propertyList->GetMap()->begin(); it != propertyList->GetMap()->end(); ++it )
    {
      // Create seralizer for this property
      const mitk::BaseProperty* prop = it->second.first;
      std::string serializerName = std::string( prop->GetNameOfClass() ) + "Serializer";
      std::list< itk::LightObject::Pointer > allSerializers = itk::ObjectFactoryBase::CreateAllInstance(
        serializerName.c_str() );

      if ( allSerializers.size() != 1 )
      {
        // No or too many serializer(s) found, skip this property
        continue;
      }

      mitk::BasePropertySerializer* serializer = dynamic_cast< mitk::BasePropertySerializer* >(
        allSerializers.begin()->GetPointer() );
      if ( serializer == NULL )
      {
        // Serializer not valid; skip this property
      }

      TiXmlElement* keyElement = new TiXmlElement( "property" );
      keyElement->SetAttribute( "key", it->first );
      keyElement->SetAttribute( "type", prop->GetNameOfClass() );

        serializer->SetProperty( prop );
      TiXmlElement* valueElement = NULL;
      try
      {
        valueElement = serializer->Serialize();
      }
      catch (...)
      {
      }

      if ( valueElement == NULL )
      {
        // Serialization failed; skip this property
        continue;
      }

      // Add value to property element
      keyElement->LinkEndChild( valueElement );

      // Append serialized property to property list
      pfElement->LinkEndChild( keyElement );
    }

    // Serialize control points of PlanarFigure
    TiXmlElement* controlPointsElement = new TiXmlElement("ControlPoints");
    pfElement->LinkEndChild(controlPointsElement);
    for (PlanarFigure::VertexContainerType::ConstIterator it = vertices->Begin(); it != vertices->End(); ++it)
    {
      TiXmlElement* vElement = new TiXmlElement("Vertex");
      vElement->SetAttribute("id", it->Index());
      vElement->SetDoubleAttribute("x", it->Value().GetElement(0));
      vElement->SetDoubleAttribute("y", it->Value().GetElement(1));
      controlPointsElement->LinkEndChild(vElement);
    }
    TiXmlElement* geoElement = new TiXmlElement("Geometry");
    const PlaneGeometry* planeGeo = dynamic_cast<const PlaneGeometry*>(pf->GetGeometry2D());
    if (planeGeo != NULL)
    {
      // Write parameters of IndexToWorldTransform of the PlaneGeometry
      typedef mitk::AffineGeometryFrame3D::TransformType TransformType;
      const TransformType* affineGeometry = planeGeo->GetIndexToWorldTransform();
      const TransformType::ParametersType& parameters = affineGeometry->GetParameters();
      TiXmlElement* vElement = new TiXmlElement( "transformParam" );
      for ( unsigned int i = 0; i < affineGeometry->GetNumberOfParameters(); ++i )
      {
        std::stringstream paramName; 
        paramName << "param" << i;
        vElement->SetDoubleAttribute( paramName.str().c_str(), parameters.GetElement( i ) );
      }
      geoElement->LinkEndChild( vElement );

      // Write bounds of the PlaneGeometry
      typedef mitk::Geometry3D::BoundsArrayType BoundsArrayType;
      const BoundsArrayType& bounds = planeGeo->GetBounds();
      vElement = new TiXmlElement( "boundsParam" );
      for ( unsigned int i = 0; i < 6; ++i )
      {
        std::stringstream boundName; 
        boundName << "bound" << i;
        vElement->SetDoubleAttribute( boundName.str().c_str(), bounds.GetElement( i ) );
      }
      geoElement->LinkEndChild( vElement );

      // Write spacing and origin of the PlaneGeometry
      Vector3D spacing = planeGeo->GetSpacing();
      Point3D origin = planeGeo->GetOrigin();
      geoElement->LinkEndChild(this->CreateXMLVectorElement("Spacing", spacing));
      geoElement->LinkEndChild(this->CreateXMLVectorElement("Origin", origin));

      pfElement->LinkEndChild(geoElement);
    }
  }

  if (document.SaveFile( m_FileName) == false)
  {
    MITK_ERROR << "Could not write planar figures to " << m_FileName << "\nTinyXML reports '" << document.ErrorDesc() << "'";
    throw std::ios_base::failure("Error during writing of planar figure xml file.");
  }
  m_Success = true;
}


TiXmlElement* mitk::PlanarFigureWriter::CreateXMLVectorElement(const char* name, itk::FixedArray<mitk::ScalarType, 3> v)
{
  TiXmlElement* vElement = new TiXmlElement(name);
  vElement->SetDoubleAttribute("x", v.GetElement(0));
  vElement->SetDoubleAttribute("y", v.GetElement(1));
  vElement->SetDoubleAttribute("z", v.GetElement(2));
  return vElement;
}


void mitk::PlanarFigureWriter::ResizeInputs( const unsigned int& num )
{
  //unsigned int prevNum = this->GetNumberOfInputs();
  this->SetNumberOfInputs( num );
  //for ( unsigned int i = prevNum; i < num; ++i )
  //{
  //  this->SetNthInput( i, mitk::PlanarFigure::New().GetPointer() );
  //}
}


void mitk::PlanarFigureWriter::SetInput( InputType* PlanarFigure )
{
  this->ProcessObject::SetNthInput( 0, PlanarFigure );
}


void mitk::PlanarFigureWriter::SetInput( const unsigned int& id, InputType* PlanarFigure )
{
  if ( id >= this->GetNumberOfInputs() )
    this->ResizeInputs( id + 1 );
  this->ProcessObject::SetNthInput( id, PlanarFigure );
}


mitk::PlanarFigure* mitk::PlanarFigureWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
    return NULL;
  else
    return dynamic_cast<InputType*> ( this->GetInput( 0 ) );
}


mitk::PlanarFigure* mitk::PlanarFigureWriter::GetInput( const unsigned int& num )
{
  return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( num ) );
}


bool mitk::PlanarFigureWriter::CanWriteDataType( DataNode* input )
{
  if ( input == NULL )
    return false;

  mitk::BaseData* data = input->GetData();
  if ( data  == NULL)
    return false;

  mitk::PlanarFigure::Pointer PlanarFigure = dynamic_cast<mitk::PlanarFigure*>( data );
  if( PlanarFigure.IsNull() )
    return false;
  // add code for special subclasses here
  return true;
}


void mitk::PlanarFigureWriter::SetInput( DataNode* input )
{
  if (this->CanWriteDataType(input))
    this->ProcessObject::SetNthInput( 0, dynamic_cast<mitk::PlanarFigure*>( input->GetData() ) );
}


std::string mitk::PlanarFigureWriter::GetWritenMIMEType()
{
  return m_MimeType;
}


std::vector<std::string> mitk::PlanarFigureWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(m_Extension);
  return possibleFileExtensions;
}


std::string mitk::PlanarFigureWriter::GetFileExtension()
{
  return m_Extension;
}
