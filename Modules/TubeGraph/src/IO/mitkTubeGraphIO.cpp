/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphIO.h"

#include "mitkCircularProfileTubeElement.h"
#include "mitkTubeGraphDefinitions.h"
#include "mitkTubeGraphProperty.h"

#include <mitkIOMimeTypes.h>

#include <tinyxml2.h>

#include <vtkMatrix4x4.h>

#include <itksys/SystemTools.hxx>

namespace mitk
{
  TubeGraphIO::TubeGraphIO(const TubeGraphIO &other) : AbstractFileIO(other) {}
  TubeGraphIO::TubeGraphIO()
    : AbstractFileIO(
        mitk::TubeGraph::GetStaticNameOfClass(), mitk::TubeGraphIO::TUBEGRAPH_MIMETYPE(), "Tube Graph Structure File")
  {
    this->RegisterService();
  }

  std::vector<BaseData::Pointer> TubeGraphIO::DoRead()
  {
    std::locale::global(std::locale("C"));

    std::vector<itk::SmartPointer<mitk::BaseData>> result;

    InputStream stream(this);
    std::string string(std::istreambuf_iterator<char>(stream), {});
    tinyxml2::XMLDocument doc;

    if (tinyxml2::XML_SUCCESS == doc.Parse(string.c_str()))
    {
      TubeGraph::Pointer newTubeGraph = TubeGraph::New();

      tinyxml2::XMLHandle hDoc(&doc);
      tinyxml2::XMLHandle hRoot = hDoc.FirstChildElement();
      tinyxml2::XMLElement *pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_GEOMETRY.c_str()).ToElement();

      // read geometry
      mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
      geometry->Initialize();

      // read origin
      mitk::Point3D origin;
      double temp = 0;
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_X.c_str(), &temp);
      origin[0] = temp;
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Y.c_str(), &temp);
      origin[1] = temp;
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Z.c_str(), &temp);
      origin[2] = temp;
      geometry->SetOrigin(origin);

      // read spacing
      Vector3D spacing;
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_SPACING_X.c_str(), &temp);
      spacing.SetElement(0, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_SPACING_Y.c_str(), &temp);
      spacing.SetElement(1, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_SPACING_Z.c_str(), &temp);
      spacing.SetElement(2, temp);
      geometry->SetSpacing(spacing);

      // read transform
      vtkMatrix4x4 *m = vtkMatrix4x4::New();
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XX.c_str(), &temp);
      m->SetElement(0, 0, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XY.c_str(), &temp);
      m->SetElement(1, 0, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XZ.c_str(), &temp);
      m->SetElement(2, 0, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YX.c_str(), &temp);
      m->SetElement(0, 1, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YY.c_str(), &temp);
      m->SetElement(1, 1, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YZ.c_str(), &temp);
      m->SetElement(2, 1, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZX.c_str(), &temp);
      m->SetElement(0, 2, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZY.c_str(), &temp);
      m->SetElement(1, 2, temp);
      pElem->QueryAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZZ.c_str(), &temp);
      m->SetElement(2, 2, temp);

      m->SetElement(0, 3, origin[0]);
      m->SetElement(1, 3, origin[1]);
      m->SetElement(2, 3, origin[2]);
      m->SetElement(3, 3, 1);
      geometry->SetIndexToWorldTransformByVtkMatrix(m);

      geometry->SetImageGeometry(false);

      // read tube graph

      // read vertices
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_VERTICES.c_str()).ToElement();
      if (pElem != nullptr)
      {
        // walk through the vertices
        for (auto *vertexElement = pElem->FirstChildElement(); vertexElement != nullptr; vertexElement = vertexElement->NextSiblingElement())
        {
          int vertexID(0);
          mitk::Point3D coordinate;
          coordinate.Fill(0.0);
          double diameter(0);

          vertexElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_VERTEX_ID.c_str(), &vertexID);

          auto *tubeElement = vertexElement->FirstChildElement();

          tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X.c_str(), &temp);
          coordinate[0] = temp;
          tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y.c_str(), &temp);
          coordinate[1] = temp;
          tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z.c_str(), &temp);
          coordinate[2] = temp;
          tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER.c_str(), &diameter);

          mitk::TubeGraphVertex vertexData;
          auto *newElement = new mitk::CircularProfileTubeElement(coordinate, diameter);
          vertexData.SetTubeElement(newElement);

          mitk::TubeGraph::VertexDescriptorType newVertex = newTubeGraph->AddVertex(vertexData);
          if (static_cast<int>(newVertex) != vertexID)
          {
            MITK_ERROR << "Aborting tube graph creation, different vertex ids.";
            return result;
            ;
          }
        }
      }

      // read edges
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_EDGES.c_str()).ToElement();
      if (pElem != nullptr)
      {
        // walk through the edges
        auto edgeElement = pElem->FirstChildElement();
        for ( ; edgeElement != nullptr; edgeElement = edgeElement->NextSiblingElement())
        {
          int edgeID(0), edgeSourceID(0), edgeTargetID(0);
          mitk::Point3D coordinate;
          double diameter(0);

          edgeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_EDGE_ID.c_str(), &edgeID);
          edgeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_EDGE_SOURCE_ID.c_str(), &edgeSourceID);
          edgeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_EDGE_TARGET_ID.c_str(), &edgeTargetID);

          mitk::TubeGraphEdge edgeData;

          for (auto *tubeElement = edgeElement->FirstChildElement(mitk::TubeGraphDefinitions::XML_ELEMENT.c_str()); tubeElement != nullptr; tubeElement = tubeElement->NextSiblingElement())
          {
            tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X.c_str(), &temp);
            coordinate[0] = temp;
            tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y.c_str(), &temp);
            coordinate[1] = temp;
            tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z.c_str(), &temp);
            coordinate[2] = temp;
            tubeElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER.c_str(), &diameter);

            auto *newElement = new mitk::CircularProfileTubeElement(coordinate, diameter);

            edgeData.AddTubeElement(newElement);
          }
          try
          {
            newTubeGraph->AddEdge(edgeSourceID, edgeTargetID, edgeData);
          }
          catch (const std::runtime_error &error)
          {
            MITK_ERROR << error.what();
            return result;
          }
        }
      }

      // Compute bounding box
      BoundingBox::Pointer bb = this->ComputeBoundingBox(newTubeGraph);
      geometry->SetBounds(bb->GetBounds());

      MITK_INFO << "Tube Graph read";
      MITK_INFO << "Edge numb:" << newTubeGraph->GetNumberOfEdges()
                << " Vertices: " << newTubeGraph->GetNumberOfVertices();

      MITK_INFO << "Reading tube graph property";
      mitk::TubeGraphProperty::Pointer newProperty = mitk::TubeGraphProperty::New();

      // read label groups
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_LABELGROUPS.c_str()).ToElement();
      if (pElem != nullptr)
      {
        // walk through the label groups
        for (auto *labelGroupElement = pElem->FirstChildElement(); labelGroupElement != nullptr; labelGroupElement = labelGroupElement->NextSiblingElement())
        {
          auto *newLabelGroup = new mitk::TubeGraphProperty::LabelGroup();
          const char *labelGroupName;

          labelGroupName =
            labelGroupElement->Attribute((char *)mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME.c_str());
          if (labelGroupName)
            newLabelGroup->labelGroupName = labelGroupName;

          for (auto *labelElement = labelGroupElement->FirstChildElement(mitk::TubeGraphDefinitions::XML_LABEL.c_str()); labelElement != nullptr; labelElement = labelElement->NextSiblingElement())
          {
            auto *newLabel = new mitk::TubeGraphProperty::LabelGroup::Label();
            const char *labelName;
            bool isVisible = true;
            Color color;

            labelName = labelElement->Attribute((char *)mitk::TubeGraphDefinitions::XML_LABEL_NAME.c_str());
            if (labelName)
              newLabel->labelName = labelName;

            labelElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_LABEL_VISIBILITY.c_str(), &temp);
            if (temp == 0)
              isVisible = false;
            else
              isVisible = true;
            labelElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_R.c_str(), &temp);
            color[0] = temp;
            labelElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_G.c_str(), &temp);
            color[1] = temp;
            labelElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_B.c_str(), &temp);
            color[2] = temp;

            newLabel->isVisible = isVisible;
            newLabel->labelColor = color;
            newLabelGroup->labels.push_back(newLabel);
          }
          newProperty->AddLabelGroup(newLabelGroup, newProperty->GetLabelGroups().size());
        }
      }
      // read attributations
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_ATTRIBUTIONS.c_str()).ToElement();
      if (pElem != nullptr)
      {
        std::map<TubeGraphProperty::TubeToLabelGroupType, std::string> tubeToLabelsMap;
        for (auto *tubeToLabelElement = pElem->FirstChildElement(); tubeToLabelElement != nullptr; tubeToLabelElement = tubeToLabelElement->NextSiblingElement())
        {
          TubeGraph::TubeDescriptorType tube;
          auto *labelGroup = new mitk::TubeGraphProperty::LabelGroup();
          auto *label = new mitk::TubeGraphProperty::LabelGroup::Label();

          tubeToLabelElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1.c_str(), &temp);
          tube.first = temp;
          tubeToLabelElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2.c_str(), &temp);
          tube.second = temp;
          const char *labelGroupName =
            tubeToLabelElement->Attribute(mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME.c_str());
          if (labelGroupName)
            labelGroup = newProperty->GetLabelGroupByName(labelGroupName);

          const char *labelName =
            tubeToLabelElement->Attribute(mitk::TubeGraphDefinitions::XML_LABEL_NAME.c_str());
          if (labelName)
            label = newProperty->GetLabelByName(labelGroup, labelName);

          if (tube != TubeGraph::ErrorId && labelGroup != nullptr && label != nullptr)
          {
            TubeGraphProperty::TubeToLabelGroupType tubeToLabelGroup(tube, labelGroupName);
            tubeToLabelsMap.insert(
              std::pair<TubeGraphProperty::TubeToLabelGroupType, std::string>(tubeToLabelGroup, labelName));
          }
        }
        if (tubeToLabelsMap.size() > 0)
          newProperty->SetTubesToLabels(tubeToLabelsMap);
      }
      // read annotations
      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_ANNOTATIONS.c_str()).ToElement();
      if (pElem != nullptr)
      {
        for (auto *annotationElement = pElem->FirstChildElement(); annotationElement != nullptr; annotationElement = annotationElement->NextSiblingElement())
        {
          auto *annotation = new mitk::TubeGraphProperty::Annotation();
          TubeGraph::TubeDescriptorType tube;

          const char *annotationName =
            annotationElement->Attribute(mitk::TubeGraphDefinitions::XML_ANNOTATION_NAME.c_str());
          annotation->name = nullptr != annotationName ? annotationName : "";

          const char *annotationDescription =
            annotationElement->Attribute(mitk::TubeGraphDefinitions::XML_ANNOTATION_DESCRIPTION.c_str());
          annotation->description = nullptr != annotationDescription ? annotationDescription : "";

          annotationElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1.c_str(), &temp);
          tube.first = temp;
          annotationElement->QueryAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2.c_str(), &temp);
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
      result.push_back(newTubeGraph.GetPointer());
    }
    else
    {
      mitkThrow() << "Parsing error at line " << doc.ErrorLineNum() << ": " << doc.ErrorStr();
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

    if (!out.good())
    {
      mitkThrow() << "Stream not good.";
    }

    std::locale previousLocale(out.getloc());
    std::locale I("C");
    out.imbue(I);

    const auto *tubeGraph = dynamic_cast<const mitk::TubeGraph *>(this->GetInput());
    // Get geometry of the tube graph
    mitk::Geometry3D::Pointer geometry = dynamic_cast<mitk::Geometry3D *>(tubeGraph->GetGeometry());
    // Get property of the tube graph
    mitk::TubeGraphProperty::Pointer tubeGraphProperty = dynamic_cast<mitk::TubeGraphProperty *>(
      tubeGraph->GetProperty("Tube Graph.Visualization Information").GetPointer());
    // Create XML document
    tinyxml2::XMLDocument documentXML;
    { // Begin document
      documentXML.InsertEndChild(documentXML.NewDeclaration());

      auto *mainXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_TUBEGRAPH_FILE.c_str());
      mainXML->SetAttribute(mitk::TubeGraphDefinitions::XML_FILE_VERSION.c_str(), mitk::TubeGraphDefinitions::VERSION_STRING.c_str());
      documentXML.InsertEndChild(mainXML);

      auto *geometryXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_GEOMETRY.c_str());
      { // begin geometry
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XX.c_str(), geometry->GetMatrixColumn(0)[0]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XY.c_str(), geometry->GetMatrixColumn(0)[1]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_XZ.c_str(), geometry->GetMatrixColumn(0)[2]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YX.c_str(), geometry->GetMatrixColumn(1)[0]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YY.c_str(), geometry->GetMatrixColumn(1)[1]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_YZ.c_str(), geometry->GetMatrixColumn(1)[2]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZX.c_str(), geometry->GetMatrixColumn(2)[0]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZY.c_str(), geometry->GetMatrixColumn(2)[1]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_MATRIX_ZZ.c_str(), geometry->GetMatrixColumn(2)[2]);

        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_X.c_str(), geometry->GetOrigin()[0]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Y.c_str(), geometry->GetOrigin()[1]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ORIGIN_Z.c_str(), geometry->GetOrigin()[2]);

        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_SPACING_X.c_str(), geometry->GetSpacing()[0]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_SPACING_Y.c_str(), geometry->GetSpacing()[1]);
        geometryXML->SetAttribute(mitk::TubeGraphDefinitions::XML_SPACING_Z.c_str(), geometry->GetSpacing()[2]);

      } // end geometry
      mainXML->InsertEndChild(geometryXML);

      auto *verticesXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_VERTICES.c_str());
      { // begin vertices section
        std::vector<mitk::TubeGraphVertex> vertexVector = tubeGraph->GetVectorOfAllVertices();
        for (unsigned int index = 0; index < vertexVector.size(); index++)
        {
          auto *vertexXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_VERTEX.c_str());
          vertexXML->SetAttribute(mitk::TubeGraphDefinitions::XML_VERTEX_ID.c_str(),
                                  static_cast<int>(tubeGraph->GetVertexDescriptor(vertexVector[index])));
          // element of each vertex
          const mitk::TubeElement *element = vertexVector[index].GetTubeElement();
          auto *elementXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_ELEMENT.c_str());
          elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X.c_str(),
                                         element->GetCoordinates().GetElement(0));
          elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y.c_str(),
                                         element->GetCoordinates().GetElement(1));
          elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z.c_str(),
                                         element->GetCoordinates().GetElement(2));
          if (dynamic_cast<const mitk::CircularProfileTubeElement *>(element))
            elementXML->SetAttribute(
              mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER.c_str(),
              (dynamic_cast<const mitk::CircularProfileTubeElement *>(element))->GetDiameter());
          else
            elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER.c_str(), 2);

          vertexXML->InsertEndChild(elementXML);

          verticesXML->InsertEndChild(vertexXML);
        }
      } // end vertices section
      mainXML->InsertEndChild(verticesXML);

      auto *edgesXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_EDGES.c_str());
      { // begin edges section
        std::vector<mitk::TubeGraphEdge> edgeVector = tubeGraph->GetVectorOfAllEdges();
        for (unsigned int index = 0; index < edgeVector.size(); index++)
        {
          auto *edgeXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_EDGE.c_str());
          edgeXML->SetAttribute(mitk::TubeGraphDefinitions::XML_EDGE_ID.c_str(), index);
          std::pair<mitk::TubeGraphVertex, mitk::TubeGraphVertex> soureTargetPair =
            tubeGraph->GetVerticesOfAnEdge(tubeGraph->GetEdgeDescriptor(edgeVector[index]));
          edgeXML->SetAttribute(mitk::TubeGraphDefinitions::XML_EDGE_SOURCE_ID.c_str(),
            static_cast<int>(tubeGraph->GetVertexDescriptor(soureTargetPair.first)));
          edgeXML->SetAttribute(mitk::TubeGraphDefinitions::XML_EDGE_TARGET_ID.c_str(),
            static_cast<int>(tubeGraph->GetVertexDescriptor(soureTargetPair.second)));

          // begin elements of the edge
          std::vector<mitk::TubeElement *> elementVector = edgeVector[index].GetElementVector();
          for (unsigned int elementIndex = 0; elementIndex < elementVector.size(); elementIndex++)
          {
            auto *elementXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_ELEMENT.c_str());
            elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_X.c_str(),
                                           elementVector[elementIndex]->GetCoordinates().GetElement(0));
            elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Y.c_str(),
                                           elementVector[elementIndex]->GetCoordinates().GetElement(1));
            elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_Z.c_str(),
                                           elementVector[elementIndex]->GetCoordinates().GetElement(2));
            if (dynamic_cast<const mitk::CircularProfileTubeElement *>(elementVector[elementIndex]))
              elementXML->SetAttribute(
                mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER.c_str(),
                (dynamic_cast<const mitk::CircularProfileTubeElement *>(elementVector[elementIndex]))->GetDiameter());
            else
              elementXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ELEMENT_DIAMETER.c_str(), 2);
            edgeXML->InsertEndChild(elementXML);
            // elementsXML->InsertEndChild(elementXML);
          }
          edgesXML->InsertEndChild(edgeXML);
        }
      } // end edges section
      mainXML->InsertEndChild(edgesXML);

      auto *labelGroupsXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_LABELGROUPS.c_str());
      { // begin label group  section
        std::vector<TubeGraphProperty::LabelGroup *> labelGroupVector = tubeGraphProperty->GetLabelGroups();
        for (unsigned int index = 0; index < labelGroupVector.size(); index++)
        {
          auto *labelGroupXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_LABELGROUP.c_str());
          labelGroupXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME.c_str(),
                                      labelGroupVector[index]->labelGroupName.c_str());
          // begin labels of the label group
          std::vector<TubeGraphProperty::LabelGroup::Label *> labelVector = labelGroupVector[index]->labels;
          for (unsigned int labelIndex = 0; labelIndex < labelVector.size(); labelIndex++)
          {
            auto *labelXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_LABEL.c_str());
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_NAME.c_str(), labelVector[labelIndex]->labelName.c_str());
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_VISIBILITY.c_str(),
                                   labelVector[labelIndex]->isVisible ? 1 : 0);
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_R.c_str(),
                                         labelVector[labelIndex]->labelColor[0]);
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_G.c_str(),
                                         labelVector[labelIndex]->labelColor[1]);
            labelXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_COLOR_B.c_str(),
                                         labelVector[labelIndex]->labelColor[2]);
            labelGroupXML->InsertEndChild(labelXML);
          }
          labelGroupsXML->InsertEndChild(labelGroupXML);
        }
      } // end labe group section
      mainXML->InsertEndChild(labelGroupsXML);

      auto *attributionsXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_ATTRIBUTIONS.c_str());
      { // begin attributions section
        std::map<mitk::TubeGraphProperty::TubeToLabelGroupType, std::string> tubeToLabelGroup =
          tubeGraphProperty->GetTubesToLabels();
        for (auto it =
               tubeToLabelGroup.begin();
             it != tubeToLabelGroup.end();
             it++)
        {
          auto *attributXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_ATTRIBUTION.c_str());
          attributXML->SetAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1.c_str(), static_cast<int>(it->first.first.first));
          attributXML->SetAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2.c_str(), static_cast<int>(it->first.first.second));
          attributXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABELGROUP_NAME.c_str(), it->first.second.c_str());
          attributXML->SetAttribute(mitk::TubeGraphDefinitions::XML_LABEL_NAME.c_str(), it->second.c_str());
          attributionsXML->InsertEndChild(attributXML);
        }

      } // end attributions section
      mainXML->InsertEndChild(attributionsXML);

      auto *annotationsXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_ANNOTATIONS.c_str());
      { // begin annotations section
        std::vector<mitk::TubeGraphProperty::Annotation *> annotations = tubeGraphProperty->GetAnnotations();
        for (unsigned int index = 0; index < annotations.size(); index++)
        {
          auto *annotationXML = documentXML.NewElement(mitk::TubeGraphDefinitions::XML_ANNOTATION.c_str());
          annotationXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ANNOTATION_NAME.c_str(), annotations[index]->name.c_str());
          annotationXML->SetAttribute(mitk::TubeGraphDefinitions::XML_ANNOTATION_DESCRIPTION.c_str(), annotations[index]->description.c_str());
          annotationXML->SetAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_1.c_str(), static_cast<int>(annotations[index]->tube.first));
          annotationXML->SetAttribute(mitk::TubeGraphDefinitions::XML_TUBE_ID_2.c_str(), static_cast<int>(annotations[index]->tube.second));

          annotationsXML->InsertEndChild(annotationXML);
        }
      } // end annotations section
      mainXML->InsertEndChild(annotationsXML);
    } // end document

    tinyxml2::XMLPrinter printer;
    documentXML.Print(&printer);

    out << printer.CStr();
  }

  AbstractFileIO::ConfidenceLevel TubeGraphIO::GetWriterConfidenceLevel() const
  {
    if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported)
      return Unsupported;
    return Supported;
  }

  TubeGraphIO *TubeGraphIO::IOClone() const { return new TubeGraphIO(*this); }
}

const mitk::BoundingBox::Pointer mitk::TubeGraphIO::ComputeBoundingBox(mitk::TubeGraph::Pointer graph) const
{
  BoundingBox::Pointer boundingBox = BoundingBox::New();
  BoundingBox::PointIdentifier pointid = 0;
  BoundingBox::PointsContainer::Pointer pointscontainer = BoundingBox::PointsContainer::New();

  ScalarType nullpoint[] = {0, 0, 0};
  BoundingBox::PointType p(nullpoint);

  // traverse the tree and add each point to the pointscontainer

  mitk::Point3D pos;

  std::vector<mitk::TubeGraphVertex> vertexVector = graph->GetVectorOfAllVertices();
  for (auto vertex = vertexVector.begin(); vertex != vertexVector.end();
       ++vertex)
  {
    pos = vertex->GetTubeElement()->GetCoordinates();
    p[0] = pos[0];
    p[1] = pos[1];
    p[2] = pos[2];
    pointscontainer->InsertElement(pointid++, p);
  }

  std::vector<mitk::TubeGraphEdge> edgeVector = graph->GetVectorOfAllEdges();

  for (auto edge = edgeVector.begin(); edge != edgeVector.end(); ++edge)
  {
    std::vector<mitk::TubeElement *> allElements = edge->GetElementVector();
    for (unsigned int index = 0; index < edge->GetNumberOfElements(); index++)
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
