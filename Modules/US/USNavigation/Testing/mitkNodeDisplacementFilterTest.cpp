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

#include <mitkTestingMacros.h>
#include "mitkNodeDisplacementFilter.h"
#include "mitkPointSet.h"
#include "mitkSurface.h"

class mitkNodeDisplacementFilterTestClass
{
public:

  /*
  * \brief Returns a reference string for serialized calibrations.
  */

  static void TestNodeDisplacement()
  {
    /* create helper objects: navigation data with data valid */
    mitk::NavigationData::PositionType initialPos;
    mitk::FillVector3D(initialPos, 1.0, 2.0, 3.0);
    mitk::ScalarType initialError(22.22);
    mitk::NavigationData::Pointer nd0 = mitk::NavigationData::New();
    nd0->SetPosition(initialPos);
    nd0->SetPositionAccuracy(initialError);
    nd0->SetDataValid(true);
    nd0->SetName("testName");

    mitk::NodeDisplacementFilter::Pointer nodeDisplacementFilter = mitk::NodeDisplacementFilter::New();
    nodeDisplacementFilter->SetInput(nd0);
    nodeDisplacementFilter->SelectInput(0);


    // test translation
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    mitk::Surface::Pointer surface = mitk::Surface::New();
    mitk::Point3D origin;
    mitk::FillVector3D(origin, 5.0, 3.0, 6.0);
    surface->GetGeometry()->SetOrigin(origin);
    node->SetData(surface->Clone());
    MITK_TEST_CONDITION(nodeDisplacementFilter->AddNode(node), "Adding node to node displacement filter.");

    nodeDisplacementFilter->Modified();
    nodeDisplacementFilter->Update();
    MITK_TEST_CONDITION(mitk::Equal(*(surface->GetGeometry()), *(node->GetData()->GetGeometry()), mitk::eps, true),
                        "Geometry must not have changed as the reference point is still at the same position.")

    mitk::NavigationData::PositionType movedPos;
    mitk::FillVector3D(movedPos, -3.0, 6.0, 3.23);
    nd0->SetPosition(movedPos);

    nodeDisplacementFilter->Modified();
    nodeDisplacementFilter->Update();

    surface->GetGeometry()->SetOrigin(surface->GetGeometry()->GetOrigin()+(movedPos-initialPos));
    MITK_TEST_CONDITION(mitk::Equal(*(surface->GetGeometry()), *(node->GetData()->GetGeometry()), mitk::eps, true),
                        "Node must have moved accordingly to the reference node.")


    // test rotation
    mitk::DataNode::Pointer node2 = mitk::DataNode::New();
    mitk::Surface::Pointer surface2 = mitk::Surface::New();
    mitk::FillVector3D(origin, 0, 0, 0);
    surface->GetGeometry()->SetOrigin(origin);
    node2->SetData(surface2->Clone());
    MITK_TEST_CONDITION(nodeDisplacementFilter->AddNode(node2), "Adding second node to node displacement filter.");


    mitk::NavigationData::OrientationType rotation(0.1, 0.2, 0.3, 0.4);
    nd0->SetOrientation(rotation);

    mitk::AffineTransform3D::Pointer navigationDataTransform = nd0->GetAffineTransform3D();

    nodeDisplacementFilter->Modified();
    nodeDisplacementFilter->Update();

    MITK_TEST_CONDITION(mitk::Equal(*navigationDataTransform, *(node2->GetData()->GetGeometry()->GetIndexToWorldTransform()), mitk::eps, true),
                        "Rotation of the node must be the same as the rotation of the navigation data.")
  }
};

/**
* This function is testing methods of the class USDevice.
*/
int mitkNodeDisplacementFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkNodeDisplacementFilterTest");

  mitkNodeDisplacementFilterTestClass::TestNodeDisplacement();

  MITK_TEST_END();
}
