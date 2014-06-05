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

#include "mitkTestingMacros.h"

#include "mitkPlanarAngle.h"
#include "mitkPlanarCircle.h"
#include "mitkPlanarCross.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarSubdivisionPolygon.h"
#include "mitkPlanarRectangle.h"

#include "mitkPlanarFigureWriter.h"
#include "mitkPlanarFigureReader.h"

#include "mitkPlaneGeometry.h"

#include <itksys/SystemTools.hxx>

static mitk::PlanarFigure::Pointer Clone(mitk::PlanarFigure::Pointer original)
{
  return original->Clone();
}

/** \brief Helper class for testing PlanarFigure reader and writer classes. */
class PlanarFigureIOTestClass
{
public:

  typedef std::list< mitk::PlanarFigure::Pointer > PlanarFigureList;
  typedef std::vector< mitk::PlanarFigureWriter::Pointer > PlanarFigureToMemoryWriterList;

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
    planarAngle->SetPlaneGeometry( planeGeometry );
    planarAngle->PlaceFigure( p0 );
    planarAngle->SetCurrentControlPoint( p1 );
    planarAngle->AddControlPoint( p2 );
    planarAngle->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarAngle.GetPointer() );

    // Create PlanarCircle
    mitk::PlanarCircle::Pointer planarCircle = mitk::PlanarCircle::New();
    planarCircle->SetPlaneGeometry( planeGeometry );
    planarCircle->PlaceFigure( p0 );
    planarCircle->SetCurrentControlPoint( p1 );
    planarCircle->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarCircle.GetPointer() );

    // Create PlanarCross
    mitk::PlanarCross::Pointer planarCross = mitk::PlanarCross::New();
    planarCross->SetSingleLineMode( false );
    planarCross->SetPlaneGeometry( planeGeometry );
    planarCross->PlaceFigure( p0 );
    planarCross->SetCurrentControlPoint( p1 );
    planarCross->AddControlPoint( p2 );
    planarCross->AddControlPoint( p3 );
    planarCross->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarCross.GetPointer() );

    // Create PlanarFourPointAngle
    mitk::PlanarFourPointAngle::Pointer planarFourPointAngle = mitk::PlanarFourPointAngle::New();
    planarFourPointAngle->SetPlaneGeometry( planeGeometry );
    planarFourPointAngle->PlaceFigure( p0 );
    planarFourPointAngle->SetCurrentControlPoint( p1 );
    planarFourPointAngle->AddControlPoint( p2 );
    planarFourPointAngle->AddControlPoint( p3 );
    planarFourPointAngle->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarFourPointAngle.GetPointer() );

    // Create PlanarLine
    mitk::PlanarLine::Pointer planarLine = mitk::PlanarLine::New();
    planarLine->SetPlaneGeometry( planeGeometry );
    planarLine->PlaceFigure( p0 );
    planarLine->SetCurrentControlPoint( p1 );
    planarLine->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarLine.GetPointer() );

    // Create PlanarPolygon
    mitk::PlanarPolygon::Pointer planarPolygon = mitk::PlanarPolygon::New();
    planarPolygon->SetClosed( false );
    planarPolygon->SetPlaneGeometry( planeGeometry );
    planarPolygon->PlaceFigure( p0 );
    planarPolygon->SetCurrentControlPoint( p1 );
    planarPolygon->AddControlPoint( p2 );
    planarPolygon->AddControlPoint( p3 );
    planarPolygon->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarPolygon.GetPointer() );

    // Create PlanarSubdivisionPolygon
    mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygon = mitk::PlanarSubdivisionPolygon::New();
    planarSubdivisionPolygon->SetClosed( false );
    planarSubdivisionPolygon->SetPlaneGeometry( planeGeometry );
    planarSubdivisionPolygon->PlaceFigure( p0 );
    planarSubdivisionPolygon->SetCurrentControlPoint( p1 );
    planarSubdivisionPolygon->AddControlPoint( p2 );
    planarSubdivisionPolygon->AddControlPoint( p3 );
    planarSubdivisionPolygon->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarSubdivisionPolygon.GetPointer() );

    // Create PlanarRectangle
    mitk::PlanarRectangle::Pointer planarRectangle = mitk::PlanarRectangle::New();
    planarRectangle->SetPlaneGeometry( planeGeometry );
    planarRectangle->PlaceFigure( p0 );
    planarRectangle->SetCurrentControlPoint( p1 );
    planarRectangle->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarRectangle.GetPointer() );

    //create preciseGeometry which is using float coordinates
    mitk::PlaneGeometry::Pointer preciseGeometry = mitk::PlaneGeometry::New();
    mitk::Vector3D right;
    right[0] = 0.0;
    right[1] = 1.23456;
    right[2] = 0.0;

    mitk::Vector3D down;
    down[0] = 1.23456;
    down[1] = 0.0;
    down[2] = 0.0;

    mitk::Vector3D spacing;
    spacing[0] = 0.0123456;
    spacing[1] = 0.0123456;
    spacing[2] = 1.123456;
    preciseGeometry->InitializeStandardPlane( right, down, &spacing );

    //convert points into the precise coordinates
    mitk::Point2D p0precise; p0precise[0] = p0[0] * spacing[0]; p0precise[1] = p0[1] * spacing[1];
    mitk::Point2D p1precise; p1precise[0] = p1[0] * spacing[0]; p1precise[1] = p1[1] * spacing[1];
    mitk::Point2D p2precise; p2precise[0] = p2[0] * spacing[0]; p2precise[1] = p2[1] * spacing[1];
    mitk::Point2D p3precise; p3precise[0] = p3[0] * spacing[0]; p3precise[1] = p3[1] * spacing[1];

    //Now all PlanarFigures are create using the precise Geometry
    // Create PlanarCross
    mitk::PlanarCross::Pointer nochncross = mitk::PlanarCross::New();
    nochncross->SetSingleLineMode( false );
    nochncross->SetPlaneGeometry( preciseGeometry );
    nochncross->PlaceFigure( p0precise );
    nochncross->SetCurrentControlPoint( p1precise );
    nochncross->AddControlPoint( p2precise );
    nochncross->AddControlPoint( p3precise );
    nochncross->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( nochncross.GetPointer() );

    // Create PlanarAngle
    mitk::PlanarAngle::Pointer planarAnglePrecise = mitk::PlanarAngle::New();
    planarAnglePrecise->SetPlaneGeometry( preciseGeometry );
    planarAnglePrecise->PlaceFigure( p0precise );
    planarAnglePrecise->SetCurrentControlPoint( p1precise );
    planarAnglePrecise->AddControlPoint( p2precise );
    planarAnglePrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarAnglePrecise.GetPointer() );

    // Create PlanarCircle
    mitk::PlanarCircle::Pointer planarCirclePrecise = mitk::PlanarCircle::New();
    planarCirclePrecise->SetPlaneGeometry( preciseGeometry );
    planarCirclePrecise->PlaceFigure( p0precise );
    planarCirclePrecise->SetCurrentControlPoint( p1precise );
    planarCirclePrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarCirclePrecise.GetPointer() );

    // Create PlanarFourPointAngle
    mitk::PlanarFourPointAngle::Pointer planarFourPointAnglePrecise = mitk::PlanarFourPointAngle::New();
    planarFourPointAnglePrecise->SetPlaneGeometry( preciseGeometry );
    planarFourPointAnglePrecise->PlaceFigure( p0precise );
    planarFourPointAnglePrecise->SetCurrentControlPoint( p1precise );
    planarFourPointAnglePrecise->AddControlPoint( p2precise );
    planarFourPointAnglePrecise->AddControlPoint( p3precise );
    planarFourPointAnglePrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarFourPointAnglePrecise.GetPointer() );

    // Create PlanarLine
    mitk::PlanarLine::Pointer planarLinePrecise = mitk::PlanarLine::New();
    planarLinePrecise->SetPlaneGeometry( preciseGeometry );
    planarLinePrecise->PlaceFigure( p0precise );
    planarLinePrecise->SetCurrentControlPoint( p1precise );
    planarLinePrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarLinePrecise.GetPointer() );

    // Create PlanarPolygon
    mitk::PlanarPolygon::Pointer planarPolygonPrecise = mitk::PlanarPolygon::New();
    planarPolygonPrecise->SetClosed( false );
    planarPolygonPrecise->SetPlaneGeometry( preciseGeometry );
    planarPolygonPrecise->PlaceFigure( p0precise );
    planarPolygonPrecise->SetCurrentControlPoint( p1precise );
    planarPolygonPrecise->AddControlPoint( p2precise );
    planarPolygonPrecise->AddControlPoint( p3precise );
    planarPolygonPrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarPolygonPrecise.GetPointer() );

    // Create PlanarSubdivisionPolygon
    mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygonPrecise = mitk::PlanarSubdivisionPolygon::New();
    planarSubdivisionPolygonPrecise->SetClosed( false );
    planarSubdivisionPolygonPrecise->SetPlaneGeometry( preciseGeometry );
    planarSubdivisionPolygonPrecise->PlaceFigure( p0precise );
    planarSubdivisionPolygonPrecise->SetCurrentControlPoint( p1precise );
    planarSubdivisionPolygonPrecise->AddControlPoint( p2precise );
    planarSubdivisionPolygonPrecise->AddControlPoint( p3precise );
    planarSubdivisionPolygonPrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarSubdivisionPolygonPrecise.GetPointer() );

    // Create PlanarRectangle
    mitk::PlanarRectangle::Pointer planarRectanglePrecise = mitk::PlanarRectangle::New();
    planarRectanglePrecise->SetPlaneGeometry( preciseGeometry );
    planarRectanglePrecise->PlaceFigure( p0precise );
    planarRectanglePrecise->SetCurrentControlPoint( p1precise );
    planarRectanglePrecise->GetPropertyList()->SetBoolProperty( "initiallyplaced", true );
    planarFigures.push_back( planarRectanglePrecise.GetPointer() );

    return planarFigures;
  }

  static PlanarFigureList CreateDeepCopiedPlanarFigures(PlanarFigureList original)
  {
    PlanarFigureList copiedPlanarFigures;

    PlanarFigureList::iterator it1;

    for ( it1 = original.begin(); it1 != original.end(); ++it1 )
    {
      mitk::PlanarFigure::Pointer copiedFigure = (*it1)->Clone();

      copiedPlanarFigures.push_back(copiedFigure);
    }
    return copiedPlanarFigures;
  }

  static PlanarFigureList CreateClonedPlanarFigures(PlanarFigureList original)
  {
    PlanarFigureList clonedPlanarFigures;
    clonedPlanarFigures.resize(original.size());
    std::transform(original.begin(), original.end(), clonedPlanarFigures.begin(), Clone);
    return clonedPlanarFigures;
  }

  static void VerifyPlanarFigures( PlanarFigureList &planarFigures1, PlanarFigureList &planarFigures2 )
  {
    PlanarFigureList::iterator it1, it2;

    int i = 0;
    for ( it1 = planarFigures1.begin(); it1 != planarFigures1.end(); ++it1 )
    {
      bool planarFigureFound = false;
      int j = 0;
      for ( it2 = planarFigures2.begin(); it2 != planarFigures2.end(); ++it2 )
      {
        // Compare PlanarFigures (returns false if different types)
        if ( ComparePlanarFigures( *it1, *it2 ) )
        {
          planarFigureFound = true;
        }
        ++j;
      }

      // Test if (at least) on PlanarFigure of the first type was found in the second list
      MITK_TEST_CONDITION_REQUIRED(
          planarFigureFound,
          "Testing if " << (*it1)->GetNameOfClass() << " has a counterpart " << i );
      ++i;
    }
  }

  static bool ComparePlanarFigures( mitk::PlanarFigure* figure1, mitk::PlanarFigure* figure2 )
  {
    // Test if PlanarFigures are of same type; otherwise return
    if ( strcmp( figure1->GetNameOfClass(), figure2->GetNameOfClass() ) != 0 )
    {
      return false;
    }

    if( strcmp( figure1->GetNameOfClass(), "PlanarCross" ) == 0 )
    {
      std::cout << "Planar Cross Found" << std::endl;
    }

    // Test for equal number of control points
    if(figure1->GetNumberOfControlPoints() != figure2->GetNumberOfControlPoints())
    {
      return false;
    }

    // Test if all control points are equal
    for ( unsigned int i = 0; i < figure1->GetNumberOfControlPoints(); ++i )
    {
      mitk::Point2D point1 = figure1->GetControlPoint( i );
      mitk::Point2D point2 = figure2->GetControlPoint( i );

      if(point1.EuclideanDistanceTo( point2 ) >= mitk::eps)
      {
        return false;
      }
    }

    // Test for equal number of properties
    typedef mitk::PropertyList::PropertyMap PropertyMap;
    const PropertyMap* properties1 = figure1->GetPropertyList()->GetMap();
    const PropertyMap* properties2 = figure2->GetPropertyList()->GetMap();

    if(properties1->size() != properties2->size())
    {
      return false;
    }

    MITK_INFO << "List 1:";
    for (PropertyMap::const_iterator i1 = properties1->begin(); i1 != properties1->end(); ++i1)
    {
      std::cout << i1->first << std::endl;
    }

    MITK_INFO << "List 2:";
    for (PropertyMap::const_iterator i2 = properties2->begin(); i2 != properties2->end(); ++i2)
    {
      std::cout << i2->first << std::endl;
    }

    MITK_INFO << "-------";

    // Test if all properties are equal
    if(!std::equal( properties1->begin(), properties1->end(), properties2->begin(), PropertyMapEntryCompare() ))
    {
      return false;
    }

    // Test if Geometry is equal
    const mitk::PlaneGeometry* planeGeometry1 = dynamic_cast<const mitk::PlaneGeometry*>(figure1->GetPlaneGeometry());
    const mitk::PlaneGeometry* planeGeometry2 = dynamic_cast<const mitk::PlaneGeometry*>(figure2->GetPlaneGeometry());

    // Test Geometry transform parameters
    typedef mitk::Geometry3D::TransformType TransformType;
    const TransformType* affineGeometry1 = planeGeometry1->GetIndexToWorldTransform();
    const TransformType::ParametersType& parameters1 = affineGeometry1->GetParameters();
    const TransformType::ParametersType& parameters2 = planeGeometry2->GetIndexToWorldTransform()->GetParameters();
    for ( unsigned int i = 0; i < affineGeometry1->GetNumberOfParameters(); ++i )
    {
      if ( fabs(parameters1.GetElement( i ) - parameters2.GetElement( i )) >= mitk::eps )
      {
        return false;
      }
    }

    // Test Geometry bounds
    typedef mitk::Geometry3D::BoundsArrayType BoundsArrayType;
    const BoundsArrayType& bounds1 = planeGeometry1->GetBounds();
    const BoundsArrayType& bounds2 = planeGeometry2->GetBounds();
    for ( unsigned int i = 0; i < 6; ++i )
    {
      if ( fabs(bounds1.GetElement( i ) - bounds2.GetElement( i )) >= mitk::eps )
      {
        return false;
      };
    }

    // Test Geometry spacing and origin
    mitk::Vector3D spacing1 = planeGeometry1->GetSpacing();
    mitk::Vector3D spacing2 = planeGeometry2->GetSpacing();
    if((spacing1 - spacing2).GetNorm() >= mitk::eps)
    {
      return false;
    }

    mitk::Point3D origin1 = planeGeometry1->GetOrigin();
    mitk::Point3D origin2 = planeGeometry2->GetOrigin();

    if(origin1.EuclideanDistanceTo( origin2 ) >= mitk::eps)
    {
      return false;
    }
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




  static PlanarFigureToMemoryWriterList SerializePlanarFiguresToMemoryBuffers( PlanarFigureList &planarFigures )
  {
    PlanarFigureToMemoryWriterList pfMemoryWriters;
    unsigned int i;
    PlanarFigureList::iterator it;

    bool success = true;
    for ( it = planarFigures.begin(), i = 0;
    it != planarFigures.end();
    ++it, ++i )
    {
      mitk::PlanarFigureWriter::Pointer writer = mitk::PlanarFigureWriter::New();
      writer->SetWriteToMemory( true );
      writer->SetInput( *it );
      writer->Update();

      pfMemoryWriters.push_back(writer);

      if(!writer->GetSuccess())
        success = false;
    }

    MITK_TEST_CONDITION_REQUIRED(success, "Testing if writing to memory buffers was successful");

    return pfMemoryWriters;
  }

  static PlanarFigureList DeserializePlanarFiguresFromMemoryBuffers( PlanarFigureToMemoryWriterList pfMemoryWriters)
  {
    // Store them in the list and return it
    PlanarFigureList planarFigures;
    bool success = true;
    for ( unsigned int i = 0; i < pfMemoryWriters.size(); ++i )
    {
      // Read in the planar figures
      mitk::PlanarFigureReader::Pointer reader = mitk::PlanarFigureReader::New();
      reader->SetReadFromMemory( true );
      reader->SetMemoryBuffer(pfMemoryWriters[i]->GetMemoryPointer(), pfMemoryWriters[i]->GetMemorySize());
      reader->Update();
      mitk::PlanarFigure* figure = reader->GetOutput( 0 );
      planarFigures.push_back( figure );

      if(!reader->GetSuccess())
        success = false;
    }

    MITK_TEST_CONDITION_REQUIRED(success, "Testing if reading was successful");

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
      MITK_INFO << "Comparing " << entry1.first << "(" << entry1.second->GetValueAsString() << ") and " << entry2.first << "(" << entry2.second->GetValueAsString() << ")";
      // Compare property objects contained in the map entries (see mitk::PropertyList)
      return *(entry1.second) == *(entry2.second);
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
  MITK_TEST_BEGIN("PlanarFigureIO");

  // Create a number of PlanarFigure objects
  PlanarFigureIOTestClass::PlanarFigureList originalPlanarFigures =
      PlanarFigureIOTestClass::CreatePlanarFigures();

  // Create a number of "deep-copied" planar figures to test the DeepCopy function (deprecated)
  PlanarFigureIOTestClass::PlanarFigureList copiedPlanarFigures =
      PlanarFigureIOTestClass::CreateDeepCopiedPlanarFigures(originalPlanarFigures);

  PlanarFigureIOTestClass::VerifyPlanarFigures(originalPlanarFigures, copiedPlanarFigures );

  // Create a number of cloned planar figures to test the Clone function
  PlanarFigureIOTestClass::PlanarFigureList clonedPlanarFigures =
      PlanarFigureIOTestClass::CreateClonedPlanarFigures(originalPlanarFigures);

  PlanarFigureIOTestClass::VerifyPlanarFigures(originalPlanarFigures, clonedPlanarFigures );

  // Write PlanarFigure objects into temp file
  // tmpname
  static unsigned long count = 0;
  unsigned long n = count++;
  std::ostringstream name;
  for (int i = 0; i < 6; ++i)
  {
    name << char('a' + (n % 26));
    n /= 26;
  }
  std::string myname;
  myname.append(name.str());

  std::string fileName = itksys::SystemTools::GetCurrentWorkingDirectory() + myname + ".pf";

  PlanarFigureIOTestClass::SerializePlanarFigures( originalPlanarFigures, fileName );

  // Write PlanarFigure objects to memory buffers
  PlanarFigureIOTestClass::PlanarFigureToMemoryWriterList writersWithMemoryBuffers =
      PlanarFigureIOTestClass::SerializePlanarFiguresToMemoryBuffers( originalPlanarFigures );

  // Read PlanarFigure objects from temp file
  PlanarFigureIOTestClass::PlanarFigureList retrievedPlanarFigures =
      PlanarFigureIOTestClass::DeserializePlanarFigures( fileName );

  // Read PlanarFigure objects from memory buffers
  PlanarFigureIOTestClass::PlanarFigureList retrievedPlanarFiguresFromMemory =
      PlanarFigureIOTestClass::DeserializePlanarFiguresFromMemoryBuffers( writersWithMemoryBuffers );

  PlanarFigureIOTestClass::PlanarFigureToMemoryWriterList::iterator it = writersWithMemoryBuffers.begin();
  while(it != writersWithMemoryBuffers.end())
  {
    (*it)->ReleaseMemory();
    ++it;
  }

  // Test if original and retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures( originalPlanarFigures, retrievedPlanarFigures );

  // Test if original and memory retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures( originalPlanarFigures, retrievedPlanarFiguresFromMemory );

  //empty the originalPlanarFigures
  originalPlanarFigures.empty();

  // Test if deep-copied and retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures( copiedPlanarFigures, retrievedPlanarFigures );





  MITK_TEST_END()
    }
