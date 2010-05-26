/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkTestingMacros.h"

#include "mitkPlanarAngle.h"
#include "mitkPlanarCircle.h"
#include "mitkPlanarCross.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarRectangle.h"

#include "mitkPlanarFigureWriter.h"
#include "mitkPlanarFigureReader.h"

#include "mitkPlaneGeometry.h"

#include <Poco/Zip/ZipLocalFileHeader.h>
#include "Poco/TemporaryFile.h"


/** \brief Helper class for testing PlanarFigure reader and writer classes. */
class PlanarFigureIOTestClass
{
public:

  typedef std::list< mitk::PlanarFigure::Pointer > PlanarFigureList;

  static PlanarFigureList CreatePlanarFigures()
  {
    PlanarFigureList planarFigures;

    // Create PlaneGeometry on which to place the PlanarFigures
    mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
    planeGeometry->InitializeStandardPlane( 100.0, 100.0 );

    // Create a few sample points for PlanarFigure placement
    mitk::Point2D p0; p0[0] = 20.0; p0[1] = 20.0;
    mitk::Point2D p1; p1[0] = 80.0; p1[1] = 80.0;
    mitk::Point2D p2; p2[0] = 90.0; p2[1] = 10.0;
    mitk::Point2D p3; p3[0] = 10.0; p3[1] = 90.0;

    // Create PlanarAngle
    mitk::PlanarAngle::Pointer planarAngle = mitk::PlanarAngle::New();
    planarAngle->SetGeometry2D( planeGeometry );
    planarAngle->PlaceFigure( p0 );
    planarAngle->SetCurrentControlPoint( p1 );
    planarAngle->AddControlPoint( p2 );
    planarFigures.push_back( planarAngle.GetPointer() );
    
    // Create PlanarCircle
    mitk::PlanarCircle::Pointer planarCircle = mitk::PlanarCircle::New();
    planarCircle->SetGeometry2D( planeGeometry );
    planarCircle->PlaceFigure( p0 );
    planarCircle->SetCurrentControlPoint( p1 );
    planarFigures.push_back( planarCircle.GetPointer() );
    
    // Create PlanarCross
    mitk::PlanarCross::Pointer planarCross = mitk::PlanarCross::New();
    planarCross->SetSingleLineMode( false );
    planarCross->SetGeometry2D( planeGeometry );
    planarCross->PlaceFigure( p0 );
    planarCross->SetCurrentControlPoint( p1 );
    planarCross->AddControlPoint( p2 );
    planarCross->AddControlPoint( p3 );
    planarFigures.push_back( planarCross.GetPointer() );
    
    // Create PlanarFourPointAngle
    mitk::PlanarFourPointAngle::Pointer planarFourPointAngle = mitk::PlanarFourPointAngle::New();
    planarFourPointAngle->SetGeometry2D( planeGeometry );
    planarFourPointAngle->PlaceFigure( p0 );
    planarFourPointAngle->SetCurrentControlPoint( p1 );
    planarFourPointAngle->AddControlPoint( p2 );
    planarFourPointAngle->AddControlPoint( p3 );
    planarFigures.push_back( planarFourPointAngle.GetPointer() );
    
    // Create PlanarLine
    mitk::PlanarLine::Pointer planarLine = mitk::PlanarLine::New();
    planarLine->SetGeometry2D( planeGeometry );
    planarLine->PlaceFigure( p0 );
    planarLine->SetCurrentControlPoint( p1 );
    planarFigures.push_back( planarLine.GetPointer() );
    
    // Create PlanarPolygon
    mitk::PlanarPolygon::Pointer planarPolygon = mitk::PlanarPolygon::New();
    planarPolygon->SetClosed( false );
    planarPolygon->SetGeometry2D( planeGeometry );
    planarPolygon->PlaceFigure( p0 );
    planarPolygon->SetCurrentControlPoint( p1 );
    planarPolygon->AddControlPoint( p2 );
    planarPolygon->AddControlPoint( p3 );
    planarFigures.push_back( planarPolygon.GetPointer() );
    
    // Create PlanarRectangle
    mitk::PlanarRectangle::Pointer planarRectangle = mitk::PlanarRectangle::New();
    planarRectangle->SetGeometry2D( planeGeometry );
    planarRectangle->PlaceFigure( p0 );
    planarRectangle->SetCurrentControlPoint( p1 );
    planarFigures.push_back( planarRectangle.GetPointer() );

    return planarFigures;
  }
  

  static void VerifyPlanarFigures( PlanarFigureList &planarFigures1, PlanarFigureList &planarFigures2 )
  {
    PlanarFigureList::iterator it1, it2;

    for ( it1 = planarFigures1.begin(); it1 != planarFigures1.end(); ++it1 )
    {
      bool planarFigureFound = false;
      for ( it2 = planarFigures2.begin(); it2 != planarFigures2.end(); ++it2 )
      {
        // Compare PlanarFigures (returns false if different types)
        if ( ComparePlanarFigures( *it1, *it2 ) )
        {
          planarFigureFound = true;
        }
      }

      // Test if (at least) on PlanarFigure of the first type was found in the second list
      MITK_TEST_CONDITION_REQUIRED( 
        planarFigureFound,
        "Testing if " << (*it1)->GetNameOfClass() << " has a counterpart" );
    }
  }

  static bool ComparePlanarFigures( mitk::PlanarFigure* figure1, mitk::PlanarFigure* figure2 )
  {
    // Test if PlanarFigures are of same type; otherwise return
    if ( strcmp( figure1->GetNameOfClass(), figure2->GetNameOfClass() ) != 0 )
    {
      return false;
    }

    const char* figureName = figure1->GetNameOfClass();

    // Test for equal number of control points
    MITK_TEST_CONDITION_REQUIRED( 
      figure1->GetNumberOfControlPoints() == figure2->GetNumberOfControlPoints(),
      figureName << ": Testing number of control points" );

    // Test if all control points are equal
    for ( unsigned int i = 0; i < figure1->GetNumberOfControlPoints(); ++i )
    {
      mitk::Point2D& point1 = figure1->GetControlPoint( i );
      mitk::Point2D& point2 = figure2->GetControlPoint( i );

      MITK_TEST_CONDITION_REQUIRED( 
        point1.EuclideanDistanceTo( point2 ) < mitk::eps, 
        figureName << ": Testing equality of control point " << i );
    }


    // Test for equal number of properties
    typedef mitk::PropertyList::PropertyMap PropertyMap;
    const PropertyMap* properties1 = figure1->GetPropertyList()->GetMap();
    const PropertyMap* properties2 = figure2->GetPropertyList()->GetMap();

    MITK_TEST_CONDITION_REQUIRED( 
      properties1->size() == properties2->size(),
      figureName << ": Testing number of properties" );


    // Test if all properties are equal
    MITK_TEST_CONDITION_REQUIRED( 
      std::equal( properties1->begin(), properties1->end(), properties2->begin(), PropertyMapEntryCompare() ),
      figureName << ": Testing equality of properties");


    // Test if Geometry is equal
    const mitk::PlaneGeometry* planeGeometry1 = dynamic_cast<const mitk::PlaneGeometry*>(figure1->GetGeometry2D());
    const mitk::PlaneGeometry* planeGeometry2 = dynamic_cast<const mitk::PlaneGeometry*>(figure2->GetGeometry2D());

    // Test Geometry transform parameters
    bool parametersEqual = true;
    typedef mitk::AffineGeometryFrame3D::TransformType TransformType;
    const TransformType* affineGeometry1 = planeGeometry1->GetIndexToWorldTransform();
    const TransformType::ParametersType& parameters1 = affineGeometry1->GetParameters();
    const TransformType::ParametersType& parameters2 = planeGeometry2->GetIndexToWorldTransform()->GetParameters();
    for ( unsigned int i = 0; i < affineGeometry1->GetNumberOfParameters(); ++i )
    {
      if ( fabs(parameters1.GetElement( i ) - parameters2.GetElement( i )) > mitk::eps )
      {
        parametersEqual = false;
      }
    }

    MITK_TEST_CONDITION_REQUIRED( 
      parametersEqual,
      figureName << ": Testing if Geometry transform parameters are equal");


    // Test Geometry bounds
    bool boundsEqual = true;
    typedef mitk::Geometry3D::BoundsArrayType BoundsArrayType;
    const BoundsArrayType& bounds1 = planeGeometry1->GetBounds();
    const BoundsArrayType& bounds2 = planeGeometry2->GetBounds();
    for ( unsigned int i = 0; i < 6; ++i )
    {
      if ( fabs(bounds1.GetElement( i ) - bounds2.GetElement( i )) > mitk::eps )
      {
        boundsEqual = false;
      };
    }

    MITK_TEST_CONDITION_REQUIRED( 
      boundsEqual,
      figureName << ": Testing if Geometry bounds are equal");


    // Test Geometry spacing and origin
    mitk::Vector3D spacing1 = planeGeometry1->GetSpacing();
    mitk::Vector3D spacing2 = planeGeometry2->GetSpacing();

    MITK_TEST_CONDITION_REQUIRED( 
      (spacing1 - spacing2).GetNorm() < mitk::eps,
      figureName << ": Testing if Geometry spacing is equal");


    mitk::Point3D origin1 = planeGeometry1->GetOrigin();
    mitk::Point3D origin2 = planeGeometry2->GetOrigin();

    MITK_TEST_CONDITION_REQUIRED( 
      origin1.EuclideanDistanceTo( origin2 ) < mitk::eps,
      figureName << ": Testing if Geometry origin are equal");


    return true;
  }

  
  static void SerializePlanarFigures( PlanarFigureList &planarFigures, std::string& fileName )
  {
    //std::string  sceneFileName = Poco::Path::temp() + /*Poco::Path::separator() +*/ "scene.zip";
    std::cout << "File name: " << fileName << std::endl;

    mitk::PlanarFigureWriter::Pointer writer = mitk::PlanarFigureWriter::New();
    writer->SetFileName( fileName.c_str() );

    unsigned int i;
    PlanarFigureList::iterator it;
    for ( it = planarFigures.begin(), i = 0;
          it != planarFigures.end();
          ++it, ++i )
    {
      writer->SetInput( i, *it );
    }

    writer->Update();

    MITK_TEST_CONDITION_REQUIRED( 
      writer->GetSuccess(),
      "Testing if writing was successful");
  }
  

  static PlanarFigureList DeserializePlanarFigures( std::string& fileName)
  {
    // Read in the planar figures
    mitk::PlanarFigureReader::Pointer reader = mitk::PlanarFigureReader::New();
    reader->SetFileName( fileName.c_str() );
    reader->Update();

    MITK_TEST_CONDITION_REQUIRED( 
      reader->GetSuccess(),
      "Testing if reading was successful");


    // Store them in the list and return it
    PlanarFigureList planarFigures;
    for ( unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i )
    {
      mitk::PlanarFigure* figure = reader->GetOutput( i );
      planarFigures.push_back( figure );
    }

    return planarFigures;
  }

private:
  class PropertyMapEntryCompare
  {
  public:
    bool operator()(
      const mitk::PropertyList::PropertyMap::value_type &entry1,
      const mitk::PropertyList::PropertyMap::value_type &entry2 )
    {
      // Compare property objects contained in the map entries (see mitk::PropertyList)
      return *(entry1.second.first) == *(entry2.second.first);
    }
  };
  
}; // end test helper class
  

/** \brief Test for PlanarFigure reader and writer classes.
 *
 * The test works as follows:
 *
 * First, a number of PlanarFigure objects of different types are created and placed with
 * various control points. These objects are the serialized to file, read again from file, and
 * the retrieved objects are compared with their control points, properties, and geometry
 * information to the original PlanarFigure objects.
 */
int mitkPlanarFigureIOTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("PlanarFigureIO")

  // Create a number of PlanarFigure objects
  PlanarFigureIOTestClass::PlanarFigureList originalPlanarFigures = 
    PlanarFigureIOTestClass::CreatePlanarFigures();


  // Write PlanarFigure objects into temp file
  Poco::Path newname( Poco::TemporaryFile::tempName() );
  std::string fileName = Poco::Path::temp() + newname.getFileName() + ".pf";
  PlanarFigureIOTestClass::SerializePlanarFigures( originalPlanarFigures, fileName );


  // Read PlanarFigure objects from temp file
  PlanarFigureIOTestClass::PlanarFigureList retrievedPlanarFigures =
    PlanarFigureIOTestClass::DeserializePlanarFigures( fileName );


  // Test if original and retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures( originalPlanarFigures, retrievedPlanarFigures );


  MITK_TEST_END()
}
