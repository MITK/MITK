/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestingMacros.h"

#include "mitkPlanarAngle.h"
#include "mitkPlanarCircle.h"
#include "mitkPlanarCross.h"
#include "mitkPlanarFourPointAngle.h"
#include "mitkPlanarLine.h"
#include "mitkPlanarPolygon.h"
#include "mitkPlanarRectangle.h"
#include "mitkPlanarSubdivisionPolygon.h"

#include "mitkPlaneGeometry.h"

#include "mitkGeometry3D.h"
#include "mitkAbstractFileIO.h"
#include "mitkFileReaderRegistry.h"
#include "mitkFileWriterRegistry.h"
#include "mitkIOUtil.h"

#include <itksys/SystemTools.hxx>

/** \brief Helper class for testing PlanarFigure reader and writer classes. */
class PlanarFigureIOTestClass
{
public:
  typedef std::map<const std::string, mitk::PlanarFigure::Pointer> PlanarFigureMap;
  typedef std::map<const std::string, std::string> PlanarFigureToStreamMap;

  static PlanarFigureMap CreatePlanarFigures()
  {
    PlanarFigureMap planarFigures;

    // Create PlaneGeometry on which to place the PlanarFigures
    mitk::PlaneGeometry::Pointer planeGeometry = mitk::PlaneGeometry::New();
    planeGeometry->InitializeStandardPlane(100.0, 100.0);

    // Create a few sample points for PlanarFigure placement
    mitk::Point2D p0;
    p0[0] = 20.0;
    p0[1] = 20.0;
    mitk::Point2D p1;
    p1[0] = 80.0;
    p1[1] = 80.0;
    mitk::Point2D p2;
    p2[0] = 90.0;
    p2[1] = 10.0;
    mitk::Point2D p3;
    p3[0] = 10.0;
    p3[1] = 90.0;

    // Create PlanarAngle
    mitk::PlanarAngle::Pointer planarAngle = mitk::PlanarAngle::New();
    planarAngle->SetPlaneGeometry(planeGeometry);
    planarAngle->PlaceFigure(p0);
    planarAngle->SetCurrentControlPoint(p1);
    planarAngle->AddControlPoint(p2);
    planarAngle->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarAngle",planarAngle.GetPointer());

    // Create PlanarCircle
    mitk::PlanarCircle::Pointer planarCircle = mitk::PlanarCircle::New();
    planarCircle->SetPlaneGeometry(planeGeometry);
    planarCircle->PlaceFigure(p0);
    planarCircle->SetCurrentControlPoint(p1);
    planarCircle->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarCircle",planarCircle.GetPointer());

    // Create PlanarCross
    mitk::PlanarCross::Pointer planarCross = mitk::PlanarCross::New();
    planarCross->SetSingleLineMode(false);
    planarCross->SetPlaneGeometry(planeGeometry);
    planarCross->PlaceFigure(p0);
    planarCross->SetCurrentControlPoint(p1);
    planarCross->AddControlPoint(p2);
    planarCross->AddControlPoint(p3);
    planarCross->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarCross",planarCross.GetPointer());

    // Create PlanarFourPointAngle
    mitk::PlanarFourPointAngle::Pointer planarFourPointAngle = mitk::PlanarFourPointAngle::New();
    planarFourPointAngle->SetPlaneGeometry(planeGeometry);
    planarFourPointAngle->PlaceFigure(p0);
    planarFourPointAngle->SetCurrentControlPoint(p1);
    planarFourPointAngle->AddControlPoint(p2);
    planarFourPointAngle->AddControlPoint(p3);
    planarFourPointAngle->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarFourPointAngle",planarFourPointAngle.GetPointer());

    // Create PlanarLine
    mitk::PlanarLine::Pointer planarLine = mitk::PlanarLine::New();
    planarLine->SetPlaneGeometry(planeGeometry);
    planarLine->PlaceFigure(p0);
    planarLine->SetCurrentControlPoint(p1);
    planarLine->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarLine",planarLine.GetPointer());

    // Create PlanarPolygon
    mitk::PlanarPolygon::Pointer planarPolygon = mitk::PlanarPolygon::New();
    planarPolygon->SetClosed(false);
    planarPolygon->SetPlaneGeometry(planeGeometry);
    planarPolygon->PlaceFigure(p0);
    planarPolygon->SetCurrentControlPoint(p1);
    planarPolygon->AddControlPoint(p2);
    planarPolygon->AddControlPoint(p3);
    planarPolygon->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarPolygon",planarPolygon.GetPointer());

    // Create PlanarSubdivisionPolygon
    mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygon = mitk::PlanarSubdivisionPolygon::New();
    planarSubdivisionPolygon->SetClosed(false);
    planarSubdivisionPolygon->SetPlaneGeometry(planeGeometry);
    planarSubdivisionPolygon->PlaceFigure(p0);
    planarSubdivisionPolygon->SetCurrentControlPoint(p1);
    planarSubdivisionPolygon->AddControlPoint(p2);
    planarSubdivisionPolygon->AddControlPoint(p3);
    planarSubdivisionPolygon->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarSubdivisionPolygon",planarSubdivisionPolygon.GetPointer());

    // Create PlanarRectangle
    mitk::PlanarRectangle::Pointer planarRectangle = mitk::PlanarRectangle::New();
    planarRectangle->SetPlaneGeometry(planeGeometry);
    planarRectangle->PlaceFigure(p0);
    planarRectangle->SetCurrentControlPoint(p1);
    planarRectangle->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarRectangle",planarRectangle.GetPointer());

    // create preciseGeometry which is using float coordinates
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
    preciseGeometry->InitializeStandardPlane(right, down, &spacing);

    // convert points into the precise coordinates
    mitk::Point2D p0precise;
    p0precise[0] = p0[0] * spacing[0];
    p0precise[1] = p0[1] * spacing[1];
    mitk::Point2D p1precise;
    p1precise[0] = p1[0] * spacing[0];
    p1precise[1] = p1[1] * spacing[1];
    mitk::Point2D p2precise;
    p2precise[0] = p2[0] * spacing[0];
    p2precise[1] = p2[1] * spacing[1];
    mitk::Point2D p3precise;
    p3precise[0] = p3[0] * spacing[0];
    p3precise[1] = p3[1] * spacing[1];

    // Now all PlanarFigures are create using the precise Geometry
    // Create PlanarCross
    mitk::PlanarCross::Pointer nochncross = mitk::PlanarCross::New();
    nochncross->SetSingleLineMode(false);
    nochncross->SetPlaneGeometry(preciseGeometry);
    nochncross->PlaceFigure(p0precise);
    nochncross->SetCurrentControlPoint(p1precise);
    nochncross->AddControlPoint(p2precise);
    nochncross->AddControlPoint(p3precise);
    nochncross->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("nochncross", nochncross.GetPointer());

    // Create PlanarAngle
    mitk::PlanarAngle::Pointer planarAnglePrecise = mitk::PlanarAngle::New();
    planarAnglePrecise->SetPlaneGeometry(preciseGeometry);
    planarAnglePrecise->PlaceFigure(p0precise);
    planarAnglePrecise->SetCurrentControlPoint(p1precise);
    planarAnglePrecise->AddControlPoint(p2precise);
    planarAnglePrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarAnglePrecise",planarAnglePrecise.GetPointer());

    // Create PlanarCircle
    mitk::PlanarCircle::Pointer planarCirclePrecise = mitk::PlanarCircle::New();
    planarCirclePrecise->SetPlaneGeometry(preciseGeometry);
    planarCirclePrecise->PlaceFigure(p0precise);
    planarCirclePrecise->SetCurrentControlPoint(p1precise);
    planarCirclePrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarCirclePrecise",planarCirclePrecise.GetPointer());

    // Create PlanarFourPointAngle
    mitk::PlanarFourPointAngle::Pointer planarFourPointAnglePrecise = mitk::PlanarFourPointAngle::New();
    planarFourPointAnglePrecise->SetPlaneGeometry(preciseGeometry);
    planarFourPointAnglePrecise->PlaceFigure(p0precise);
    planarFourPointAnglePrecise->SetCurrentControlPoint(p1precise);
    planarFourPointAnglePrecise->AddControlPoint(p2precise);
    planarFourPointAnglePrecise->AddControlPoint(p3precise);
    planarFourPointAnglePrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarFourPointAnglePrecise",planarFourPointAnglePrecise.GetPointer());

    // Create PlanarLine
    mitk::PlanarLine::Pointer planarLinePrecise = mitk::PlanarLine::New();
    planarLinePrecise->SetPlaneGeometry(preciseGeometry);
    planarLinePrecise->PlaceFigure(p0precise);
    planarLinePrecise->SetCurrentControlPoint(p1precise);
    planarLinePrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarLinePrecise",planarLinePrecise.GetPointer());

    // Create PlanarPolygon
    mitk::PlanarPolygon::Pointer planarPolygonPrecise = mitk::PlanarPolygon::New();
    planarPolygonPrecise->SetClosed(false);
    planarPolygonPrecise->SetPlaneGeometry(preciseGeometry);
    planarPolygonPrecise->PlaceFigure(p0precise);
    planarPolygonPrecise->SetCurrentControlPoint(p1precise);
    planarPolygonPrecise->AddControlPoint(p2precise);
    planarPolygonPrecise->AddControlPoint(p3precise);
    planarPolygonPrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarPolygonPrecise",planarPolygonPrecise.GetPointer());

    // Create PlanarSubdivisionPolygon
    mitk::PlanarSubdivisionPolygon::Pointer planarSubdivisionPolygonPrecise = mitk::PlanarSubdivisionPolygon::New();
    planarSubdivisionPolygonPrecise->SetClosed(false);
    planarSubdivisionPolygonPrecise->SetPlaneGeometry(preciseGeometry);
    planarSubdivisionPolygonPrecise->PlaceFigure(p0precise);
    planarSubdivisionPolygonPrecise->SetCurrentControlPoint(p1precise);
    planarSubdivisionPolygonPrecise->AddControlPoint(p2precise);
    planarSubdivisionPolygonPrecise->AddControlPoint(p3precise);
    planarSubdivisionPolygonPrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarSubdivisionPolygonPrecise",planarSubdivisionPolygonPrecise.GetPointer());

    // Create PlanarRectangle
    mitk::PlanarRectangle::Pointer planarRectanglePrecise = mitk::PlanarRectangle::New();
    planarRectanglePrecise->SetPlaneGeometry(preciseGeometry);
    planarRectanglePrecise->PlaceFigure(p0precise);
    planarRectanglePrecise->SetCurrentControlPoint(p1precise);
    planarRectanglePrecise->GetPropertyList()->SetBoolProperty("initiallyplaced", true);
    planarFigures.emplace("planarRectanglePrecise",planarRectanglePrecise.GetPointer());

    return planarFigures;
  }

  static PlanarFigureMap CreateClonedPlanarFigures(PlanarFigureMap original)
  {
    PlanarFigureMap copiedPlanarFigures;

    for (const auto& pf : original)
    {
      mitk::PlanarFigure::Pointer copiedFigure = pf.second->Clone();

      copiedPlanarFigures[pf.first] = copiedFigure;
    }
    return copiedPlanarFigures;
  }

  static void VerifyPlanarFigures(PlanarFigureMap &referencePfs, PlanarFigureMap &testPfs)
  {
    PlanarFigureMap::iterator it1, it2;

    int i = 0;
    for (it1 = referencePfs.begin(); it1 != referencePfs.end(); ++it1)
    {
      bool planarFigureFound = false;
      int j = 0;
      for (it2 = testPfs.begin(); it2 != testPfs.end(); ++it2)
      {
        // Compare PlanarFigures (returns false if different types)
        if (ComparePlanarFigures(it1->second, it2->second))
        {
          planarFigureFound = true;
        }
        ++j;
      }

      // Test if (at least) on PlanarFigure of the first type was found in the second list
      MITK_TEST_CONDITION_REQUIRED(planarFigureFound,
                                   "Testing if " << it1->second->GetNameOfClass() << " has a counterpart " << i);
      ++i;
    }
  }

  static bool ComparePlanarFigures(const mitk::PlanarFigure *referencePf, const mitk::PlanarFigure *testPf)
  {
    // Test if PlanarFigures are of same type; otherwise return
    if (strcmp(referencePf->GetNameOfClass(), testPf->GetNameOfClass()) != 0)
    {
      return false;
    }

    if (strcmp(referencePf->GetNameOfClass(), "PlanarCross") == 0)
    {
      std::cout << "Planar Cross Found" << std::endl;
    }

    // Test for equal number of control points
    if (referencePf->GetNumberOfControlPoints() != testPf->GetNumberOfControlPoints())
    {
      return false;
    }

    // Test if all control points are equal
    for (unsigned int i = 0; i < referencePf->GetNumberOfControlPoints(); ++i)
    {
      mitk::Point2D point1 = referencePf->GetControlPoint(i);
      mitk::Point2D point2 = testPf->GetControlPoint(i);

      if (point1.EuclideanDistanceTo(point2) >= mitk::eps)
      {
        return false;
      }
    }

    // Test for equal number of properties
    typedef mitk::PropertyList::PropertyMap PropertyMap;
    const PropertyMap *refProperties = referencePf->GetPropertyList()->GetMap();
    const PropertyMap *testProperties = testPf->GetPropertyList()->GetMap();

    MITK_INFO << "List 1:";
    for (auto i1 = refProperties->begin(); i1 != refProperties->end(); ++i1)
    {
      std::cout << i1->first << std::endl;
    }

    MITK_INFO << "List 2:";
    for (auto i2 = testProperties->begin(); i2 != testProperties->end(); ++i2)
    {
      std::cout << i2->first << std::endl;
    }

    MITK_INFO << "-------";

    //remark test planar figures may have additional properties
    //(e.g. reader meta information), but they are not relevant
    //for the test. Only check of all properties of the reference
    //are present and correct.
    for (const auto& prop : *refProperties)
    {
      auto finding = testProperties->find(prop.first);
      if (finding == testProperties->end())
      {
        return false;
      }

      MITK_INFO << "Comparing " << prop.first << "(" << prop.second->GetValueAsString() << ") and " << finding->first
        << "(" << finding->second->GetValueAsString() << ")";
      // Compare property objects contained in the map entries (see mitk::PropertyList)
      if (!(*(prop.second) == *(finding->second))) return false;
    }

    // Test if Geometry is equal
    const auto *planeGeometry1 = dynamic_cast<const mitk::PlaneGeometry *>(referencePf->GetPlaneGeometry());
    const auto *planeGeometry2 = dynamic_cast<const mitk::PlaneGeometry *>(testPf->GetPlaneGeometry());

    // Test Geometry transform parameters
    typedef mitk::Geometry3D::TransformType TransformType;
    const TransformType *affineGeometry1 = planeGeometry1->GetIndexToWorldTransform();
    const TransformType::ParametersType &parameters1 = affineGeometry1->GetParameters();
    const TransformType::ParametersType &parameters2 = planeGeometry2->GetIndexToWorldTransform()->GetParameters();
    for (unsigned int i = 0; i < affineGeometry1->GetNumberOfParameters(); ++i)
    {
      if (fabs(parameters1.GetElement(i) - parameters2.GetElement(i)) >= mitk::eps)
      {
        return false;
      }
    }

    // Test Geometry bounds
    typedef mitk::Geometry3D::BoundsArrayType BoundsArrayType;
    const BoundsArrayType &bounds1 = planeGeometry1->GetBounds();
    const BoundsArrayType &bounds2 = planeGeometry2->GetBounds();
    for (unsigned int i = 0; i < 6; ++i)
    {
      if (fabs(bounds1.GetElement(i) - bounds2.GetElement(i)) >= mitk::eps)
      {
        return false;
      };
    }

    // Test Geometry spacing and origin
    mitk::Vector3D spacing1 = planeGeometry1->GetSpacing();
    mitk::Vector3D spacing2 = planeGeometry2->GetSpacing();
    if ((spacing1 - spacing2).GetNorm() >= mitk::eps)
    {
      return false;
    }

    mitk::Point3D origin1 = planeGeometry1->GetOrigin();
    mitk::Point3D origin2 = planeGeometry2->GetOrigin();

    if (origin1.EuclideanDistanceTo(origin2) >= mitk::eps)
    {
      return false;
    }
    return true;
  }

  static PlanarFigureToStreamMap SerializePlanarFiguresToMemoryBuffers(PlanarFigureMap &planarFigures)
  {
    PlanarFigureToStreamMap pfMemoryStreams;

    for (const auto& pf : planarFigures)
    {
      mitk::FileWriterRegistry writerRegistry;
      auto writers = writerRegistry.GetWriters(pf.second.GetPointer(), "");

      std::ostringstream stream;
      writers[0]->SetOutputStream("",&stream);
      writers[0]->SetInput(pf.second);
      writers[0]->Write();
      pfMemoryStreams.emplace(pf.first, stream.str());
    }

    return pfMemoryStreams;
  }

  static PlanarFigureMap DeserializePlanarFiguresFromMemoryBuffers(PlanarFigureToStreamMap pfMemoryStreams)
  {
    // Store them in the list and return it
    PlanarFigureMap planarFigures;

    mitk::FileReaderRegistry readerRegistry;
    std::vector<mitk::IFileReader*> readers =
      readerRegistry.GetReaders(mitk::FileReaderRegistry::GetMimeTypeForFile("pf"));

    for (const auto& pfStream : pfMemoryStreams)
    {
      std::istringstream stream;
      stream.str(pfStream.second);
      readers[0]->SetInput("", &stream);
      auto pfRead = readers[0]->Read();
      MITK_TEST_CONDITION(pfRead.size() == 1, "One planar figure should be read from stream.");
      auto pf = dynamic_cast<mitk::PlanarFigure*>(pfRead.front().GetPointer());
      MITK_TEST_CONDITION(pf != nullptr, "Loaded data should be a planar figure.");
      planarFigures.emplace(pfStream.first, pf);
    }

    return planarFigures;
  }

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
int mitkPlanarFigureIOTest(int /* argc */, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("PlanarFigureIO");

  // Create a number of PlanarFigure objects
  PlanarFigureIOTestClass::PlanarFigureMap originalPlanarFigures = PlanarFigureIOTestClass::CreatePlanarFigures();

  // Create a number of cloned planar figures to test the Clone function
  PlanarFigureIOTestClass::PlanarFigureMap clonedPlanarFigures =
    PlanarFigureIOTestClass::CreateClonedPlanarFigures(originalPlanarFigures);

  PlanarFigureIOTestClass::VerifyPlanarFigures(originalPlanarFigures, clonedPlanarFigures);


  std::map <const std::string, const std::string> pfFileNameMap;
  for (const auto& pf : originalPlanarFigures)
  {
    std::string filename = mitk::IOUtil::CreateTemporaryFile(pf.first+"_XXXXXX.pf", itksys::SystemTools::GetCurrentWorkingDirectory());
    mitk::IOUtil::Save(pf.second, filename);
    pfFileNameMap.emplace(pf.first, filename);
  }

  // Write PlanarFigure objects to memory buffers
  PlanarFigureIOTestClass::PlanarFigureToStreamMap writersStreams =
    PlanarFigureIOTestClass::SerializePlanarFiguresToMemoryBuffers(originalPlanarFigures);

  // Read PlanarFigure objects from temp file
  PlanarFigureIOTestClass::PlanarFigureMap retrievedPlanarFigures;
  for (const auto& files : pfFileNameMap)
  {
    auto pf = mitk::IOUtil::Load<mitk::PlanarFigure>(files.second);
    retrievedPlanarFigures.emplace(files.first, pf);
  }

  // Read PlanarFigure objects from memory buffers
  PlanarFigureIOTestClass::PlanarFigureMap retrievedPlanarFiguresFromMemory =
    PlanarFigureIOTestClass::DeserializePlanarFiguresFromMemoryBuffers(writersStreams);

  // Test if original and retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures(originalPlanarFigures, retrievedPlanarFigures);

  // Test if original and memory retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures(originalPlanarFigures, retrievedPlanarFiguresFromMemory);

  // empty the originalPlanarFigures
  originalPlanarFigures.clear();

  // Test if cloned and retrieved PlanarFigure objects are the same
  PlanarFigureIOTestClass::VerifyPlanarFigures(clonedPlanarFigures, retrievedPlanarFigures);

  MITK_TEST_END()
}
