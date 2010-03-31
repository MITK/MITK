/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <string>

#include <FL/Fl.h>
#include <FL/Fl_Window.h>
#include <FL/Fl_Box.h>
#include <Fl/Fl_File_Chooser.h>
#include "FLmitkRenderWindow/FLmitkRenderWindow.h"

// #include "mitkIpPic.h"
#include "mitkPicFileReader.h"

#include "mitkStringProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkSliceNavigationController.h"

#include "mitkDataNodeFactory.h"
#include "mitkSample.h"

int main(int argc, char **argv) {
  const char* fileName = NULL;
  if (argc == 2 && argv[1]) {
    fileName = argv[1];
  } else {
    fileName = fl_file_chooser("Open file","*.dcm;*.png;*.jog;*.tiff;*.dcm;*.DCM;*.seq;*.pic;*.pic.gz;*.seq.gz;*.pic;*.pic.gz;*.png;*.stl",NULL);
  }
  if (!fileName) { exit(0);}

  UserInterface ui; 
  mitk::SliceNavigationController::Pointer &sliceCtrl = ui.mainWid->sliceCtrl;    
  sliceCtrl = mitk::SliceNavigationController::New("navigation");
  ui.mainWid->InitRenderer();
  ui.mainWid->GetRenderer()->SetMapperID(1);
  mitk::DataTree::Pointer tree = mitk::DataTree::New();

  mitk::DataNodeFactory::Pointer factory = mitk::DataNodeFactory::New();

  factory->SetFileName( fileName );
  factory->Update();
  if (factory->GetNumberOfOutputs() > 1) {
    fl_alert("WARNING: More than one image in file. Only showing first one."); 
  }
  mitk::DataTreePreOrderIterator it(tree);
  mitk::DataNode::Pointer node = factory->GetOutput( 0 );
  assert(node.IsNotNull());
  {
    it.Add( node );
    ui.mainWid->SetNode(node);
  }
  ui.mainWid->GetRenderer()->SetData(&it);
  ui.mainWid->RequestUpdate();

  mitk::BoundingBox::Pointer bb = mitk::DataTree::ComputeVisibleBoundingBox(&it);

  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  geometry->Initialize();
  geometry->SetBounds(bb->GetBounds());

  //tell the navigator the geometry to be sliced (with geometry a Geometry3D::ConstPointer)
  sliceCtrl->SetInputWorldGeometry(geometry.GetPointer());
  //tell the navigator in which direction it shall slice the data
  sliceCtrl->SetViewDirection(mitk::SliceNavigationController::Transversal);
  //Connect one or more BaseRenderer to this navigator, i.e.: events sent 
  //by the navigator when stepping through the slices (e.g. by 
  //sliceCtrl->GetSlice()->Next()) will be received by the BaseRenderer 
  //(in this example only slice-changes, see also ConnectGeometryTimeEvent 
  //and ConnectGeometryEvents.)
  sliceCtrl->ConnectGeometrySliceEvent(ui.mainWid->GetRenderer());
  //create a world geometry and send the information to the connected renderer(s)
  sliceCtrl->Update();
  sliceCtrl->GetSlice()->SetPos(3);

  ui.sliceSlider->bounds(0,sliceCtrl->GetSlice()->GetSteps()-1);
  ui.sliceSlider->precision(0); 
  ui.mainWid->RequestUpdate();
  ui.mainWin->show(argc, argv);

  return Fl::run();
}
