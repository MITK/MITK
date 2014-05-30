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


#include "mitkPlanarFigureReader.h"

#include "mitkPlanarAngle.h"
#include "mitkPlanarCircle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarArrow.h"
#include "mitkPlanarCross.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkPlanarRectangle.h"
#include "mitkPlaneGeometry.h"
#include "mitkPlanarEllipse.h"
#include "mitkPlanarDoubleEllipse.h"
#include "mitkPlanarBezierCurve.h"

#include "mitkBasePropertySerializer.h"

#include <tinyxml.h>
#include <itksys/SystemTools.hxx>


mitk::PlanarFigureReader::PlanarFigureReader() : PlanarFigureSource(), FileReader(),
m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredOutputs(1);
  this->SetNumberOfIndexedOutputs(1);
  this->SetNthOutput(0, this->MakeOutput(0));

  m_CanReadFromMemory = true;


  //this->Modified();
  //this->GetOutput()->Modified();
  //this->GetOutput()->ReleaseData();
}


mitk::PlanarFigureReader::~PlanarFigureReader()
{}


void mitk::PlanarFigureReader::GenerateData()
{
  m_Success = false;
  this->SetNumberOfIndexedOutputs(0); // reset all outputs, we add new ones depending on the file content

  TiXmlDocument document;

  if(m_ReadFromMemory)
  {
    if(m_MemoryBuffer == NULL || m_MemorySize == 0)
    {
      //check
      itkWarningMacro( << "Sorry, memory buffer has not been set!" );
      return;
    }
    if(m_MemoryBuffer[ m_MemorySize - 1 ]  == '\0')
    {
      document.Parse(m_MemoryBuffer);
    }
    else
    {
      char * tmpArray = new char[(int)m_MemorySize+1];
      tmpArray[m_MemorySize] = '\0';
      memcpy(tmpArray,m_MemoryBuffer,m_MemorySize);

      document.Parse(m_MemoryBuffer);

      delete [] tmpArray;
    }
  }
  else
  {
    if (m_FileName.empty())
    {
      itkWarningMacro( << "Sorry, filename has not been set!" );
      return;
    }
    if (this->CanReadFile( m_FileName.c_str()) == false)
    {
      itkWarningMacro( << "Sorry, can't read file " << m_FileName << "!" );
      return;
    }
    if (!document.LoadFile(m_FileName))
    {
      MITK_ERROR << "Could not open/read/parse " << m_FileName << ". TinyXML reports: '" << document.ErrorDesc() << "'. "
              << "The error occurred in row " << document.ErrorRow() << ", column " << document.ErrorCol() << ".";
      return;
    }
  }

  int fileVersion = 1;
  TiXmlElement* versionObject = document.FirstChildElement("Version");
  if (versionObject != NULL)
  {
    if ( versionObject->QueryIntAttribute( "FileVersion", &fileVersion ) != TIXML_SUCCESS )
    {
      MITK_WARN << m_FileName << " does not contain version information! Trying version 1 format." << std::endl;
    }
  }
  else
  {
    MITK_WARN << m_FileName << " does not contain version information! Trying version 1 format." << std::endl;
  }
  if (fileVersion != 1)  // add file version selection and version specific file parsing here, if newer file versions are created
  {
    MITK_WARN << "File version > 1 is not supported by this reader.";
      return;
  }

  /* file version 1 reader code */
  for( TiXmlElement* pfElement = document.FirstChildElement("PlanarFigure");
       pfElement != NULL;
       pfElement = pfElement->NextSiblingElement("PlanarFigure") )
  {
    if (pfElement == NULL)
      continue;

    std::string type = pfElement->Attribute("type");

    mitk::PlanarFigure::Pointer planarFigure = NULL;
    if (type == "PlanarAngle")
    {
      planarFigure = mitk::PlanarAngle::New();
    }
    else if (type == "PlanarCircle")
    {
      planarFigure = mitk::PlanarCircle::New();
    }
    else if (type == "PlanarEllipse")
    {
      planarFigure = mitk::PlanarEllipse::New();
    }
    else if (type == "PlanarCross")
    {
      planarFigure = mitk::PlanarCross::New();
    }
    else if (type == "PlanarFourPointAngle")
    {
      planarFigure = mitk::PlanarFourPointAngle::New();
    }
    else if (type == "PlanarLine")
    {
      planarFigure = mitk::PlanarLine::New();
    }
    else if (type == "PlanarPolygon")
    {
      planarFigure = mitk::PlanarPolygon::New();
    }
    else if (type == "PlanarSubdivisionPolygon")
    {
      planarFigure = mitk::PlanarSubdivisionPolygon::New();
    }
    else if (type == "PlanarRectangle")
    {
      planarFigure = mitk::PlanarRectangle::New();
    }
    else if (type == "PlanarArrow")
    {
      planarFigure = mitk::PlanarArrow::New();
    }
    else if (type == "PlanarDoubleEllipse")
    {
      planarFigure = mitk::PlanarDoubleEllipse::New();
    }
    else if (type == "PlanarBezierCurve")
    {
      planarFigure = mitk::PlanarBezierCurve::New();
    }
    else
    {
      // unknown type
      MITK_WARN << "encountered unknown planar figure type '" << type << "'. Skipping this element.";
      continue;
    }


    // Read properties of the planar figure
    for( TiXmlElement* propertyElement = pfElement->FirstChildElement("property");
         propertyElement != NULL;
         propertyElement = propertyElement->NextSiblingElement("property") )
    {
      const char* keya = propertyElement->Attribute("key");
      std::string key( keya ? keya : "");

      const char* typea = propertyElement->Attribute("type");
      std::string type( typea ? typea : "");

      // hand propertyElement to specific reader
      std::stringstream propertyDeserializerClassName;
      propertyDeserializerClassName << type << "Serializer";

      std::list<itk::LightObject::Pointer> readers =
        itk::ObjectFactoryBase::CreateAllInstance(propertyDeserializerClassName.str().c_str());
      if (readers.size() < 1)
      {
        MITK_ERROR << "No property reader found for " << type;
      }
      if (readers.size() > 1)
      {
        MITK_WARN << "Multiple property readers found for " << type << ". Using arbitrary first one.";
      }

      for ( std::list<itk::LightObject::Pointer>::iterator iter = readers.begin();
        iter != readers.end();
        ++iter )
      {
        if (BasePropertySerializer* reader = dynamic_cast<BasePropertySerializer*>( iter->GetPointer() ) )
        {
          BaseProperty::Pointer property = reader->Deserialize( propertyElement->FirstChildElement() );
          if (property.IsNotNull())
          {
            planarFigure->GetPropertyList()->ReplaceProperty(key, property);
          }
          else
          {
            MITK_ERROR << "There were errors while loading property '" << key << "' of type " << type << ". Your data may be corrupted";
          }
          break;
        }
      }
    }

    // If we load a planarFigure, it has definitely been placed correctly.
    // If we do not set this property here, we cannot load old planarFigures
    // without messing up the interaction (PF-Interactor needs this property.
    planarFigure->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );


    // Read geometry of containing plane
    TiXmlElement* geoElement = pfElement->FirstChildElement("Geometry");
    if (geoElement != NULL)
    {
      try
      {
        // Create plane geometry
        mitk::PlaneGeometry::Pointer planeGeo = mitk::PlaneGeometry::New();

        // Extract and set plane transform parameters
        DoubleList transformList = this->GetDoubleAttributeListFromXMLNode( geoElement->FirstChildElement( "transformParam" ), "param", 12 );

        typedef mitk::BaseGeometry::TransformType TransformType;
        TransformType::ParametersType parameters;
        parameters.SetSize( 12 );

        unsigned int i;
        DoubleList::iterator it;
        for ( it = transformList.begin(), i = 0;
              it != transformList.end();
              ++it, ++i )
        {
          parameters.SetElement( i, *it );
        }

        typedef mitk::BaseGeometry::TransformType TransformType;
        TransformType::Pointer affineGeometry = TransformType::New();
        affineGeometry->SetParameters( parameters );
        planeGeo->SetIndexToWorldTransform( affineGeometry );


        // Extract and set plane bounds
        DoubleList boundsList = this->GetDoubleAttributeListFromXMLNode( geoElement->FirstChildElement( "boundsParam" ), "bound", 6 );

        typedef mitk::BaseGeometry::BoundsArrayType BoundsArrayType;

        BoundsArrayType bounds;
        for ( it = boundsList.begin(), i = 0;
              it != boundsList.end();
              ++it, ++i )
        {
          bounds[i] = *it;
        }

        planeGeo->SetBounds( bounds );


        // Extract and set spacing and origin
        Vector3D spacing = this->GetVectorFromXMLNode(geoElement->FirstChildElement("Spacing"));
        planeGeo->SetSpacing( spacing );

        Point3D origin = this->GetPointFromXMLNode(geoElement->FirstChildElement("Origin"));
        planeGeo->SetOrigin( origin );
        planarFigure->SetPlaneGeometry(planeGeo);
      }
      catch (...)
      {
      }
    }
    TiXmlElement* cpElement = pfElement->FirstChildElement("ControlPoints");
    bool first = true;
    if (cpElement != NULL)
      for( TiXmlElement* vertElement = cpElement->FirstChildElement("Vertex"); vertElement != NULL; vertElement = vertElement->NextSiblingElement("Vertex"))
      {
        if (vertElement == NULL)
          continue;
        int id = 0;
        mitk::Point2D::ValueType x = 0.0;
        mitk::Point2D::ValueType y = 0.0;
        if (vertElement->QueryIntAttribute("id", &id) == TIXML_WRONG_TYPE)
          return; // TODO: can we do a better error handling?
        if (vertElement->QueryDoubleAttribute("x", &x) == TIXML_WRONG_TYPE)
          return; // TODO: can we do a better error handling?
        if (vertElement->QueryDoubleAttribute("y", &y) == TIXML_WRONG_TYPE)
          return; // TODO: can we do a better error handling?
        Point2D p;
        p.SetElement(0, x);
        p.SetElement(1, y);
        if (first == true)  // needed to set m_FigurePlaced to true
        {
          planarFigure->PlaceFigure(p);
          first = false;
        }
        planarFigure->SetControlPoint(id, p, true);
      }

    // Calculate feature quantities of this PlanarFigure
    planarFigure->EvaluateFeatures();

    // Make sure that no control point is currently selected
    planarFigure->DeselectControlPoint();

    // \TODO: what about m_FigurePlaced and m_SelectedControlPoint ??
    this->SetNthOutput( this->GetNumberOfOutputs(), planarFigure );  // add planarFigure as new output of this filter
  }
  m_Success = true;
}

mitk::Point3D mitk::PlanarFigureReader::GetPointFromXMLNode(TiXmlElement* e)
{
  if (e == NULL)
    throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?
  mitk::Point3D point;
  mitk::ScalarType p(-1.0);
  if (e->QueryDoubleAttribute("x", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  point.SetElement(0, p);
  if (e->QueryDoubleAttribute("y", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  point.SetElement(1, p);
  if (e->QueryDoubleAttribute("z", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  point.SetElement(2, p);
  return point;
}


mitk::Vector3D mitk::PlanarFigureReader::GetVectorFromXMLNode(TiXmlElement* e)
{
  if (e == NULL)
    throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?
  mitk::Vector3D vector;
  mitk::ScalarType p(-1.0);
  if (e->QueryDoubleAttribute("x", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  vector.SetElement(0, p);
  if (e->QueryDoubleAttribute("y", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  vector.SetElement(1, p);
  if (e->QueryDoubleAttribute("z", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  vector.SetElement(2, p);
  return vector;
}

mitk::PlanarFigureReader::DoubleList
mitk::PlanarFigureReader::GetDoubleAttributeListFromXMLNode(TiXmlElement* e, const char *attributeNameBase, unsigned int count)
{
  DoubleList list;

  if (e == NULL)
    throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?

  for ( unsigned int i = 0; i < count; ++i )
  {
    mitk::ScalarType p(-1.0);
    std::stringstream attributeName;
    attributeName << attributeNameBase << i;

    if (e->QueryDoubleAttribute( attributeName.str().c_str(), &p ) == TIXML_WRONG_TYPE)
      throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
    list.push_back( p );
  }


  return list;
}

void mitk::PlanarFigureReader::GenerateOutputInformation()
{
}

int mitk::PlanarFigureReader::CanReadFile ( const char *name )
{
  if (std::string(name).empty())
    return false;

  return (itksys::SystemTools::LowerCase(itksys::SystemTools::GetFilenameLastExtension(name)) == ".pf");  //assume, we can read all .pf files

  //TiXmlDocument document(name);
  //if (document.LoadFile() == false)
  //  return false;
  //return (document.FirstChildElement("PlanarFigure") != NULL);
}

bool mitk::PlanarFigureReader::CanReadFile(const std::string filename, const std::string, const std::string)
{
  if (filename.empty())
    return false;

  return (itksys::SystemTools::LowerCase(itksys::SystemTools::GetFilenameLastExtension(filename)) == ".pf");  //assume, we can read all .pf files

  //TiXmlDocument document(filename);
  //if (document.LoadFile() == false)
  //  return false;
  //return (document.FirstChildElement("PlanarFigure") != NULL);
}

void mitk::PlanarFigureReader::ResizeOutputs( const unsigned int& num )
{
  unsigned int prevNum = this->GetNumberOfOutputs();
  this->SetNumberOfIndexedOutputs( num );
  for ( unsigned int i = prevNum; i < num; ++i )
  {
    this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
  }
}
