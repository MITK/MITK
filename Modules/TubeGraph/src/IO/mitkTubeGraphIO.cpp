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

#include "mitkTubeGraphIO.h"

#include "mitkTubeGraphDefinitions.h"
#include "mitkCircularProfileTubeElement.h"
#include "mitkTubeGraphProperty.h"

#include <mitkIOMimeTypes.h>

#include <tinyxml.h>

#include <vtkMatrix4x4.h>

#include <itksys/SystemTools.hxx>

namespace mitk {
  TubeGraphIO::TubeGraphIO(const TubeGraphIO& other)
    : AbstractFileIO(other)
  {
  }

  TubeGraphIO::TubeGraphIO()
    : AbstractFileIO(mitk::TubeGraph::GetStaticNameOfClass(), mitk::TubeGraphIO::TUBEGRAPH_MIMETYPE(), "Tube Graph Structure File")
  {
    this->RegisterService();
  }

  std::vector<BaseData::Pointer> TubeGraphIO::Read()
  {
    std::locale::global(std::locale("C"));

    std::vector< itk::SmartPointer<mitk::BaseData> > result;

    InputStream stream(this);

    TiXmlDocument doc;
    stream >> doc;
    if (!doc.Error())
    {
      TubeGraph::Pointer newTubeGraph = TubeGraph::New();

      TiXmlHandle hDoc(&doc);
      TiXmlElement* pElem;
      TiXmlHandle hRoot(0);

      pElem = hDoc.FirstChildElement().Element();

      // save this for later
      hRoot = TiXmlHandle(pElem);

      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_GEOMETRY).Element();

      // read geometry
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
      geometry->Initialize();

      // read origin
      mitk::Point3D origin;
      double temp = 0;
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_ORIGIN_X, &temp);
      origin[0] = temp;
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Y, &temp);
      origin[1] = temp;
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Z, &temp);
      origin[2] = temp;
      geometry->SetOrigin(origin);

      // read spacing
      Vector3D spacing;
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_SPACING_X, &temp);
      spacing.SetElement(0, temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_SPACING_Y, &temp);
      spacing.SetElement(1, temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_SPACING_Z, &temp);
      spacing.SetElement(2, temp);
      geometry->SetSpacing(spacing);

      // read transform
      vtkMatrix4x4* m = vtkMatrix4x4::New();
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_XX, &temp);
      m->SetElement(0,0,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_XY, &temp);
      m->SetElement(1,0,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_XZ, &temp);
      m->SetElement(2,0,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_YX, &temp);
      m->SetElement(0,1,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_YY, &temp);
      m->SetElement(1,1,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_YZ, &temp);
      m->SetElement(2,1,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZX, &temp);
      m->SetElement(0,2,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZY, &temp);
      m->SetElement(1,2,temp);
      pElem->Attribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZZ, &temp);
      m->SetElement(2,2,temp);

      m->SetElement(0,3,origin[0]);
      m->SetElement(1,3,origin[1]);
      m->SetElement(2,3,origin[2]);
      m->SetElement(3,3,1);
      geometry->SetIndexToWorldTransformByVtkMatrix(m);

      geometry->SetImageGeometry(false);

      // read tube graph

      // read vertices
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_VERTICES).Element();
      if (pElem != 0)
      {
        // walk through the vertices
        TiXmlElement* vertexElement = pElem->FirstChildElement();

        for( vertexElement; vertexElement; vertexElement = vertexElement->NextSiblingElement())
        {
          int vertexID(0);
          mitk::Point3D coordinate;
          coordinate.Fill( 0.0 );
          double diameter(0);

          vertexElement->Attribute(mitk::TubeGraphDefinitions::XML_VERTEX_ID, &vertexID);

          TiXmlElement* tubeElement = vertexElement->FirstChildElement();

          tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X, &temp);
          coordinate[0] = temp;
          tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y, &temp);
          coordinate[1] = temp;
          tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z, &temp);
          coordinate[2] = temp;
          tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER, &diameter);

          mitk::TubeGraphVertex vertexData;
          mitk::CircularProfileTubeElement* newElement = new mitk::CircularProfileTubeElement(coordinate, diameter);
          vertexData.SetTubeElement(newElement);

          mitk::TubeGraph::VertexDescriptorType newVertex = newTubeGraph->AddVertex(vertexData);
          if ( newVertex != vertexID )
          {
            MITK_ERROR << "Aborting tube graph creation, different vertex ids.";
            return result;;
          }
        }
      }

      // read edges
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_EDGES).Element();
      if (pElem != 0)
      {
        // walk through the edges
        TiXmlElement* edgeElement = pElem->FirstChildElement();

        for( edgeElement; edgeElement; edgeElement=edgeElement->NextSiblingElement())
        {
          int edgeID(0), edgeSourceID(0), edgeTargetID(0);
          mitk::Point3D coordinate;
          double diameter(0);

          edgeElement->Attribute(mitk::TubeGraphDefinitions::XML_EDGE_ID, &edgeID);
          edgeElement->Attribute(mitk::TubeGraphDefinitions::XML_EDGE_SOURCE_ID, &edgeSourceID);
          edgeElement->Attribute(mitk::TubeGraphDefinitions::XML_EDGE_TARGET_ID, &edgeTargetID);

          mitk::TubeGraphEdge edgeData;

          TiXmlElement* tubeElement = edgeElement->FirstChildElement(mitk::TubeGraphDefinitions::XML_ELEMENT);
          for( tubeElement; tubeElement; tubeElement = tubeElement->NextSiblingElement())
          {
            tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X, &temp);
            coordinate[0] = temp;
            tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y, &temp);
            coordinate[1] = temp;
            tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z, &temp);
            coordinate[2] = temp;
            tubeElement->Attribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER, &diameter);

            mitk::CircularProfileTubeElement* newElement = new mitk::CircularProfileTubeElement(coordinate, diameter);

            edgeData.AddTubeElement(newElement);
          }
          try
          {
            newTubeGraph->AddEdge( edgeSourceID, edgeTargetID, edgeData);
          }
          catch (const std::runtime_error& error)
          {
            MITK_ERROR << error.what();
            return result;
          }

        }
      }

      //Compute bounding box
      BoundingBox::Pointer bb = this->ComputeBoundingBox(newTubeGraph);
      geometry->SetBounds(bb->GetBounds());

      MITK_INFO << "Tube Graph read";
      MITK_INFO<<"Edge numb:"<< newTubeGraph->GetNumberOfEdges()<< " Vertices: "<< newTubeGraph->GetNumberOfVertices();

      MITK_INFO << "Reading tube graph property";
      mitk::TubeGraphProperty::Pointer newProperty = mitk::TubeGraphProperty::New();

      // read label groups
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_LABELGROUPS).Element();
      if (pElem !=0)
      {
        // walk through the label groups
        TiXmlElement* labelGroupElement = pElem->FirstChildElement();

        for( labelGroupElement; labelGroupElement; labelGroupElement = labelGroupElement->NextSiblingElement())
        {
          mitk::TubeGraphProperty::LabelGroup* newLabelGroup = new mitk::TubeGraphProperty::LabelGroup();
          const char * labelGroupName;


          labelGroupName = labelGroupElement->Attribute((char*)mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME.c_str());
          if (labelGroupName)
            newLabelGroup->labelGroupName = labelGroupName;


          //labelGroupElement->Attribute(mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME, &labelGroupName);
          //newLabelGroup.labeGroupName = labelGroupName;

          TiXmlElement* labelElement = labelGroupElement->FirstChildElement(mitk::TubeGraphDefinitions::XML_LABEL);
          for( labelElement; labelElement; labelElement = labelElement->NextSiblingElement())
          {

            mitk::TubeGraphProperty::LabelGroup::Label* newLabel = new mitk::TubeGraphProperty::LabelGroup::Label();
            const char * labelName;
            bool isVisible = true;
            Color color;

            labelName = labelElement->Attribute((char*)mitk::TubeGraphDefinitions::XML_LABEL_NAME.c_str());
            if (labelName)
              newLabel->labelName = labelName;

            labelElement->Attribute(mitk::TubeGraphDefinitions::XML_LABEL_VISIBILITY, &temp);
            if(temp == 0)
              isVisible = false;
            else
              isVisible = true;
            labelElement->Attribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_R, &temp);
            color[0] = temp;
            labelElement->Attribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_G, &temp);
            color[1] = temp;
            labelElement->Attribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_B, &temp);
            color[2] = temp;

            newLabel->isVisible = isVisible;
            newLabel->labelColor = color;
            newLabelGroup->labels.push_back(newLabel);
          }
          newProperty->AddLabelGroup(newLabelGroup, newProperty->GetLabelGroups().size());
        }

      }
      // read attributations
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_ATTRIBUTIONS).Element();
      if (pElem !=0)
      {

        TiXmlElement* tubeToLabelElement = pElem->FirstChildElement();
        std::map<TubeGraphProperty::TubeToLabelGroupType, std::string > tubeToLabelsMap;
        for( tubeToLabelElement; tubeToLabelElement; tubeToLabelElement = tubeToLabelElement->NextSiblingElement())
        {
          TubeGraph::TubeDescriptorType tube;
          mitk::TubeGraphProperty::LabelGroup* labelGroup = new mitk::TubeGraphProperty::LabelGroup();
          mitk::TubeGraphProperty::LabelGroup::Label* label = new mitk::TubeGraphProperty::LabelGroup::Label();


          tubeToLabelElement->Attribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1, &temp);
          tube.first = temp;
          tubeToLabelElement->Attribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2, &temp);
          tube.second = temp;
          const char * labelGroupName = tubeToLabelElement->Attribute((char*)mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME.c_str());
          if (labelGroupName)
            labelGroup = newProperty->GetLabelGroupByName(labelGroupName);

          const char * labelName = tubeToLabelElement->Attribute((char*)mitk::TubeGraphDefinitions::XML_LABEL_NAME.c_str());
          if (labelName)
            label = newProperty->GetLabelByName(labelGroup, labelName);

          if (tube != TubeGraph::ErrorId && labelGroup != NULL && label != NULL)
          {
            TubeGraphProperty::TubeToLabelGroupType tubeToLabelGroup (tube, labelGroupName);
            tubeToLabelsMap.insert (std::pair<TubeGraphProperty::TubeToLabelGroupType, std::string>(tubeToLabelGroup, labelName));
          }
        }
        if(tubeToLabelsMap.size() > 0)
          newProperty->SetTubesToLabels(tubeToLabelsMap);
      }
      // read annotations
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_ANNOTATIONS).Element();
      if (pElem !=0)
      {
        TiXmlElement* annotationElement = pElem->FirstChildElement();
        for( annotationElement; annotationElement; annotationElement = annotationElement->NextSiblingElement())
        {
          mitk::TubeGraphProperty::Annotation* annotation = new mitk::TubeGraphProperty::Annotation();
          std::string annotationName;
          std::string annotationDescription;
          TubeGraph::TubeDescriptorType tube;

          annotationName = annotationElement->Attribute((char*)mitk::TubeGraphDefinitions::XML_ANNOTATION_NAME.c_str());
          annotation->name = annotationName;

          annotationDescription = annotationElement->Attribute((char*)mitk::TubeGraphDefinitions::XML_ANNOTATION_DESCRIPTION.c_str());
          annotation->description = annotationDescription;

          annotationElement->Attribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1, &temp);
          tube.first = temp;
          annotationElement->Attribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2, &temp);
          tube.second = temp;

          if (tube != TubeGraph::ErrorId)
          {
            annotation->tube = tube;
            newProperty->AddAnnotation(annotation);
          }
        }
      }

      MITK_INFO << "Tube Graph Property read";

      newTubeGraph->SetGeometry(geometry);
      newTubeGraph->SetProperty("Tube Graph.Visualization Information", newProperty);
      result.push_back( newTubeGraph.GetPointer() );
    }

    else
    {
      mitkThrow() << "Parsing error at line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
    }
    return result;
  }

  AbstractFileIO::ConfidenceLevel TubeGraphIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;
    return Supported;
  }

  void TubeGraphIO::Write()
  {
    OutputStream out(this);

    if ( !out.good() )
    {
      mitkThrow() << "Stream not good.";
    }

    std::locale previousLocale(out.getloc());
    std::locale I("C");
    out.imbue(I);

    const mitk::TubeGraph* tubeGraph = dynamic_cast<const mitk::TubeGraph*>(this->GetInput());
    // Get geometry of the tube graph
    mitk::Geometry3D::Pointer geometry = dynamic_cast<mitk::Geometry3D*>(tubeGraph->GetGeometry());
    // Get property of the tube graph
    mitk::TubeGraphProperty::Pointer tubeGraphProperty = dynamic_cast<mitk::TubeGraphProperty*>(tubeGraph->GetProperty("Tube Graph.Visualization Information").GetPointer());
    // Create XML document
    TiXmlDocument documentXML;
    {//Begin document
      TiXmlDeclaration* declXML = new TiXmlDeclaration( "1.0", "", "" );
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
        std::vector<mitk::TubeGraphVertex> vertexVector = tubeGraph->GetVectorOfAllVertices();
        for(unsigned int index = 0; index < vertexVector.size(); index++)
        {
          TiXmlElement* vertexXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_VERTEX);
          vertexXML->SetAttribute(mitk::TubeGraphDefinitions::XML_VERTEX_ID, tubeGraph->GetVertexDescriptor(vertexVector[index]));
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
        std::vector<mitk::TubeGraphEdge> edgeVector = tubeGraph->GetVectorOfAllEdges();
        for(unsigned int index = 0; index < edgeVector.size(); index++)
        {
          TiXmlElement* edgeXML = new TiXmlElement(mitk::TubeGraphDefinitions::XML_EDGE);
          edgeXML->SetAttribute( mitk::TubeGraphDefinitions::XML_EDGE_ID, index);
          std::pair< mitk::TubeGraphVertex, mitk::TubeGraphVertex > soureTargetPair = tubeGraph->GetVerticesOfAnEdge(tubeGraph->GetEdgeDescriptor(edgeVector[index]));
          edgeXML->SetAttribute( mitk::TubeGraphDefinitions::XML_EDGE_SOURCE_ID, tubeGraph->GetVertexDescriptor(soureTargetPair.first));
          edgeXML->SetAttribute( mitk::TubeGraphDefinitions::XML_EDGE_TARGET_ID, tubeGraph->GetVertexDescriptor(soureTargetPair.second));

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
        std::vector<TubeGraphProperty::LabelGroup*>   labelGroupVector = tubeGraphProperty->GetLabelGroups();
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
        std::map<mitk::TubeGraphProperty::TubeToLabelGroupType, std::string>  tubeToLabelGroup = tubeGraphProperty->GetTubesToLabels();
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
        std::vector<mitk::TubeGraphProperty::Annotation*>  annotations = tubeGraphProperty->GetAnnotations();
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

    TiXmlPrinter printer;
    printer.SetStreamPrinting();
    documentXML.Accept( &printer );
    out<< printer.Str() ;
  }


  AbstractFileIO::ConfidenceLevel TubeGraphIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported) return Unsupported;
    return Supported;
  }

  TubeGraphIO* TubeGraphIO::IOClone() const
  {
    return new TubeGraphIO(*this);
  }
}

const mitk::BoundingBox::Pointer mitk::TubeGraphIO::ComputeBoundingBox(mitk::TubeGraph::Pointer graph ) const
{
  BoundingBox::Pointer boundingBox = BoundingBox::New();
  BoundingBox::PointIdentifier pointid = 0;
  BoundingBox::PointsContainer::Pointer pointscontainer = BoundingBox::PointsContainer::New();

  ScalarType nullpoint[] = { 0,0,0 };
  BoundingBox::PointType p(nullpoint);

  //traverse the tree and add each point to the pointscontainer

  mitk::Point3D pos;

  std::vector<mitk::TubeGraphVertex> vertexVector = graph->GetVectorOfAllVertices();
  for(std::vector<mitk::TubeGraphVertex>::iterator vertex = vertexVector.begin(); vertex != vertexVector.end(); ++vertex)
  {
    pos =  vertex->GetTubeElement()->GetCoordinates();
    p[0] = pos[0];
    p[1] = pos[1];
    p[2] = pos[2];
    pointscontainer->InsertElement(pointid++, p);
  }

  std::vector<mitk::TubeGraphEdge> edgeVector = graph->GetVectorOfAllEdges();

  for(std::vector<mitk::TubeGraphEdge>::iterator edge = edgeVector.begin(); edge != edgeVector.end(); ++edge)
  {
    std::vector<mitk::TubeElement*> allElements = edge->GetElementVector();
    for(unsigned int index =0; index < edge->GetNumberOfElements(); index++)
    {
      pos = allElements[index]->GetCoordinates();
      p[0] = pos[0];
      p[1] = pos[1];
      p[2] = pos[2];
      pointscontainer->InsertElement(pointid++, p);
    }
  }

  boundingBox->SetPoints(pointscontainer);
  boundingBox->ComputeBoundingBox();

  return boundingBox;
}
