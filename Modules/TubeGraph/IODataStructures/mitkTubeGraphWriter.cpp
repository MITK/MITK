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

#include "mitkTubeGraphWriter.h"
#include <tinyxml.h>
#include "mitkTubeGraphDefinitions.h"
#include "mitkCircularProfileTubeElement.h"
#include "mitkTubeElement.h"
#include "mitkTubeGraphDefinitions.h"
#include "mitkTubeGraphGeometry3D.h"
#include "mitkTubeGraphProperty.h"

#include "itksys/SystemTools.hxx"

#include <tinyxml.h>

mitk::TubeGraphWriter::TubeGraphWriter()
: m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::TubeGraphWriter::~TubeGraphWriter()
{
}

void mitk::TubeGraphWriter::GenerateData()
{
  MITK_INFO << "Writing tube graph";
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to TubeGraphWriter is NULL!");
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro(<< "Sorry, filename has not been set!" );
    return ;
  }

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);

  if (ext == ".tsf")
  {
    // Get geometry of the tube graph
    mitk::TubeGraphGeometry3D::Pointer geometry = dynamic_cast<mitk::TubeGraphGeometry3D*>(input->GetGeometry());
    // Get property of the tube graph
    mitk::TubeGraphProperty::Pointer inputProperty = dynamic_cast<mitk::TubeGraphProperty*>(input->GetProperty("Tube Graph.Visualization Information").GetPointer());

    // Create XML document
    TiXmlDocument documentXML;
    { // begin document
      TiXmlDeclaration* declXML = new TiXmlDeclaration( "1.0", "", "" ); // TODO what to write here? encoding? etc....
      documentXML.LinkEndChild(declXML);

      TiXmlElement* mainXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_TUBEGRAPH_FILE);
      mainXML->SetAttribute(mitk::TubeGraphDefinitions::XML_FILE_VERSION, mitk::TubeGraphDefinitions::VERSION_STRING);
      documentXML.LinkEndChild(mainXML);

      TiXmlElement* geometryXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_GEOMETRY);
      { // begin geometry
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XX, geometry->GetMatrixColumn(0)[0]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XY, geometry->GetMatrixColumn(0)[1]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XZ, geometry->GetMatrixColumn(0)[2]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YX, geometry->GetMatrixColumn(1)[0]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YY, geometry->GetMatrixColumn(1)[1]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YZ, geometry->GetMatrixColumn(1)[2]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZX, geometry->GetMatrixColumn(2)[0]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZY, geometry->GetMatrixColumn(2)[1]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZZ, geometry->GetMatrixColumn(2)[2]);

        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_X, geometry->GetOrigin()[0]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Y, geometry->GetOrigin()[1]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Z, geometry->GetOrigin()[2]);

        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_SPACING_X, geometry->GetSpacing()[0]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_SPACING_Y, geometry->GetSpacing()[1]);
        geometryXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_SPACING_Z, geometry->GetSpacing()[2]);

      } // end geometry
      mainXML->LinkEndChild(geometryXML);

      TiXmlElement* verticesXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_VERTICES);
      { // begin vertices section
        std::vector<mitk::TubeGraphVertex> vertexVector = input->GetVectorOfAllVertices();
        for(unsigned int index = 0; index < vertexVector.size(); index++)
        {
          TiXmlElement* vertexXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_VERTEX);
          vertexXML->SetAttribute(mitk::TubeGraphDefinitions::XML_VERTEX_ID, input->GetVertexDescriptor(vertexVector[index]));
          //element of each vertex
          const mitk::TubeElement* element = vertexVector[index].GetTubeElement();
          TiXmlElement* elementXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_ELEMENT );
          elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X , element->GetCoordinates().GetElement(0) );
          elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y , element->GetCoordinates().GetElement(1) );
          elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z , element->GetCoordinates().GetElement(2) );
          if (dynamic_cast<const mitk::CircularProfileTubeElement* >(element))
            elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER ,(dynamic_cast<const mitk::CircularProfileTubeElement* >(element))->GetDiameter() );
          else
            elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER, 2);

          vertexXML->LinkEndChild(elementXML);

          verticesXML->LinkEndChild(vertexXML);
        }
      } // end vertices section
      mainXML->LinkEndChild(verticesXML);

      TiXmlElement* edgesXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_EDGES);
      { // begin edges section
        std::vector<mitk::TubeGraphEdge> edgeVector = input->GetVectorOfAllEdges();
        for(unsigned int index = 0; index < edgeVector.size(); index++)
        {
          TiXmlElement* edgeXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_EDGE);
          edgeXML->SetAttribute( mitk::TubeGraphDefinitions::XML_EDGE_ID, index);
          std::pair< mitk::TubeGraphVertex, mitk::TubeGraphVertex > soureTargetPair = input->GetVerticesOfAnEdge(input->GetEdgeDescriptor(edgeVector[index]));
          edgeXML->SetAttribute( mitk::TubeGraphDefinitions::XML_EDGE_SOURCE_ID, input->GetVertexDescriptor(soureTargetPair.first));
          edgeXML->SetAttribute( mitk::TubeGraphDefinitions::XML_EDGE_TARGET_ID, input->GetVertexDescriptor(soureTargetPair.second));

          //begin elements of the edge
          std::vector<mitk::TubeElement*> elementVector = edgeVector[index].GetElementVector();
          for(unsigned int elementIndex =0; elementIndex < elementVector.size(); elementIndex++)
          {
            TiXmlElement* elementXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_ELEMENT );
            elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X , elementVector[elementIndex]->GetCoordinates().GetElement(0) );
            elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y , elementVector[elementIndex]->GetCoordinates().GetElement(1) );
            elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z , elementVector[elementIndex]->GetCoordinates().GetElement(2) );
            if (dynamic_cast<const mitk::CircularProfileTubeElement* >(elementVector[elementIndex]))
              elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER ,(dynamic_cast<const mitk::CircularProfileTubeElement* >(elementVector[elementIndex]))->GetDiameter());
            else
              elementXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER, 2);
            edgeXML->LinkEndChild(elementXML);
            //elementsXML->LinkEndChild(elementXML);
          }
          edgesXML->LinkEndChild(edgeXML);
        }
      } // end edges section
      mainXML->LinkEndChild(edgesXML);

      TiXmlElement* labelGroupsXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_LABELGROUPS);
      { // begin label group  section
        std::vector<TubeGraphProperty::LabelGroup*>   labelGroupVector = inputProperty->GetLabelGroups();
        for(unsigned int index = 0; index < labelGroupVector.size(); index++)
        {
          TiXmlElement* labelGroupXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_LABELGROUP);
          labelGroupXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME, labelGroupVector[index]->labelGroupName);
          //begin labels of the label group
          std::vector<TubeGraphProperty::LabelGroup::Label*> labelVector = labelGroupVector[index]->labels;
          for(unsigned int labelIndex = 0; labelIndex < labelVector.size(); labelIndex++)
          {
            TiXmlElement* labelXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_LABEL);
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_NAME, labelVector[labelIndex]->labelName);
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_VISIBILITY, labelVector[labelIndex]->isVisible);
            labelXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_R, labelVector[labelIndex]->labelColor[0]);
            labelXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_G, labelVector[labelIndex]->labelColor[1]);
            labelXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_B, labelVector[labelIndex]->labelColor[2]);
            labelGroupXML->LinkEndChild(labelXML);
          }
          labelGroupsXML->LinkEndChild(labelGroupXML);
        }
      } // end labe group section
      mainXML->LinkEndChild(labelGroupsXML);

      TiXmlElement* attributionsXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_ATTRIBUTIONS);
      { // begin attributions section
        std::map<mitk::TubeGraphProperty::TubeToLabelGroupType, std::string>  tubeToLabelGroup = inputProperty->GetTubesToLabels();
        for(std::map<mitk::TubeGraphProperty::TubeToLabelGroupType, std::string >::iterator it = tubeToLabelGroup.begin(); it != tubeToLabelGroup.end(); it++)
        {
          TiXmlElement* attributXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_ATTRIBUTION);
          attributXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1, it->first.first.first);
          attributXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2, it->first.first.second);
          attributXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME, it->first.second);
          attributXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_NAME, it->second);
          attributionsXML->LinkEndChild(attributXML);
        }

      } // end attributions section
      mainXML->LinkEndChild(attributionsXML);

      TiXmlElement* annotationsXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_ANNOTATIONS);
      { // begin annotations section
        std::vector<mitk::TubeGraphProperty::Annotation*>  annotations = inputProperty->GetAnnotations();
         for(unsigned int index = 0; index < annotations.size(); index++)
        {
          TiXmlElement* annotationXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_ANNOTATION);
          annotationXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ANNOTATION_NAME, annotations[index]->name);
          annotationXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ANNOTATION_DESCRIPTION, annotations[index]->description);
          annotationXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1, annotations[index]->tube.first);
          annotationXML->SetDoubleAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2, annotations[index]->tube.second);

          annotationsXML->LinkEndChild(annotationXML);
        }
      } // end annotations section
      mainXML->LinkEndChild(annotationsXML);


    } // end document
    documentXML.SaveFile(m_FileName);

    m_Success = true;

    MITK_INFO << "Tube graph written";
  }
}

void mitk::TubeGraphWriter::SetInput(mitk::TubeGraphWriter::InputType* tubeGraph)
{
  this->ProcessObject::SetNthInput(0, tubeGraph);
}

mitk::TubeGraphWriter::InputType* mitk::TubeGraphWriter::GetInput()
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  else
    return dynamic_cast<mitk::TubeGraphWriter::InputType*>(this->ProcessObject::GetInput(0));
}

std::vector<std::string> mitk::TubeGraphWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".tsf");
  return possibleFileExtensions;
}
