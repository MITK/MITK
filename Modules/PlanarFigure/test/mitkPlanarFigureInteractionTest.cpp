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
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>

#include <mitkIOUtil.h>
#include <mitkInteractionTestHelper.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarFigureReader.h>
#include <mitkPlanarFigureWriter.h>

#include <mitkPlanarAngle.h>
#include <mitkPlanarArrow.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarEllipse.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarSubdivisionPolygon.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>

#include <vtkDebugLeaks.h>

#include "usModuleRegistry.h"

class mitkPlanarFigureInteractionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPlanarFigureInteractionTestSuite);
  MITK_TEST(AngleInteractionCreate);
  MITK_TEST(BezierCurveInteractionCreate);
  MITK_TEST(CircleInteractionCreate);
  MITK_TEST(DoubleEllipseInteractionCreate);
  MITK_TEST(PlanarFourPointAngleInteractionCreate);
  MITK_TEST(PlanarLineInteractionCreate);
  MITK_TEST(PlanarPolygonInteractionCreate);
  MITK_TEST(NonClosedPlanarPolygonInteractionCreate);
  MITK_TEST(RectangleInteractionCreate);
  MITK_TEST(PlanarSubdivisionInteractionCreate);

  CPPUNIT_TEST_SUITE_END();


public:

  void setUp()
  {
    /// \todo Fix leaks of vtkObjects. Bug 18095.
    vtkDebugLeaks::SetExitError(0);
  }

  void tearDown()
  {
  }

  void RunTest(mitk::PlanarFigure::Pointer figure, std::string interactionXmlPath, std::string referenceFigurePath)
  {
    mitk::DataNode::Pointer node;
    mitk::PlanarFigureInteractor::Pointer figureInteractor;

    //Create DataNode as a container for our PlanarFigure
    node = mitk::DataNode::New();
    node->SetData(figure);

    mitk::InteractionTestHelper interactionTestHelper(GetTestDataFilePath(interactionXmlPath));


    //Load a bounding image
    mitk::Image::Pointer testImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("InteractionTestData/InputData/scaledSingleSlice.nrrd"));

    mitk::DataNode::Pointer dn = mitk::DataNode::New();
    dn->SetData(testImage);
    interactionTestHelper.AddNodeToStorage(dn);
    interactionTestHelper.GetDataStorage()->Add(node, dn);


    node->SetName("PLANAR FIGURE");
    // set as selected
    node->SetSelected(true);
    node->AddProperty("selected", mitk::BoolProperty::New(true));

    //Load state machine
    figureInteractor = mitk::PlanarFigureInteractor::New();
    us::Module* planarFigureModule = us::ModuleRegistry::GetModule("MitkPlanarFigure");
    figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
    figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
    figureInteractor->SetDataNode( node );


    //Start Interaction
    interactionTestHelper.PlaybackInteraction();

    //Load reference PlanarFigure
    mitk::PlanarFigureReader::Pointer reader = mitk::PlanarFigureReader::New();
    reader->SetFileName(GetTestDataFilePath(referenceFigurePath));
    reader->Update();
    mitk::PlanarFigure::Pointer reference = reader->GetOutput(0);

    //Compare figures
    MITK_ASSERT_EQUAL(figure, reference, "Compare figure with reference");
  }

  void AngleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarAngle::New();
    RunTest(figure, "InteractionTestData/Interactions/Angle.xml", "InteractionTestData/ReferenceData/Angle.pf");
  }

  void BezierCurveInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarBezierCurve::New();
    RunTest(figure, "InteractionTestData/Interactions/Bezier.xml", "InteractionTestData/ReferenceData/Bezier.pf");
  }

  void CircleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarCircle::New();
    RunTest(figure, "InteractionTestData/Interactions/Circle.xml", "InteractionTestData/ReferenceData/Circle.pf");
  }

  void DoubleEllipseInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarDoubleEllipse::New();
    RunTest(figure, "InteractionTestData/Interactions/DoubleEllipse.xml", "InteractionTestData/ReferenceData/DoubleEllipse.pf");
  }

  void PlanarSubdivisionInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarSubdivisionPolygon::New();
    RunTest(figure, "InteractionTestData/Interactions/SubDivision.xml", "InteractionTestData/ReferenceData/SubDivision.pf");
  }

  void PlanarFourPointAngleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarFourPointAngle::New();
    RunTest(figure, "InteractionTestData/Interactions/Planar4PointAngle.xml", "InteractionTestData/ReferenceData/Planar4PointAngle.pf");
  }

  void PlanarLineInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarLine::New();
    RunTest(figure, "InteractionTestData/Interactions/Line.xml", "InteractionTestData/ReferenceData/Line.pf");
  }

  void PlanarPolygonInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarPolygon::New();
    RunTest(figure, "InteractionTestData/Interactions/Polygon.xml", "InteractionTestData/ReferenceData/Polygon.pf");
  }

  void NonClosedPlanarPolygonInteractionCreate()
  {
    mitk::PlanarPolygon::Pointer figure;
    figure = mitk::PlanarPolygon::New();
    figure->ClosedOff();
    RunTest(figure.GetPointer(), "InteractionTestData/Interactions/Path.xml", "InteractionTestData/ReferenceData/Path.pf");
  }

  void RectangleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarRectangle::New();
    RunTest(figure, "InteractionTestData/Interactions/Rectangle.xml", "InteractionTestData/ReferenceData/Rectangle.pf");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureInteraction)
