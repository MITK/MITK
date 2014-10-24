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

#include "mitkConnectomicsNetworkReader.h"
#include "mitkConnectomicsNetworkDefinitions.h"
#include <tinyxml.h>
#include "itksys/SystemTools.hxx"
#include <vtkMatrix4x4.h>
#include "mitkGeometry3D.h"
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"

namespace mitk
{

  ConnectomicsNetworkReader::ConnectomicsNetworkReader(const ConnectomicsNetworkReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  ConnectomicsNetworkReader::ConnectomicsNetworkReader()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_DESCRIPTION() )
  {
    m_ServiceReg = this->RegisterService();
  }

  ConnectomicsNetworkReader::~ConnectomicsNetworkReader()
  {
  }

  std::vector<itk::SmartPointer<BaseData> > ConnectomicsNetworkReader::Read()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;
    std::string location = GetInputLocation();

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(location);
    ext = itksys::SystemTools::LowerCase(ext);

    if ( location == "")
    {
      MITK_ERROR << "No file name specified.";
    }
    else if (ext == ".cnf")
    {
      try
      {
        mitk::ConnectomicsNetwork::Pointer outputNetwork = mitk::ConnectomicsNetwork::New();

        TiXmlDocument doc( location );
        bool loadOkay = doc.LoadFile();
        if(!loadOkay)
        {
          mitkThrow() << "Could not open file " << location << " for reading.";
        }

        TiXmlHandle hDoc(&doc);
        TiXmlElement* pElem;
        TiXmlHandle hRoot(0);

        pElem = hDoc.FirstChildElement().Element();

        // save this for later
        hRoot = TiXmlHandle(pElem);

        pElem = hRoot.FirstChildElement(mitk::ConnectomicsNetworkDefinitions::XML_GEOMETRY).Element();

        // read geometry
        mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();

        // read origin
        mitk::Point3D origin;
        double temp = 0;
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_X, &temp);
        origin[0] = temp;
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_Y, &temp);
        origin[1] = temp;
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_ORIGIN_Z, &temp);
        origin[2] = temp;
        geometry->SetOrigin(origin);

        // read spacing
        ScalarType spacing[3];
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_SPACING_X, &temp);
        spacing[0] = temp;
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_SPACING_Y, &temp);
        spacing[1] = temp;
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_SPACING_Z, &temp);
        spacing[2] = temp;
        geometry->SetSpacing(spacing);

        // read transform
        vtkMatrix4x4* m = vtkMatrix4x4::New();
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XX, &temp);
        m->SetElement(0,0,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XY, &temp);
        m->SetElement(1,0,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_XZ, &temp);
        m->SetElement(2,0,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YX, &temp);
        m->SetElement(0,1,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YY, &temp);
        m->SetElement(1,1,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_YZ, &temp);
        m->SetElement(2,1,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZX, &temp);
        m->SetElement(0,2,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZY, &temp);
        m->SetElement(1,2,temp);
        pElem->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_MATRIX_ZZ, &temp);
        m->SetElement(2,2,temp);

        m->SetElement(0,3,origin[0]);
        m->SetElement(1,3,origin[1]);
        m->SetElement(2,3,origin[2]);
        m->SetElement(3,3,1);
        geometry->SetIndexToWorldTransformByVtkMatrix(m);

        geometry->SetImageGeometry(true);
        outputNetwork->SetGeometry(geometry);

        // read network
        std::map< int, mitk::ConnectomicsNetwork::VertexDescriptorType > idToVertexMap;
        // read vertices
        pElem = hRoot.FirstChildElement(mitk::ConnectomicsNetworkDefinitions::XML_VERTICES).Element();
        {
          // walk through the vertices
          TiXmlElement* vertexElement = pElem->FirstChildElement();

          for( ; vertexElement; vertexElement=vertexElement->NextSiblingElement())
          {
            std::vector< float > pos;
            std::string label;
            int vertexID(0);

            vertexElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_X, &temp);
            pos.push_back(temp);
            vertexElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_Y, &temp);
            pos.push_back(temp);
            vertexElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_Z, &temp);
            pos.push_back(temp);
            vertexElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_ID, &vertexID);
            vertexElement->QueryStringAttribute(mitk::ConnectomicsNetworkDefinitions::XML_VERTEX_LABEL, &label);

            mitk::ConnectomicsNetwork::VertexDescriptorType newVertex = outputNetwork->AddVertex( vertexID );
            outputNetwork->SetLabel( newVertex, label );
            outputNetwork->SetCoordinates( newVertex, pos );

            if ( idToVertexMap.count( vertexID ) > 0 )
            {
              MITK_ERROR << "Aborting network creation, duplicate vertex ID in file.";
              return result;
            }
            idToVertexMap.insert( std::pair< int, mitk::ConnectomicsNetwork::VertexDescriptorType >( vertexID, newVertex) );
          }
        }

        // read edges
        pElem = hRoot.FirstChildElement(mitk::ConnectomicsNetworkDefinitions::XML_EDGES).Element();
        {
          // walk through the edges
          TiXmlElement* edgeElement = pElem->FirstChildElement();

          for( ; edgeElement; edgeElement=edgeElement->NextSiblingElement())
          {
            int edgeID(0), edgeSourceID(0), edgeTargetID(0), edgeWeight(0);

            edgeElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_EDGE_ID, &edgeID);
            edgeElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_EDGE_SOURCE_ID, &edgeSourceID);
            edgeElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_EDGE_TARGET_ID, &edgeTargetID);
            edgeElement->Attribute(mitk::ConnectomicsNetworkDefinitions::XML_EDGE_WEIGHT_ID, &edgeWeight);

            mitk::ConnectomicsNetwork::VertexDescriptorType source = idToVertexMap.find( edgeSourceID )->second;
            mitk::ConnectomicsNetwork::VertexDescriptorType target = idToVertexMap.find( edgeTargetID )->second;
            outputNetwork->AddEdge( source, target, edgeSourceID, edgeTargetID, edgeWeight);
          }
        }

        outputNetwork->UpdateBounds();
        result.push_back(outputNetwork.GetPointer());
        MITK_INFO << "Network read";
      }
      catch (mitk::Exception e)
      {
        MITK_ERROR << e.GetDescription();
      }
      catch(...)
      {
        MITK_ERROR << "Unknown error occured while trying to read file.";
      }
    }

    return result;
  }


} //namespace MITK

mitk::ConnectomicsNetworkReader* mitk::ConnectomicsNetworkReader::Clone() const
{
  return new ConnectomicsNetworkReader(*this);
}
