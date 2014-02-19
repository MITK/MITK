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

#include "mitkTubeGraphReader.h"

#include "mitkTubeGraphDefinitions.h"
#include "mitkTubeGraphGeometry3D.h"
#include "mitkTubeGraphProperty.h"

#include <tinyxml.h>
#include "itksys/SystemTools.hxx"
#include <vtkMatrix4x4.h>
#include <mitkCircularProfileTubeElement.h>

void mitk::TubeGraphReader::GenerateData()
{
  MITK_INFO << "Reading tube graph";
  if (!m_OutputCache)
  {
    Superclass::SetNumberOfRequiredOutputs(0);
    this->GenerateOutputInformation();
  }
  //still empty
  if (!m_OutputCache)
  {
    itkWarningMacro("Tube graph cache is empty!");
    return;
  }

  Superclass::SetNumberOfRequiredOutputs(1);
  Superclass::SetNthOutput(0, m_OutputCache.GetPointer());
}

void mitk::TubeGraphReader::GenerateOutputInformation()
{
  m_OutputCache = OutputType::New();

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);

  if (m_FileName == "")
  {
    MITK_ERROR << "No file name specified.";
  }
  else if (ext == ".tsf")
  {
    try
    {
      TiXmlDocument doc(m_FileName);
      doc.LoadFile();

      TiXmlHandle hDoc(&doc);
      TiXmlElement* pElem;
      TiXmlHandle hRoot(0);

      pElem = hDoc.FirstChildElement().Element();

      // save this for later
      hRoot = TiXmlHandle(pElem);

      pElem = hRoot.FirstChildElement(mitk::TubeGraphDefinitions::XML_GEOMETRY).Element();

      // read geometry
      mitk::TubeGraphGeometry3D::Pointer geometry = mitk::TubeGraphGeometry3D::New();

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

          mitk::TubeGraphReader::OutputType::VertexDescriptorType newVertex = m_OutputCache->AddVertex(vertexData);
          if ( newVertex != vertexID )
          {
            MITK_ERROR << "Aborting tube graph creation, different vertex ids.";
            return;
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
            m_OutputCache->AddEdge( edgeSourceID, edgeTargetID, edgeData);
          }
          catch (const std::runtime_error& error)
          {
            MITK_ERROR << error.what();
            return;
          }

        }
      }
      MITK_INFO << "Tube Graph read";

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
      geometry->Initialize(m_OutputCache);
      m_OutputCache->SetGeometry(geometry);
      m_OutputCache->SetProperty("Tube Graph.Visualization Information", newProperty);
    }
    catch(...)
    {
      MITK_INFO << "Could not read file ";
    }
  }
}

void mitk::TubeGraphReader::Update()
{
  this->GenerateData();
}

const char* mitk::TubeGraphReader::GetFileName() const
{
  return m_FileName.c_str();
}


void mitk::TubeGraphReader::SetFileName(const char* aFileName)
{
  m_FileName = aFileName;
}


const char* mitk::TubeGraphReader::GetFilePrefix() const
{
  return m_FilePrefix.c_str();
}


void mitk::TubeGraphReader::SetFilePrefix(const char* aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}


const char* mitk::TubeGraphReader::GetFilePattern() const
{
  return m_FilePattern.c_str();
}


void mitk::TubeGraphReader::SetFilePattern(const char* aFilePattern)
{
  m_FilePattern = aFilePattern;
}


bool mitk::TubeGraphReader::CanReadFile(
                                        const std::string filename, const std::string /*filePrefix*/,
                                        const std::string /*filePattern*/)
{
  // First check the extension
  if(  filename == "" )
  {
    return false;
  }
  std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
  ext = itksys::SystemTools::LowerCase(ext);

  if (ext == ".tsf")
  {
    return true;
  }

  return false;
}

mitk::BaseDataSource::DataObjectPointer mitk::TubeGraphReader::MakeOutput(const DataObjectIdentifierType &name)
{
  itkDebugMacro("MakeOutput(" << name << ")");
  if( this->IsIndexedOutputName(name) )
    {
    return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
  return static_cast<itk::DataObject*>(OutputType::New().GetPointer());
}

mitk::BaseDataSource::DataObjectPointer mitk::TubeGraphReader::MakeOutput(DataObjectPointerArraySizeType /*idx*/)
{
  return OutputType::New().GetPointer();
}
