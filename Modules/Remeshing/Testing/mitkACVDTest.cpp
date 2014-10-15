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

#include <mitkACVD.h>
#include <mitkIOUtil.h>
#include <mitkTestingMacros.h>
#include <vtkDebugLeaks.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <sstream>

#define _MITK_TEST_FOR_EXCEPTION(STATEMENT, EXCEPTION, MESSAGE) \
  MITK_TEST_OUTPUT_NO_ENDL(<< MESSAGE) \
  try \
  { \
    STATEMENT; \
    MITK_TEST_OUTPUT(<< " [FAILED]") \
    mitk::TestManager::GetInstance()->TestFailed(); \
  } \
  catch (const EXCEPTION& e) \
  { \
    MITK_TEST_OUTPUT(<< "\n  " << e.GetDescription() << " [PASSED]") \
    mitk::TestManager::GetInstance()->TestPassed(); \
  }

template <typename T>
static T lexical_cast(const std::string& string)
{
  std::istringstream sstream(string);
  T value;

  sstream >> value;

  if (sstream.fail())
  {
    MITK_ERROR << "Lexical cast failed for '" << string << "'!";
    exit(EXIT_FAILURE);
  }

  return value;
}

static void Remesh_SurfaceIsNull_ThrowsException()
{
  mitk::Surface::ConstPointer surface;
  _MITK_TEST_FOR_EXCEPTION(mitk::ACVD::Remesh(surface, 0, 100, 0.0), mitk::Exception, "Remesh_SurfaceIsNull_ThrowsException")
}

static void Remesh_PolyDataIsNull_ThrowsException()
{
  mitk::Surface::ConstPointer surface = mitk::Surface::New().GetPointer();
  _MITK_TEST_FOR_EXCEPTION(mitk::ACVD::Remesh(surface, 0, 100, 0.0), mitk::Exception, "Remesh_PolyDataIsNull_ThrowsException")
}

static void Remesh_SurfaceDoesNotHaveDataAtTimeStep_ThrowsException()
{
  mitk::Surface::ConstPointer surface = mitk::Surface::New().GetPointer();
  _MITK_TEST_FOR_EXCEPTION(mitk::ACVD::Remesh(surface, 1, 100, 0.0), mitk::Exception, "Remesh_SurfaceDoesNotHaveDataAtTimeStep_ThrowsException")
}

static void Remesh_SurfaceHasNoPolygons_ThrowsException()
{
  mitk::Surface::Pointer surface = mitk::Surface::New();
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  surface->SetVtkPolyData(polyData);
  _MITK_TEST_FOR_EXCEPTION(mitk::ACVD::Remesh(surface.GetPointer(), 0, 100, 0.0), mitk::Exception, "Remesh_SurfaceHasNoPolygons_ThrowsException")
}

static void Remesh_SurfaceIsValid_ReturnsRemeshedSurface(const std::string& filename, unsigned int t, int numVertices, double gradation, int subsampling, double edgeSplitting, int optimizationLevel, bool forceManifold, bool boundaryFixing)
{
  mitk::Surface::ConstPointer surface = mitk::IOUtil::LoadSurface(filename).GetPointer();
  mitk::Surface::Pointer remeshedSurface = mitk::ACVD::Remesh(surface, t, numVertices, gradation, subsampling, edgeSplitting, optimizationLevel, forceManifold, boundaryFixing);
  MITK_TEST_CONDITION(remeshedSurface.IsNotNull() && remeshedSurface->GetVtkPolyData() != NULL && remeshedSurface->GetVtkPolyData()->GetNumberOfPolys() != 0, "Remesh_SurfaceIsValid_ReturnsRemeshedSurface")
}

int mitkACVDTest(int argc, char* argv[])
{
  if (argc != 10)
  {
    MITK_ERROR << "Invalid argument count!\n"
               << "Usage: mitkACVDTest <filename> <t> <numVertices> <gradation> <subsampling>\n"
               << "                    <edgeSplitting> <optimizationLevel> <forceManifiold>\n"
               << "                    <boundaryFixing>\n"
               << "  See MITK API documentation of mitk::ACVD::Remesh() for details.";

    return EXIT_FAILURE;
  }

  const std::string filename = argv[1];
  const unsigned int t = lexical_cast<unsigned int>(argv[2]);
  const int numVertices = lexical_cast<int>(argv[3]);
  const double gradation = lexical_cast<double>(argv[4]);
  const int subsampling = lexical_cast<int>(argv[5]);
  const double edgeSplitting = lexical_cast<double>(argv[6]);
  const int optimizationLevel = lexical_cast<int>(argv[7]);
  const bool forceManifold = lexical_cast<bool>(argv[8]);
  const bool boundaryFixing = lexical_cast<bool>(argv[9]);

  MITK_TEST_BEGIN("mitkACVDTest")

    vtkDebugLeaks::SetExitError(0);

    Remesh_SurfaceIsNull_ThrowsException();
    Remesh_PolyDataIsNull_ThrowsException();
    Remesh_SurfaceDoesNotHaveDataAtTimeStep_ThrowsException();
    Remesh_SurfaceHasNoPolygons_ThrowsException();

    Remesh_SurfaceIsValid_ReturnsRemeshedSurface(filename, t, numVertices, gradation, subsampling, edgeSplitting, optimizationLevel, forceManifold, boundaryFixing);

  MITK_TEST_END()
}
