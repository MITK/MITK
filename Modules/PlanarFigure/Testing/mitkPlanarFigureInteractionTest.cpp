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
#include <mitkPlanarRectangle.h>
#include <mitkPlanarCircle.h>

#include "usModuleRegistry.h"
#include "mitkPlanarFigureReader.h"
#include "mitkPlanarFigureWriter.h"



class mitkPlanarFigureInteractionTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPlanarFigureInteractionTestSuite);
  MITK_TEST(RectangleInteraction);
  MITK_TEST(CircleInteraction);
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
    mitk::Image::Pointer testImage = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
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

  void RectangleInteraction()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarRectangle::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarRectangle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarRectangle_Create.pf");
  }

  void CircleInteraction()
  {
    mitk::PlanarFigure::Pointer figure;
    figure = mitk::PlanarCircle::New();
    RunTest(figure, "InteractionTestData/Interactions/PlanarFigureInteractor_PlanarCircle_Create.xml", "InteractionTestData/ReferenceData/PlanarFigureInteractor_PlanarCircle_Create.pf");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureInteraction)
