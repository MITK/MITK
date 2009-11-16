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
    LOG_ERROR << "Could not write planar figures. File name is invalid";
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
    InputType::Pointer pf = this->GetInput( i );
    if (pf.IsNull())
      continue;
    TiXmlElement* pfElement = new TiXmlElement("PlanarFigure");
    pfElement->SetAttribute("type", pf->GetNameOfClass());
    if ( pf->IsClosed() )
    {
      pfElement->SetAttribute( "closed", "true" );
    }
    else
    {
      pfElement->SetAttribute( "closed", "false" );
    }
    document.LinkEndChild(pfElement);
    PlanarFigure::VertexContainerType* vertices = pf->GetControlPoints();
    if (vertices == NULL)
      continue;
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
      Vector3D xVector = planeGeo->GetAxisVector( 0 );
      Vector3D yVector = planeGeo->GetAxisVector( 1 );
      Vector3D spacing = planeGeo->GetSpacing();
      Point3D origin = planeGeo->GetOrigin();

      geoElement->LinkEndChild(this->CreateXMLVectorElement("xVector", xVector));
      geoElement->LinkEndChild(this->CreateXMLVectorElement("yVector", yVector));
      geoElement->LinkEndChild(this->CreateXMLVectorElement("Spacing", spacing));
      geoElement->LinkEndChild(this->CreateXMLVectorElement("Origin", origin));
      pfElement->LinkEndChild(geoElement);
    }
  }

  if (document.SaveFile( m_FileName) == false)
  {
    LOG_ERROR << "Could not write planar figures to " << m_FileName << "\nTinyXML reports '" << document.ErrorDesc() << "'";
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
  unsigned int prevNum = this->GetNumberOfInputs();
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


bool mitk::PlanarFigureWriter::CanWriteDataType( DataTreeNode* input )
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


void mitk::PlanarFigureWriter::SetInput( DataTreeNode* input )
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
