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

#include "usModuleRegistry.h"
#include "mitkPlanarFigureReader.h"
#include "mitkPlanarFigureWriter.h"



class mitkPlanarFigureMeasurementTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPlanarFigureMeasurementTestSuite);
  MITK_TEST(MeasurePixelSquare);
  CPPUNIT_TEST_SUITE_END();


private:
  mitk::DataNode::Pointer m_Node;
  mitk::PlanarFigure::Pointer m_Figure;
  mitk::PlanarFigureInteractor::Pointer m_FigureInteractor;


public:

  void setUp()
  {
    //Add our test node to the DataStorage of our test helper

    //Create test helper to initialize all necessary objects for interaction
  }

  void tearDown()
  {
    m_Node = NULL;
    m_Figure = NULL;
    m_FigureInteractor = NULL;
  }

  void MeasurePixelSquare()
  {

    //Create DataNode as a container for our Planar Figure to be tested
    m_Node = mitk::DataNode::New();
    m_Figure = mitk::PlanarRectangle::New();
    m_Node->SetData(m_Figure);

    std::string interactionXmlPath = "C:\\tmp\\interaction.xml";

    mitk::InteractionTestHelper interactionTestHelper(interactionXmlPath);

    std::string dicomPath = "C:\\tmp\\toy.nrrd";
    //Load the reference PointSet
    mitk::Image::Pointer testImage = mitk::IOUtil::LoadImage(dicomPath);
    mitk::DataNode::Pointer dn = mitk::DataNode::New();
    dn->SetData(testImage);
    interactionTestHelper.AddNodeToStorage(dn);
    interactionTestHelper.GetDataStorage()->Add(m_Node, dn);


    m_Node->SetName("PLANAR FIGURE");
    // set as selected
    m_Node->SetSelected( true );
    m_Node->AddProperty( "selected", mitk::BoolProperty::New(true) );

    m_FigureInteractor = mitk::PlanarFigureInteractor::New();

    us::Module* planarFigureModule = us::ModuleRegistry::GetModule( "MitkPlanarFigure" );
    m_FigureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
    m_FigureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
    m_FigureInteractor->SetDataNode( m_Node );


    //Start Interaction
    interactionTestHelper.PlaybackInteraction();

    mitk::PlanarFigureWriter::Pointer writer = mitk::PlanarFigureWriter::New();
    writer->SetFileName("C:\\tmp\\result.pf");
    writer->SetInput(m_Figure);
    writer->Update();

    mitk::PlanarFigureReader::Pointer reader = mitk::PlanarFigureReader::New();
    reader->SetFileName("C:\\tmp\\ref.pf");
    reader->Update();
    mitk::PlanarFigure::Pointer reference = reader->GetOutput(0);
    MITK_ASSERT_EQUAL(m_Figure, reference, "Compare figure with reference");

  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureMeasurement)
