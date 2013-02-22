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

#include "mitkConnectomicsNetworkWriter.h"
#include "mitkConnectomicsNetworkDefinitions.h"
#include <tinyxml.h>
#include "itksys/SystemTools.hxx"

mitk::ConnectomicsNetworkWriter::ConnectomicsNetworkWriter()
: m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}


mitk::ConnectomicsNetworkWriter::~ConnectomicsNetworkWriter()
{}


void mitk::ConnectomicsNetworkWriter::GenerateData()
{
  MITK_INFO << "Writing connectomics network";
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to ConnectomicsNetworkWriter is NULL!");
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);

  if (ext == ".cnf")
  {
    // Get geometry of the network
    mitk::Geometry3D* geometry = input->GetGeometry();

    // Create XML document
    TiXmlDocument documentXML;
    { // begin document
      TiXmlDeclaration* declXML = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
      documentXML.LinkEndChild( declXML );

      TiXmlElement* mainXML = new TiXmlElement(mitk::ConnectomicsNetworkDefinitions::XML_CONNECTOMICS_FILE);
      mainXML->SetAttribute(mitk::ConnectomicsNetworkDefinitions::XML_FILE_VERSION, mitk::ConnectomicsNetworkDefinitions::VERSION_STRING);
      documentXML.LinkEndChild(mainXML);

      TiXmlElement* geometryXML = new TiXmlElement(mitk::ConnectomicsNetworkDefinitions::XML_GEOMETRY);
      { // begin geometry
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XX, geometry->GetMatrixColumn(0)[0]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XY, geometry->GetMatrixColumn(0)[1]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XZ, geometry->GetMatrixColumn(0)[2]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YX, geometry->GetMatrixColumn(1)[0]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YY, geometry->GetMatrixColumn(1)[1]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YZ, geometry->GetMatrixColumn(1)[2]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZX, geometry->GetMatrixColumn(2)[0]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZY, geometry->GetMatrixColumn(2)[1]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZZ, geometry->GetMatrixColumn(2)[2]);

        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_X, geometry->GetOrigin()[0]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_Y, geometry->GetOrigin()[1]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_Z, geometry->GetOrigin()[2]);

        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_SPACING_X, geometry->GetSpacing()[0]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_SPACING_Y, geometry->GetSpacing()[1]);
        geometryXML->SetDoubleAttribute(mitk::ConnectomicsNetworkDefinitions::XML_SPACING_Z, geometry->GetSpacing()[2]);

      } // end geometry
      mainXML->LinkEndChild(geometryXML);

      TiXmlElement* verticesXML = new TiXmlElement(mitk::ConnectomicsNetworkDefinitions::XML_VERTICES);
      { // begin vertices section
        VertexVectorType vertexVector = this->GetInput()->GetVectorOfAllNodes();
        for( int index = 0; index < vertexVector.size(); index++ )
        {
          // not localized as of yet TODO
          TiXmlElement* vertexXML = new TiXmlElement(mitk::ConnectomicsNetworkDefinitions::XML_VERTEX );
          vertexXML->SetAttribute( mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_ID , vertexVector[ index ].id );
          vertexXML->SetAttribute( mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_LABEL , vertexVector[ index ].label );
          vertexXML->SetDoubleAttribute( mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_X , vertexVector[ index ].coordinates[0] );
          vertexXML->SetDoubleAttribute( mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_Y , vertexVector[ index ].coordinates[1] );
          vertexXML->SetDoubleAttribute( mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_Z , vertexVector[ index ].coordinates[2] );
          verticesXML->LinkEndChild(vertexXML);
        }
      } // end vertices section
      mainXML->LinkEndChild(verticesXML);

      TiXmlElement* edgesXML = new TiXmlElement(mitk::ConnectomicsNetworkDefinitions::XML_EDGES);
      { // begin edges section
        EdgeVectorType edgeVector = this->GetInput()->GetVectorOfAllEdges();
        for( int index = 0; index < edgeVector.size(); index++ )
        {
          TiXmlElement* edgeXML = new TiXmlElement(mitk::ConnectomicsNetworkDefinitions::XML_EDGE );
          edgeXML->SetAttribute( mitk::ConnectomicsNetworkDefinitions::XML_EDGE_ID , index );
          edgeXML->SetAttribute( mitk::ConnectomicsNetworkDefinitions::XML_EDGE_SOURCE_ID , edgeVector[ index ].second.sourceId );
          edgeXML->SetAttribute( mitk::ConnectomicsNetworkDefinitions::XML_EDGE_TARGET_ID , edgeVector[ index ].second.targetId );
          edgeXML->SetAttribute( mitk::ConnectomicsNetworkDefinitions::XML_EDGE_WEIGHT_ID , edgeVector[ index ].second.weight );
          edgesXML->LinkEndChild(edgeXML);
        }
      } // end edges section
      mainXML->LinkEndChild(edgesXML);

    } // end document
    documentXML.SaveFile( m_FileName );

    m_Success = true;

    MITK_INFO << "Connectomics network written";

  }
}


void mitk::ConnectomicsNetworkWriter::SetInputConnectomicsNetwork( InputType* conNetwork )
{
  this->ProcessObject::SetNthInput( 0, conNetwork );
}


mitk::ConnectomicsNetwork* mitk::ConnectomicsNetworkWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( 0 ) );
  }
}


std::vector<std::string> mitk::ConnectomicsNetworkWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".cnf");
  return possibleFileExtensions;
}
