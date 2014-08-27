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
#include <mitkPlanarLine.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarRectangle.h>

#include "usModuleRegistry.h"

class mitkPlanarFigureInteractionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPlanarFigureInteractionTestSuite);
MITK_TEST(AngleInteractionCreate);
  // MITK_TEST(ArrowInteractionCreate);
//MITK_TEST(BezierCurveInteractionCreate);
MITK_TEST(CircleInteractionCreate);
  // MITK_TEST(CrossInteractionCreate);
//MITK_TEST(DoubleEllipseInteractionCreate);
  //MITK_TEST(EllipseInteractionCreate);

//  MITK_TEST(PlanarFourPointAngleInteractionCreate);
//  MITK_TEST(PlanarLineInteractionCreate);
//  MITK_TEST(PlanarPolygonInteractionCreate);
  MITK_TEST(NonClosedPlanarPolygonInteractionCreate);
//  MITK_TEST(RectangleInteractionCreate);


  CPPUNIT_TEST_SUITE_END();


public:

  void setUp()
  {
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
    //mitk::Image::Pointer testImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));

    mitk::Image::Pointer testImage = mitk::IOUtil::LoadImage("/home/cweber/SSD-Data/Test/0011.dcm");

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


    mitk::PlanarFigureWriter::Pointer writer = mitk::PlanarFigureWriter::New();
    writer->SetFileName("/home/cweber/SSD-Data/Test/beurk.pf");
    writer->SetInput(figure);
    writer->Update();

    //Compare figures
    MITK_ASSERT_EQUAL(figure, reference, "Compare figure with reference");

  }

  void AngleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarAngle::New();
    //RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarAngle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarAngle_Create.pf");
    RunTest(figure, "/home/cweber/SSD-Data/Test/Angle.xml", "/home/cweber/SSD-Data/Test/Angle.pf");
  }
  // TODO:
  //void ArrowInteractionCreate()
  //{
  //  mitk::PlanarFigure::Pointer figure;
  //  figure = mitk::PlanarArrow::New();
  //  RunTest(figure, "InteractionTestDaa/Interactions/PlanarFigureInteractor_PlanarArrow_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarArrow_Create.pf");
  //}

  void BezierCurveInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarBezierCurve::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarCircle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarCircle_Create.pf");
  }

  void CircleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarCircle::New();
    //RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarCircle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarCircle_Create.pf");
    RunTest(figure, "/home/cweber/SSD-Data/Test/Circle.xml", "/home/cweber/SSD-Data/Test/Circle.pf");
  }

  // TODO:
  //void CrossInteractionCreate()
  //{
  //  mitk::PlanarFigure::Pointer figure;
  //  figure = mitk::PlanarCross::New();
  //  RunTest(figure, "InteractionTestDaa/Interactions/PlanarFigureInteractor_PlanarCross_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarCross_Create.pf");
  //}

  void DoubleEllipseInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarDoubleEllipse::New();
    RunTest(figure, "InteractionTestData/InteraName is DifferentLength/Circumferencections/PlanarFigureInteractor_PlanarDoubleEllipse_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarDoubleEllipse_Create.pf");
  }

  void EllipseInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarEllipse::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarEllipse_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarEllipse_Create.pf");
  }

  void PlanarFourPointAngleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarFourPointAngle::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarFourPointAngle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarFourPointAngle_Create.pf");
  }

  void PlanarLineInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarLine::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarLine_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarLine_Create.pf");
  }

  void PlanarPolygonInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarPolygon::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarPolygon_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarPolygon_Create.pf");
  }

  void NonClosedPlanarPolygonInteractionCreate()
  {
    mitk::PlanarPolygon::Pointer figure;
    figure = mitk::PlanarPolygon::New();
    figure->ClosedOff();
    //RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_NonClosedPlanarPolygon_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_NonClosedPlanarPolygon_Create.pf");
    RunTest(figure.GetPointer(), "/home/cweber/SSD-Data/Test/Path.xml", "/home/cweber/SSD-Data/Test/Path.pf");
  }

  void RectangleInteractionCreate()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarRectangle::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarRectangle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarRectangle_Create.pf");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureInteraction)
