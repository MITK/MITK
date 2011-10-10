/*
 *  mitkFiberBundleMapper2D.cpp
 *  mitk-all
 *
 *  Created by HAL9000 on 1/17/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "mitkFiberBundleMapper2D.h"
#include <mitkBaseRenderer.h>
#include "mitkFiberBundleMapper3D.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
//#include <vtkPropAssembly.h>

//#include <vtkPainterPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkLookupTable.h>
#include <vtkPoints.h>
#include <vtkCamera.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkCellArray.h>


#include <mitkPlaneGeometry.h>
#include <mitkSliceNavigationController.h>

mitk::FiberBundleMapper2D::FiberBundleMapper2D()
{
}

mitk::FiberBundleMapper2D::~FiberBundleMapper2D()
{
}


const mitk::FiberBundle* mitk::FiberBundleMapper2D::GetInput()
{
  return dynamic_cast<const mitk::FiberBundle * > ( GetData() );
}



void mitk::FiberBundleMapper2D::Update(mitk::BaseRenderer * renderer)
{



  if ( !this->IsVisible( renderer ) )
  {
    return;
  }

//  const mitk::FiberBundle::Pointer data  = const_cast< mitk::FiberBundle* > ( this->GetInput() );
//  if ( data == NULL )
//  {
//    return;
//  }

  // Calculate time step of the input data for the specified renderer (integer value)
  // this method is implemented in mitkMapper
  this->CalculateTimeStep( renderer );

  //check if updates occured in the node or on the display
  FBLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);
  const DataNode *node = this->GetDataNode();
  if ( (localStorage->m_LastUpdateTime < node->GetMTime())
      || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
      || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime()) )
  {
    MITK_INFO << "UPDATE NEEDED FOR _ " << renderer->GetName();
    this->GenerateDataForRenderer( renderer );
  }

  if ((localStorage->m_LastUpdateTime < renderer->GetDisplayGeometry()->GetMTime()) ) //was the display geometry modified? e.g. zooming, panning)
  {
    MITK_INFO << "update clipping range only:";
    //get information about current position of views
    mitk::SliceNavigationController::Pointer sliceContr = renderer->GetSliceNavigationController();
    mitk::PlaneGeometry::ConstPointer planeGeo = sliceContr->GetCurrentPlaneGeometry();

    //
     vtkCamera* camera = renderer->GetVtkRenderer()->GetActiveCamera();
    double *focPnt = camera->GetFocalPoint();
    double *camPos = camera->GetPosition();
    double dist1 = sqrt( pow( (focPnt[0]-camPos[0]), 2.0) +  pow( (focPnt[1]-camPos[1]), 2.0) + pow( (focPnt[2]-camPos[2]), 2.0));
    MITK_INFO << "DIST for CLIPPING RANGE: " << dist1;
    camera->SetClippingRange( (dist1-1.0),(dist1+1.0) );
    double cl1 =  camera->GetClippingRange()[0];
    double cl2 = camera->GetClippingRange()[1];
    MITK_INFO << "Camera clipping range: " << cl1 << " " << cl2;



  }








}


// ALL RAW DATA FOR VISUALIZATION IS GENERATED HERE.
// vtkActors and Mappers are feeded here
void mitk::FiberBundleMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  MITK_INFO << "FiberBundlemapper2D _ GENERATE DATA";

  //the handler of local storage gets feeded in this method with requested data for related renderwindow
  FBLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

//  if ( !this->IsVisible( renderer ) )
//  {
//    return;
//  }
//
  //this procedure is depricated,
  //not needed after initializaton anymore
  mitk::DataNode::ConstPointer node = this->GetDataNode();
  if ( node.IsNull() )
  {
    MITK_INFO << "check DATANODE: ....[Fail] ";
    return;
  }
  ///////////////////////////////////

  mitk::FiberBundleMapper3D::Pointer FBMapper3D = dynamic_cast< mitk::FiberBundleMapper3D* > (node->GetMapper( 2 ));
  if ( FBMapper3D->GetInput() == NULL )
  {
    MITK_INFO << "check FBMapper3D Input: ....[Fail] ";
    return;
  }


  //get information about current position of views
//  mitk::SliceNavigationController::Pointer sliceContr = renderer->GetSliceNavigationController();
//  mitk::PlaneGeometry::ConstPointer planeGeo = sliceContr->GetCurrentPlaneGeometry();
//
//
//  //generate according cutting planes based on the view position
//  double planeN[3], planeOrigin[3], cutParams[3];
////
//  planeN[0] = planeGeo->GetNormal()[0];
//  planeN[1] = planeGeo->GetNormal()[1];
//  planeN[2] = planeGeo->GetNormal()[2];
//
//  planeOrigin[0] = planeGeo->GetOrigin()[0];
//  planeOrigin[1] = planeGeo->GetOrigin()[1];
//  planeOrigin[2] = planeGeo->GetOrigin()[2];
////
//  MITK_INFO << "renWinNAME: " << renderer->GetName();
//  MITK_INFO << "PlaneNormal: " << planeN[0] << " | " << planeN[1] << " | " << planeN[2];
//  MITK_INFO << "PlaneOrigin: " << planeOrigin[0] << " | " << planeOrigin[1] << " | " << planeOrigin[2];
//
//
//  //cutParams[0] = this->GetDataNode()->GetPropertyValue("ColorCoding", nodeCC);;

  //feed cutter with parameters gathered from GUI
//  int numSlices;
//  bool existsPropNS = this->GetDataNode()->GetPropertyValue("2DPlanes", numSlices);
//  if ( !existsPropNS ){
//    return;
//  }
//
//  int distFront;
//  bool existsPropDF = this->GetDataNode()->GetPropertyValue("2DFront", distFront);
//  if ( !existsPropDF ){
//    return;
//  }
//
//  int distBackDB;
//  bool existsProp = this->GetDataNode()->GetPropertyValue("2DBack", distBackDB);
//  if ( !existsProp ){
//    return;
//  }
//
//
//  cutParams[0] = numSlices;
//  cutParams[1] = distFront;
//  cutParams[2] = distBackDB;




  //feed local storage with data we want to visualize
//  localStorage->m_SlicedResult = FBMapper3D->GetCut(planeOrigin, planeN, cutParams);
 // localStorage->m_SlicedResult = FBMapper3D->GetVtkFBPolyDataMapper();
  localStorage->m_SlicedResult = (vtkPolyData*) FBMapper3D->getVtkFiberBundleMapper()->GetInputAsDataSet();
  MITK_INFO << renderer->GetName() << " OutputPoints#: " << localStorage->m_SlicedResult->GetNumberOfPoints();


  vtkLookupTable *lut = vtkLookupTable::New();
  lut->Build();
  localStorage->m_PointMapper->SetScalarModeToUsePointFieldData();
  //m_VtkFiberDataMapperGL->SelectColorArray("FaColors");
  localStorage->m_PointMapper->SelectColorArray("ColorValues");
  localStorage->m_PointMapper->SetLookupTable(lut);  //apply the properties after the slice was set
  this->ApplyProperties( renderer );

  //setup the camera according to the actor with zooming/panning etc.
 // this->AdjustCamera( renderer, planeGeo );

  //get distance from camera to focal point


  vtkCamera* camera = renderer->GetVtkRenderer()->GetActiveCamera();
  double *focPnt = camera->GetFocalPoint();
  double *camPos = camera->GetPosition();
  double dist1 = sqrt( pow( (focPnt[0]-camPos[0]), 2.0) +  pow( (focPnt[1]-camPos[1]), 2.0) + pow( (focPnt[2]-camPos[2]), 2.0));
  MITK_INFO << "DIST for CLIPPING RANGE: " << dist1;
  camera->SetClippingRange( (dist1-1.0),(dist1+1.0) );
  double cl1 =  camera->GetClippingRange()[0];
  double cl2 = camera->GetClippingRange()[1];
  MITK_INFO << "Camera clipping range: " << cl1 << " " << cl2;

  //  feed the vtk fiber mapper with point data ...TODO do in constructor
  localStorage->m_PointMapper->SetInput(localStorage->m_SlicedResult);   // in optimized version, mapper is feeded by localStorage->m_cutter->GetOutput();
  localStorage->m_PointActor->SetMapper(localStorage->m_PointMapper);




  // We have been modified => save this for next Update()
  localStorage->m_LastUpdateTime.Modified();

}


vtkProp* mitk::FiberBundleMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{

  //MITK_INFO << "FiberBundleMapper2D GetVtkProp(renderer)";
  this->Update(renderer);
  return m_LSH.GetLocalStorage(renderer)->m_PointActor;

}

void mitk::FiberBundleMapper2D::AdjustCamera(mitk::BaseRenderer* renderer, mitk::PlaneGeometry::ConstPointer planeGeo )
{


  //activate parallel projection for 2D
  renderer->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(true);

  //get the display geometry of the current renderer.
  const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();

  //get hight and width of current point set
  double fibMapHeightInMM = 0.0;
  double displayHeightInMM = displayGeometry->GetSizeInMM()[1]; //the display height in mm (gets smaller when you zoom in)
  double zoomFactor = 0.0;

//============ ugly hack start ================================
  // ugly hack...better do it in generate data and use normal of plane or view to get requested dimensions
  // and set 2Ddimension in local storage

  std::string renWinName = renderer->GetName();
  std::string renWinTransveral = "stdmulti.widget1";
  std::string renWinSaggital = "stdmulti.widget2";
  std::string renWinCoronal = "stdmulti.widget3";

  double cameraUp[3];


  double fibmapspacing = 0.0;
  double fibmapsize = 0.0;

  double cameraPosition[3];

//


  if (renWinName == renWinTransveral) {
    MITK_INFO << renWinTransveral << "----use Z (width) and Y (height) ---";
// origin of Y = -106.737
    fibmapspacing = 2.5;
    fibmapsize = 82;
    cameraUp[0] = 0.0;
    cameraUp[1] = 1.0;
    cameraUp[2] = 0.0;
    cameraPosition[0] = -1.25;
    cameraPosition[1] = 454.0;
    cameraPosition[2] = 487.0; //Reason for 900000: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.
//renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition( cameraPosition );

  } else if (renWinName == renWinSaggital) {
    MITK_INFO << renWinSaggital << "--use X (width) and Z (height) ------";
    fibmapspacing = 2.5;
    fibmapsize = 40;
    cameraUp[0] = 0.0;
    cameraUp[1] = 0.0;
    cameraUp[2] = 1.0;
    cameraPosition[0] = 600;
    cameraPosition[1] = 16.0;
    cameraPosition[2] = 43.0; //Reason for 900000: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.
    //renderer->GetVtkRenderer()->GetActiveCamera()->Elevation(2.0);
    //renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition( cameraPosition );

  } else if (renWinName == renWinCoronal) {
    MITK_INFO << renWinCoronal << "--use X (width) and Y (height)------";
    fibmapspacing = 2.5;
    fibmapsize = 82;
    cameraUp[0] = 0.0;
    cameraUp[1] = 0.0;
    cameraUp[2] = 1.0;
    cameraPosition[0] = -1.25;
    cameraPosition[1] = -644.0;
    cameraPosition[2] = 43.0; //Reason for 900000: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.
    //renderer->GetVtkRenderer()->GetActiveCamera()->SetPosition( cameraPosition );
  }

  fibMapHeightInMM = fibmapspacing * fibmapsize;
//========== ugly hack end ==================================

  zoomFactor = fibMapHeightInMM / displayHeightInMM;

  Vector2D displayGeometryOriginInMM = displayGeometry->GetOriginInMM();  //top left of the render window (Origin)
  Vector2D displayGeometryCenterInMM = displayGeometryOriginInMM + displayGeometry->GetSizeInMM()*0.5; //center of the render window: (Origin + Size/2)



  mitk::Point2D dispCenterMM;
  dispCenterMM[0] = displayGeometryCenterInMM[0];
  dispCenterMM[1] = displayGeometryCenterInMM[1];

  mitk::Point3D dispCenter3D;
  //map diplay-geo to centerpoint
  displayGeometry->Map(dispCenterMM, dispCenter3D);

  Vector3D normal, dispCenter3Dv;
  normal[0] = planeGeo->GetNormal()[0];
  normal[1] = planeGeo->GetNormal()[1];
  normal[2] = planeGeo->GetNormal()[2];
  normal.Normalize();

  double focalPoint[3];
  focalPoint[0] = dispCenter3D[0];
  focalPoint[1] =  dispCenter3D[1];
  focalPoint[2] =  dispCenter3D[2];

  dispCenter3Dv[0] = dispCenter3D[0];
  dispCenter3Dv[1] = dispCenter3D[1];
  dispCenter3Dv[2] = dispCenter3D[2];

  //Vector3D camPos =  dispCenter3Dv * normal;
 // normal * dispCenter3D ;


  cameraPosition[0] = dispCenter3D[0] + 500 * normal[0];
  cameraPosition[1] = dispCenter3D[1] + 500 * normal[1];
  cameraPosition[2] = dispCenter3D[2] + 500 * normal[2];





  //calculate distance
  //po§sition = mittelpunkt + distanz * normierte_normale

  //Scale the rendered object:
  //The image is scaled by a single factor, because in an orthographic projection sizes
  //are preserved (so you cannot scale X and Y axis with different parameters). The
  //parameter sets the size of the total display-volume. If you set this to the image
  //height, the image plus a border with the size of the image will be rendered.
  //Therefore, the size is imageHeightInMM / 2.
  renderer->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(fibMapHeightInMM*0.5 );
  //zooming with the factor calculated by dividing displayHeight through imegeHeight. The factor is inverse, because the VTK zoom method is working inversely.
  renderer->GetVtkRenderer()->GetActiveCamera()->Zoom(zoomFactor);


    //set the camera corresponding to the textured plane
  vtkSmartPointer<vtkCamera> camera = renderer->GetVtkRenderer()->GetActiveCamera();
  if (camera)
  {
    camera->SetPosition( cameraPosition ); //set the camera position on the textured plane normal (in our case this is the view plane normal)
    camera->SetFocalPoint( focalPoint ); //set the focal point to the center of the textured plane
    camera->SetViewUp( cameraUp ); //set the view-up for the camera
    camera->SetClippingRange(0.1, 1000000.0);
    //Reason for huge range: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.

    MITK_INFO << "CamPos: " << camera->GetPosition()[0] << " "<< camera->GetPosition()[1] << " "<< camera->GetPosition()[2];
    MITK_INFO << "CamFoc: " << camera->GetFocalPoint()[0] << " "<< camera->GetFocalPoint()[1] << " "<< camera->GetFocalPoint()[2];
    MITK_INFO << "upVec: " << camera->GetViewUp()[0] << " " << camera->GetViewUp()[1] << " " << camera->GetViewUp()[2];

  }



}

// this method prepares data for VTK mapping and rendering
void mitk::FiberBundleMapper2D::ApplyProperties(mitk::BaseRenderer* renderer)
{

  //get the current localStorage for the corresponding renderer
  //FBLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

  renderer->GetVtkRenderer()->GetRenderWindow()->SetInteractor(NULL);
  //float opacity = 1.0f;
  //float rgb[3] = {1.0f, 1.0f, 1.0f};

  //set opacity (from propertyList) to visualized fibers
  //GetOpacity( opacity, renderer );
  //localStorage->m_PointActor->GetProperty()->SetOpacity((double)opacity);

  //set color (from propertyList) to visualized fibers
 // GetColor( rgb, renderer);
  //localStorage->m_PointActor->GetProperty()->SetColor((double)rgb[0], (double)rgb[1], (double)rgb[2]);



}


// following methods are essential, they actually call the GetVtkProp() method
// which returns the desired actors
void mitk::FiberBundleMapper2D::MitkRenderOverlay(BaseRenderer* renderer)
{
//   MITK_INFO << "FiberBundleMapper2D MitkRenderOVerlay(renderer)";
  if ( this->IsVisible(renderer)==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
  {
    this->GetVtkProp(renderer)->RenderOverlay(renderer->GetVtkRenderer());
  }
}

void mitk::FiberBundleMapper2D::MitkRenderOpaqueGeometry(BaseRenderer* renderer)
{
//  MITK_INFO << "FiberBundleMapper2D MitkRenderOpaqueGeometry(renderer)";
  if ( this->IsVisible( renderer )==false )
    return;

  if ( this->GetVtkProp(renderer)->GetVisibility() )
    this->GetVtkProp(renderer)->RenderOpaqueGeometry( renderer->GetVtkRenderer() );
}
void mitk::FiberBundleMapper2D::MitkRenderTranslucentGeometry(BaseRenderer* renderer)
{
//  MITK_INFO << "FiberBundleMapper2D MitkRenderTranslucentGeometry(renderer)";
  if ( this->IsVisible(renderer)==false )
    return;

  //TODO is it possible to have a visible BaseRenderer AND an invisible VtkRenderer???
  if ( this->GetVtkProp(renderer)->GetVisibility() )
    this->GetVtkProp(renderer)->RenderTranslucentPolygonalGeometry(renderer->GetVtkRenderer());

}
void mitk::FiberBundleMapper2D::MitkRenderVolumetricGeometry(BaseRenderer* renderer)
{
//  MITK_INFO << "FiberBundleMapper2D MitkRenderVolumentricGeometry(renderer)";
  if(IsVisible(renderer)==false)
    return;

  //TODO is it possible to have a visible BaseRenderer AND an invisible VtkRenderer???
  if ( GetVtkProp(renderer)->GetVisibility() )
    this->GetVtkProp(renderer)->RenderVolumetricGeometry(renderer->GetVtkRenderer());

}

mitk::FiberBundleMapper2D::FBLocalStorage::FBLocalStorage()
{
  m_PointActor = vtkSmartPointer<vtkActor>::New();
  m_PointMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

}
