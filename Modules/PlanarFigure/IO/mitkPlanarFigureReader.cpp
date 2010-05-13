/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-17 14:15:34 +0200 (Mi, 17. Jun 2009) $
Version:   $Revision: 17745 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFigureReader.h"

#include "mitkPlanarAngle.h"
#include "mitkPlanarCircle.h"
#include "mitkPlanarCross.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarRectangle.h"
#include "mitkPlaneGeometry.h"

#include <tinyxml.h>
#include <itksys/SystemTools.hxx>


mitk::PlanarFigureReader::PlanarFigureReader() : PlanarFigureSource(), FileReader(),
m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredOutputs(1);
  this->SetNumberOfOutputs(1);
  this->SetNthOutput(0, this->MakeOutput(0));
  

  //this->Modified();
  //this->GetOutput()->Modified();
  //this->GetOutput()->ReleaseData();
}


mitk::PlanarFigureReader::~PlanarFigureReader()
{}


mitk::PlanarFigureSource::DataObjectPointer mitk::PlanarFigureReader::MakeOutput ( unsigned int )
{
  return static_cast<itk::DataObject*>(PlanarCircle::New().GetPointer()); // just as a stand in for the pipeline update mechanism. This will be overwritten in GenerateData()
}


void mitk::PlanarFigureReader::GenerateData()
{
  m_Success = false;
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

  TiXmlDocument document( m_FileName);
  if (!document.LoadFile())
  {
    MITK_ERROR << "Could not open/read/parse " << m_FileName << ". TinyXML reports: '" << document.ErrorDesc() << "'. "
              << "The error occurred in row " << document.ErrorRow() << ", column " << document.ErrorCol() << ".";
    return;
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

  this->SetNumberOfOutputs(0); // reset all outputs, we add new ones depending on the file content
  /* file version 1 reader code */
  for( TiXmlElement* pfElement = document.FirstChildElement("PlanarFigure"); pfElement != NULL; pfElement = pfElement->NextSiblingElement("PlanarFigure") )
  {
    if (pfElement == NULL)
      continue;
    std::string closed = pfElement->Attribute("closed");
    std::string type = pfElement->Attribute("type");
    mitk::PlanarFigure::Pointer pf = NULL;
    if (type == "PlanarAngle")
    {
      pf = mitk::PlanarAngle::New();
    }
    else if (type == "PlanarCircle")
    {
      pf = mitk::PlanarCircle::New();
    }
    else if (type == "PlanarCross")
    {
      pf = mitk::PlanarCross::New();
    }
    else if (type == "PlanarFourPointAngle")
    {
      pf = mitk::PlanarFourPointAngle::New();
    }
    else if (type == "PlanarLine")
    {
      pf = mitk::PlanarLine::New();
    }
    else if (type == "PlanarPolygon")
    {
      mitk::PlanarPolygon::Pointer polygon = mitk::PlanarPolygon::New();
      polygon->SetClosed( closed == "true" );
      pf = polygon;
    }
    else if (type == "PlanarRectangle")
    {
      pf = mitk::PlanarRectangle::New();
    }
    else 
    {
      // unknown type
      MITK_WARN << "encountered unknown planar figure type '" << type << "'. Skipping this element.";
      continue;
    }
    TiXmlElement* geoElement = pfElement->FirstChildElement("Geometry");
    if (geoElement != NULL)
    {
      try
      {
        // Create plane geometry
        mitk::PlaneGeometry::Pointer planeGeo = mitk::PlaneGeometry::New();

        // Extract and set plane transform parameters
        DoubleList transformList = this->GetDoubleAttributeListFromXMLNode( geoElement->FirstChildElement( "transformParam" ), "param", 12 );

        typedef mitk::AffineGeometryFrame3D::TransformType TransformType;
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

        typedef mitk::AffineGeometryFrame3D::TransformType TransformType;
        TransformType::Pointer affineGeometry = TransformType::New();
        affineGeometry->SetParameters( parameters );
        planeGeo->SetIndexToWorldTransform( affineGeometry );


        // Extract and set plane bounds
        DoubleList boundsList = this->GetDoubleAttributeListFromXMLNode( geoElement->FirstChildElement( "boundsParam" ), "bound", 6 );

        typedef mitk::Geometry3D::BoundsArrayType BoundsArrayType;

        BoundsArrayType bounds;
        for ( it = boundsList.begin(), i = 0;
              it != boundsList.end();
              ++it, ++i )
        {
          bounds[i] = *it;
        }

        planeGeo->SetBounds( bounds );


        // Extract and set spacing and origin
        //Vector3D spacing = this->GetVectorFromXMLNode(geoElement->FirstChildElement("Spacing"));
        Point3D origin = this->GetPointFromXMLNode(geoElement->FirstChildElement("Origin"));
        planeGeo->SetOrigin( origin );
        pf->SetGeometry2D(planeGeo);
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
        if (vertElement->QueryFloatAttribute("x", &x) == TIXML_WRONG_TYPE)
          return; // TODO: can we do a better error handling?
        if (vertElement->QueryFloatAttribute("y", &y) == TIXML_WRONG_TYPE)
          return; // TODO: can we do a better error handling?
        Point2D p;
        p.SetElement(0, x);
        p.SetElement(1, y);
        if (first == true)  // needed to set m_FigurePlaced to true
        {
          pf->PlaceFigure(p);
          first = false;
        }
        pf->SetControlPoint(id, p, true);
      }

    // Calculate feature quantities of this PlanarFigure
    pf->EvaluateFeatures();

    // Make sure that no control point is currently selected
    pf->DeselectControlPoint();

    // \TODO: what about m_FigurePlaced and m_SelectedControlPoint ??
    this->SetNthOutput( this->GetNumberOfOutputs(), pf );  // add pf as new output of this filter
  }
  m_Success = true;
}

mitk::Point3D mitk::PlanarFigureReader::GetPointFromXMLNode(TiXmlElement* e)
{
  if (e == NULL)
    throw std::invalid_argument("node invalid"); // TODO: can we do a better error handling?
  mitk::Point3D point;
  mitk::ScalarType p(-1.0);
  if (e->QueryFloatAttribute("x", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  point.SetElement(0, p);
  if (e->QueryFloatAttribute("y", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  point.SetElement(1, p);
  if (e->QueryFloatAttribute("z", &p) == TIXML_WRONG_TYPE)
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
  if (e->QueryFloatAttribute("x", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  vector.SetElement(0, p);
  if (e->QueryFloatAttribute("y", &p) == TIXML_WRONG_TYPE)
    throw std::invalid_argument("node malformatted"); // TODO: can we do a better error handling?
  vector.SetElement(1, p);
  if (e->QueryFloatAttribute("z", &p) == TIXML_WRONG_TYPE)
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

    if (e->QueryFloatAttribute( attributeName.str().c_str(), &p ) == TIXML_WRONG_TYPE)
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
  this->SetNumberOfOutputs( num );
  for ( unsigned int i = prevNum; i < num; ++i )
  {
    this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
  }
}
