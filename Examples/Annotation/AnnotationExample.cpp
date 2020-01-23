/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"

#include <mitkStandaloneDataStorage.h>

#include <QApplication>
#include <itksys/SystemTools.hxx>

#include <mitkAnnotationUtils.h>
#include <mitkLayoutAnnotationRenderer.h>
#include <mitkManualPlacementAnnotationRenderer.h>
#include <mitkTextAnnotation2D.h>
#include <mitkTextAnnotation3D.h>

#include <mitkIOUtil.h>

//##Documentation
//## @brief Load image (nrrd format) and display it in a 2D view
int main(int argc, char *argv[])
{
  QApplication qtapplication(argc, argv);

  if (argc < 2)
  {
    fprintf(stderr, "Usage:   %s [filename] \n\n", itksys::SystemTools::GetFilenameName(argv[0]).c_str());
    return 1;
  }

  // Register Qmitk-dependent global instances
  QmitkRegisterClasses();

  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  // Load datanode (eg. many image formats, surface formats, etc.)
  mitk::IOUtil::Load(argv[1], *ds);

  // Create a RenderWindow
  QmitkRenderWindow renderWindow;

  // Tell the RenderWindow which (part of) the datastorage to render
  renderWindow.GetRenderer()->SetDataStorage(ds);

  // Initialize the RenderWindow
  auto geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);
  // mitk::RenderingManager::GetInstance()->InitializeViews();

  // Add Overlays
  //![TextAnnotation2D]
  // Create a textAnnotation2D
  mitk::TextAnnotation2D::Pointer textAnnotation = mitk::TextAnnotation2D::New();

  textAnnotation->SetText("Test!"); // set UTF-8 encoded text to render
  textAnnotation->SetFontSize(40);
  textAnnotation->SetColor(1, 0, 0); // Set text color to red
  textAnnotation->SetOpacity(1);

  // The position of the Annotation can be set to a fixed coordinate on the display.
  mitk::Point2D pos;
  pos[0] = 10;
  pos[1] = 20;
  textAnnotation->SetPosition2D(pos);

  std::string rendererID = renderWindow.GetRenderer()->GetName();

  // The LayoutAnnotationRenderer can place the TextAnnotation2D at some defined corner positions
  mitk::LayoutAnnotationRenderer::AddAnnotation(
    textAnnotation, rendererID, mitk::LayoutAnnotationRenderer::TopLeft, 5, 5, 1);
  //![TextAnnotation2D]

  //![TextAnnotation3D]
  mitk::PointSet::Pointer pointset = mitk::PointSet::New();

  // This vector is used to define an offset for the annotations, in order to show them with a margin to the actual
  // coordinate.
  mitk::Point3D offset;
  offset[0] = .5;
  offset[1] = .5;
  offset[2] = .5;

  // save references to Annotations so that they do not get deregistered
  std::vector<mitk::TextAnnotation3D::Pointer> annotationReferences;

  // Just a loop to create some points
  for (unsigned long i = 0; i < 10; i++)
  {
    // To each point, a TextAnnotation3D is created
    mitk::TextAnnotation3D::Pointer textAnnotation3D = mitk::TextAnnotation3D::New();
    mitk::Point3D point;
    point[0] = i * 20;
    point[1] = i * 30;
    point[2] = i * -50;
    pointset->InsertPoint(i, point);
    textAnnotation3D->SetText("A Point");

    // The Position is set to the point coordinate to create an annotation to the point in the PointSet.
    textAnnotation3D->SetPosition3D(point);

    // move the annotation away from the actual point
    textAnnotation3D->SetOffsetVector(offset);

    annotationReferences.push_back(textAnnotation3D);
    mitk::ManualPlacementAnnotationRenderer::AddAnnotation(textAnnotation3D, rendererID);
  }

  // Get the MicroserviceID of the registered textAnnotation
  std::string serviceID = textAnnotation->GetMicroserviceID();

  // The AnnotationUtils can retrieve any registered annotations by their microservice ID
  mitk::Annotation *annotation = mitk::AnnotationUtils::GetAnnotation(serviceID);
  // This way, it is possible to change the properties of Annotations without knowing their implementation
  annotation->SetText("changed text!");

  // also show the created pointset
  mitk::DataNode::Pointer datanode = mitk::DataNode::New();
  datanode->SetData(pointset);
  datanode->SetName("pointSet");
  ds->Add(datanode);
  //! [TextAnnotation3D]
  renderWindow.show();
  renderWindow.resize(256, 256);

  renderWindow.show();
  renderWindow.resize(256, 256);

  // cleanup: Remove References to DataStorage. This will delete the object
  ds = nullptr;
}
