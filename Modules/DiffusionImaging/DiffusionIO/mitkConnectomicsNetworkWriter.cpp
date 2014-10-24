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
#include "mitkDiffusionIOMimeTypes.h"

mitk::ConnectomicsNetworkWriter::ConnectomicsNetworkWriter()
  : AbstractFileWriter(mitk::ConnectomicsNetwork::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_DESCRIPTION() )
{
  RegisterService();
}

mitk::ConnectomicsNetworkWriter::ConnectomicsNetworkWriter(const mitk::ConnectomicsNetworkWriter& other)
  : AbstractFileWriter(other)
{
}


mitk::ConnectomicsNetworkWriter::~ConnectomicsNetworkWriter()
{}

mitk::ConnectomicsNetworkWriter* mitk::ConnectomicsNetworkWriter::Clone() const
{
  return new ConnectomicsNetworkWriter(*this);
}

void mitk::ConnectomicsNetworkWriter::Write()
{
  MITK_INFO << "Writing connectomics network";
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull() )
  {
    MITK_ERROR <<"Sorry, input to ConnectomicsNetworkWriter is NULL!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!" ;
    return ;
  }

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(this->GetOutputLocation());
  ext = itksys::SystemTools::LowerCase(ext);

  // default extension is .cnf
  if(ext == "")
  {
    ext = ".cnf";
    this->SetOutputLocation(this->GetOutputLocation() + ext);
  }

  if (ext == ".cnf")
  {
    // Get geometry of the network
    mitk::BaseGeometry* geometry = input->GetGeometry();

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
        VertexVectorType vertexVector = dynamic_cast<const InputType*>(this->GetInput())->GetVectorOfAllNodes();
        for( unsigned int index = 0; index < vertexVector.size(); index++ )
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
        EdgeVectorType edgeVector = dynamic_cast<const InputType*>(this->GetInput())->GetVectorOfAllEdges();
        for(unsigned  int index = 0; index < edgeVector.size(); index++ )
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
    documentXML.SaveFile( this->GetOutputLocation().c_str() );
    MITK_INFO << "Connectomics network written";

  }
}
