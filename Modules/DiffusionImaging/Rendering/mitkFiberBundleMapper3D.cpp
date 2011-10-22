/*=========================================================================
 
 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
 Version:   $Revision: 17179 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#include <mitkProperties.h>
#include "mitkFiberBundleMapper3D.h"


#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>


#include <vtkLookupTable.h>
#include <vtkKochanekSpline.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunctionSource.h>

#include "mitkFiberBundleInteractor.h"
#include <mitkGlobalInteraction.h>

//template<class TPixelType>
mitk::FiberBundleMapper3D::FiberBundleMapper3D() 
: m_VtkFiberDataMapperGL(vtkOpenGLPolyDataMapper::New())
, m_FiberActor(vtkOpenGLActor::New())
, m_FiberAssembly(vtkPropAssembly::New())
, m_vtkTubeMapper(vtkOpenGLPolyDataMapper::New())
, m_tubes(vtkTubeFilter::New())
, m_TubeActor(vtkOpenGLActor::New())
{


}

mitk::FiberBundleMapper3D::~FiberBundleMapper3D()
{

}


const mitk::FiberBundle* mitk::FiberBundleMapper3D::GetInput()
{
  //MITK_INFO << "FiberBundleMapper3D GetInput()" ;
  
  return static_cast<const mitk::FiberBundle * > ( GetData() );
}

vtkOpenGLPolyDataMapper* mitk::FiberBundleMapper3D::getVtkFiberBundleMapper()
{
  return m_VtkFiberDataMapperGL;
}

/* 
 This method is called once the mapper gets new input, 
 for UI rotation or changes in colorcoding this method is NOT called 
 */
void mitk::FiberBundleMapper3D::GenerateData()
{
  /* ######## FIBER PREPARATION START ######### */
  //get fiberbundle
  mitk::FiberBundle::Pointer PFiberBundle = dynamic_cast< mitk::FiberBundle* > (this->GetData());
  
  //get groupFiberBundle, which is a datastructure containing single fibers
  mitk::FiberBundle::FiberGroupType::Pointer groupFiberBundle = PFiberBundle->GetGroupFiberBundle();
  
  //extractn single fibers
  //in the groupFiberBundle all smartPointers to single fibers are stored in in a ChildrenList
  mitk::FiberBundle::ChildrenListType * FiberList;
  FiberList =  groupFiberBundle->GetChildren();
  
  /* ######## FIBER PREPARATION END ######### */
  
  /* ######## VTK FIBER REPRESENTATION  ######## */
  //create a vtkPoints object and store the all the brainFiber points in it
  
 vtkSmartPointer<vtkPoints> vtkSmoothPoints = vtkPoints::New(); //in smoothpoints the interpolated points representing a fiber are stored.
  
  //in vtkcells all polylines are stored, actually all id's of them are stored
  vtkSmartPointer<vtkCellArray> vtkSmoothCells = vtkCellArray::New(); //cellcontainer for smoothed lines
  
  //in some cases a fiber includes just 1 point, so put it in here
  vtkSmartPointer<vtkCellArray> vtkVrtxs = vtkCellArray::New(); 
  
  //colors and alpha value for each single point, RGBA = 4 components
  vtkSmartPointer<vtkUnsignedCharArray> colorsT = vtkUnsignedCharArray::New();
  colorsT->SetNumberOfComponents(4);
  colorsT->SetName("ColorValues");
  
  vtkSmartPointer<vtkDoubleArray> faColors = vtkDoubleArray::New();
  faColors->SetName("FaColors");
  
  vtkSmartPointer<vtkDoubleArray> tubeRadius = vtkDoubleArray::New();
  tubeRadius->SetName("TubeRadius");
  
  
  // iterate through FiberList
  for(mitk::FiberBundle::ChildrenListType::iterator itLst = FiberList->begin();
      itLst != FiberList->end();
      ++itLst)
  {
    //all points are stored in one vtkpoints list, soooooooo that the lines find their point id to start and end we need some kind of helper index who monitors the current ids for a polyline 
    //unsigned long pntIdxHelper = vtkpointsDTI->GetNumberOfPoints();
    
    // lists output is SpatialObject, we know we have DTITubeSpacialObjects 
    // dynamic cast only likes pointers, no smartpointers, so each dsmartpointer has membermethod .GetPointer() 
    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());
    if (dtiTract.IsNull()) {
      return; }
    
    //get list of points
    int fibrNrPnts = dtiTract->GetNumberOfPoints();
    mitk::FiberBundle::DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();
  
    //MITK_INFO << "REAL AMOUNT OF FIBERS: " << fibrNrPnts;
    
    vtkSmartPointer<vtkPoints> vtkpointsDTI =  vtkPoints::New();
    
    if (fibrNrPnts <= 0) { //this should never occour! but who knows
      MITK_INFO << "HyperERROR in fiberBundleMapper3D.cpp ...no point in fiberBundle!!! .. check ur trackingAlgorithm";
      continue;
    }
    
    
    ///////PROCESS ON FIBERS////////
    for (int i=0; i<fibrNrPnts; ++i)
    {
      mitk::FiberBundle::DTITubePointType tmpFiberPntLst = dtiPntList.at(i);
      mitk::FiberBundle::DTITubePointType::PointType tmpFiberPnt = tmpFiberPntLst.GetPosition();
      
      float tmpvtkPnt[3] = {(float)tmpFiberPnt[0], (float)tmpFiberPnt[1], (float)tmpFiberPnt[2]};
      
      mitk::Point3D indexPnt(tmpvtkPnt);
      mitk::Point3D worldPnt;
      
      // MITK_INFO << tmpFiberPnt[0] << " " << tmpFiberPnt[1] << " " << tmpFiberPnt[2];
      // MITK_INFO << worldPnt[0] << " " << worldPnt[1] << " " << worldPnt[2];
      
      /////POINT MANAGEMENT///////
      if (i%1==0) { //if u want to take not each point of bundle, alter modulo to >1
        PFiberBundle->GetGeometry()->IndexToWorld(indexPnt, worldPnt);
        double worldFbrPnt[3] = {worldPnt[0], worldPnt[1], worldPnt[2]};
        vtkpointsDTI->InsertNextPoint(worldFbrPnt);
      }

      //MITK_INFO << "REDUCED AMOUNT OF FIBERS: " << vtkpointsDTI->GetNumberOfPoints();
      
      ////POINTS OF DTI ARE READY FOR FUTHER VTK PROCESSING////
    }
    
    
    /////PROCESS POLYLINE SMOOTHING/////
    vtkSmartPointer<vtkKochanekSpline> xSpline = vtkKochanekSpline::New();
    vtkSmartPointer<vtkKochanekSpline> ySpline = vtkKochanekSpline::New();
    vtkSmartPointer<vtkKochanekSpline> zSpline = vtkKochanekSpline::New();
    
    vtkSmartPointer<vtkParametricSpline> spline = vtkParametricSpline::New();
    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(vtkpointsDTI);
    
    
    vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkParametricFunctionSource::New();
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(200);
    functionSource->SetVResolution(200);
    functionSource->SetWResolution(200);
    functionSource->Update();
    
    
    
    vtkPolyData* outputFunction = functionSource->GetOutput();
    vtkPoints* tmpSmoothPnts = outputFunction->GetPoints(); //smoothPoints of current fiber
       
    vtkSmartPointer<vtkPolyLine> smoothLine = vtkPolyLine::New();
    smoothLine->GetPointIds()->SetNumberOfIds(tmpSmoothPnts->GetNumberOfPoints());
//    MITK_INFO << "SMOOTHED AMOUNT OF POINTS:" << tmpSmoothPnts->GetNumberOfPoints();

    
    /////CREATE SMOOTH POLYLINE OBJECT////
    /////MANAGE LINE AND CORRELATED POINTS/////
    int pointHelperCnt = vtkSmoothPoints->GetNumberOfPoints(); //also put current points into global smooth pointcontainer
    int nrSmPnts = tmpSmoothPnts->GetNumberOfPoints();
    tubeRadius->SetNumberOfTuples(nrSmPnts);
    double tbradius = 1;//default value for radius
    
    for(int ism=0; ism<nrSmPnts; ism++)
    {
  //    MITK_INFO << "ism: " << ism << " of " << nrSmPnts;
      smoothLine->GetPointIds()->SetId(ism, ism+pointHelperCnt);
      vtkSmoothPoints->InsertNextPoint(tmpSmoothPnts->GetPoint(ism));
       
      // MITK_INFO << "LinePntID: " << ism << " linked to: " << ism+pointHelperCnt << " val: " << tmpSmoothPnts->GetPoint(ism)[0] << " " << tmpSmoothPnts->GetPoint(ism)[1] << " " << tmpSmoothPnts->GetPoint(ism)[2];
      tubeRadius->SetTuple1(ism,tbradius); //tuple with 1 argument

     
      //colorcoding orientation based
      unsigned char rgba[4] = {0,0,0,0};
      if (ism < nrSmPnts-1 && ism>0)
      {
        
 //       MITK_INFO << "inbetween fiber, at position:" << ism;
//        //nimm nur diff1
        vnl_vector_fixed< double, 3 > tmpPntvtk(tmpSmoothPnts->GetPoint(ism)[0], tmpSmoothPnts->GetPoint(ism)[1],tmpSmoothPnts->GetPoint(ism)[2]);
        vnl_vector_fixed< double, 3 > nxttmpPntvtk(tmpSmoothPnts->GetPoint(ism+1)[0], tmpSmoothPnts->GetPoint(ism+1)[1], tmpSmoothPnts->GetPoint(ism+1)[2]);
        vnl_vector_fixed< double, 3 > prevtmpPntvtk(tmpSmoothPnts->GetPoint(ism-1)[0], tmpSmoothPnts->GetPoint(ism-1)[1], tmpSmoothPnts->GetPoint(ism-1)[2]);
        
        vnl_vector_fixed< double, 3 > diff1;
        diff1 = tmpPntvtk - nxttmpPntvtk;
        diff1.normalize();
        
        vnl_vector_fixed< double, 3 > diff2;
        diff2 = tmpPntvtk - prevtmpPntvtk;
        diff2.normalize();
        
        vnl_vector_fixed< double, 3 > diff;
        diff = (diff1 - diff2)/2.0;
        
        
        rgba[0] = (unsigned char) (255.0 * std::abs(diff[0]));
        rgba[1] = (unsigned char) (255.0 * std::abs(diff[1]));
        rgba[2] = (unsigned char) (255.0 * std::abs(diff[2]));
        rgba[3] = (unsigned char) (255.0); 
        
        
        
      } else if(ism==0) {
        //explicit handling of startpoint of line
        
        //nimm nur diff1
        vnl_vector_fixed< double, 3 > tmpPntvtk(tmpSmoothPnts->GetPoint(ism)[0], tmpSmoothPnts->GetPoint(ism)[1],tmpSmoothPnts->GetPoint(ism)[2]);
        vnl_vector_fixed< double, 3 > nxttmpPntvtk(tmpSmoothPnts->GetPoint(ism+1)[0], tmpSmoothPnts->GetPoint(ism+1)[1], tmpSmoothPnts->GetPoint(ism+1)[2]);
        
        
        vnl_vector_fixed< double, 3 > diff1;
        diff1 = tmpPntvtk - nxttmpPntvtk;
        diff1.normalize();
        
        rgba[0] = (unsigned char) (255.0 * std::abs(diff1[0]));
        rgba[1] = (unsigned char) (255.0 * std::abs(diff1[1]));
        rgba[2] = (unsigned char) (255.0 * std::abs(diff1[2]));
        rgba[3] = (unsigned char) (255.0); 
        
//        MITK_INFO << "first point color: " << rgba[0] << " " << rgba[1] << " " << rgba[2]; 
        
        
      } else if(ism==nrSmPnts-1) {
//      last point in fiber
      
        // nimm nur diff2
        vnl_vector_fixed< double, 3 > tmpPntvtk(tmpSmoothPnts->GetPoint(ism)[0], tmpSmoothPnts->GetPoint(ism)[1],tmpSmoothPnts->GetPoint(ism)[2]);
        vnl_vector_fixed< double, 3 > prevtmpPntvtk(tmpSmoothPnts->GetPoint(ism-1)[0], tmpSmoothPnts->GetPoint(ism-1)[1], tmpSmoothPnts->GetPoint(ism-1)[2]);
        
        vnl_vector_fixed< double, 3 > diff2;
        diff2 = tmpPntvtk - prevtmpPntvtk;
        diff2.normalize();
        
        rgba[0] = (unsigned char) (255.0 * std::abs(diff2[0]));
        rgba[1] = (unsigned char) (255.0 * std::abs(diff2[1]));
        rgba[2] = (unsigned char) (255.0 * std::abs(diff2[2]));
        rgba[3] = (unsigned char) (255.0); 
//        
//        MITK_INFO << "last point color: " << rgba[0] << " " << rgba[1] << " " << rgba[2]; 
      } //end colorcoding
//      
      colorsT->InsertNextTupleValue(rgba);
      
    }//end of smoothline
    
    ///////smooth Fiber ready////////
    vtkSmoothCells->InsertNextCell(smoothLine);
  }
  
  //vtkpointsDTI->Delete();//points are not needed anymore TODO uncomment!
  
  
  /*
   
   //get FA value ... for that FA has to be interpolated as well as DTItracktLine
   float faVal = tmpFiberPntLst.GetField(mitk::FiberBundle::DTITubePointType::FA);
   //use insertNextValue cuz FA Values are reperesented as a single number (1 Tuple containing 1 parameter)
   faColors->InsertNextValue((double) faVal);
   
   */
  //vtkcells->InitTraversal();
  
  // Put points and lines together in one polyData structure
  vtkSmartPointer<vtkPolyData> polyData = vtkPolyData::New();
  polyData->SetPoints(vtkSmoothPoints);
  polyData->SetLines(vtkSmoothCells);
  
  if (vtkVrtxs->GetSize() > 0) {
    polyData->SetVerts(vtkVrtxs);
  }
  polyData->GetPointData()->AddArray(colorsT);
  //polyData->GetPointData()->AddArray(faColors);
  //polyData->GetPointData()->AddArray(tubeRadius);
  
  
  vtkSmartPointer<vtkLookupTable> lut = vtkLookupTable::New();
  lut->Build();
  
  
 // m_VtkFiberDataMapperGL = vtkOpenGLPolyDataMapper::New();
  m_VtkFiberDataMapperGL->SetInput(polyData);
  m_VtkFiberDataMapperGL->ScalarVisibilityOn();
  
  m_VtkFiberDataMapperGL->SetScalarModeToUsePointFieldData();
  //m_VtkFiberDataMapperGL->SelectColorArray("FaColors");
  m_VtkFiberDataMapperGL->SelectColorArray("ColorValues");
  m_VtkFiberDataMapperGL->SetLookupTable(lut);

  
  //m_FiberActor = vtkOpenGLActor::New();
  m_FiberActor->SetMapper(m_VtkFiberDataMapperGL);
  m_FiberActor->GetProperty()->SetOpacity(1.0);
  m_FiberActor->GetProperty()->SetPointSize(4.0f);
  // m_FiberActor->GetProperty()->SetColor(255.0, 0.0, 0.0);
  
  
  
  m_FiberAssembly->AddPart(m_FiberActor);
  
  //setting color and opacity in the fiberActor itself is not recommended 
  //here cuz color and opacity of dataNode will be considered in GetData(baserenderer*) anyway
  this->GetDataNode()->SetColor(255.0,0,0);
  this->GetDataNode()->SetOpacity(1.0);  
  
  
}




//template<class TPixelType>
void mitk::FiberBundleMapper3D::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  
  
  
  // nodeCC = 1 ... ROI colorcoding
  //   2 ... orientation colorcoding
  //   3 ... FA colorcoding
  
  
  int nodeCC;
  bool isCCd = this->GetDataNode()->GetPropertyValue("ColorCoding", nodeCC);
  if ( isCCd && nodeCC == 1 ) {
    
    
    //get color and opacity from DataNode
    int tmpline;
    
    bool isLineProp = this->GetDataNode()->GetPropertyValue("LineWidth",tmpline);
    
    bool isPointRep;
    bool successPointProp = this->GetDataNode()->GetPropertyValue("RepPoints", isPointRep);
    float pointSize;
    bool successPointSize = this->GetDataNode()->GetPropertyValue("pointSize", pointSize);
    
    if (isLineProp) {
      m_FiberActor->GetProperty()->SetLineWidth(tmpline); 
    } 
    
    if(isPointRep) {
      m_FiberActor->GetProperty()->SetRepresentationToPoints();
      m_FiberActor->GetProperty()->SetPointSize(pointSize);
    }
    
  } else if (isCCd && nodeCC == 2) {
    float tmpopa;
    this->GetDataNode()->GetOpacity(tmpopa, NULL);
    m_FiberActor->GetProperty()->SetOpacity((double) tmpopa);
    
  } else if (isCCd && nodeCC == 3) {
    
    float temprgb[3];
    this->GetDataNode()->GetColor( temprgb, NULL );
    double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
    m_FiberActor->GetProperty()->SetColor(trgb);
    
    if(m_VtkFiberDataMapperGL->GetScalarVisibility()) //can be 0 or 1, for scalarVis On or Off
    {
      m_VtkFiberDataMapperGL->ScalarVisibilityOff();
    }
    
    
    
  } else if (isCCd && nodeCC == 4) {
    
    if(!m_VtkFiberDataMapperGL->GetScalarVisibility()) //can be 0 or 1, for scalarVis On or Off
    {
      m_VtkFiberDataMapperGL->ScalarVisibilityOn();
    }
    m_VtkFiberDataMapperGL->SelectColorArray("ColorValues");
    
  } else if (isCCd && nodeCC == 5){
    
    if(!m_VtkFiberDataMapperGL->GetScalarVisibility()) //if visibility is off, switch it on
    {
      m_VtkFiberDataMapperGL->ScalarVisibilityOn();
    }    
    m_VtkFiberDataMapperGL->SelectColorArray("FaColors");
    
  } else if (isCCd && nodeCC == 6){
    //orientationbased colorcoding + FA as opacity
    //get FA out of polydata, which is saved in faColor vtkDoubleArray
    
    vtkPolyData *tmpPolyData = m_VtkFiberDataMapperGL->GetInput();
    vtkPointData *tmpPointData = tmpPolyData->GetPointData();
    
    
    
    int hasAr = tmpPointData->HasArray("FaColors");
    if(!hasAr)
      return;
    
    vtkDoubleArray *tmpFAarray = (vtkDoubleArray*)(tmpPointData->GetArray("FaColors")) ; 
    
    /*for(int i=0;  i<tmpFAarray->GetNumberOfTuples(); i++)
     {
     
     double *tmpTuple;
     tmpFAarray->GetTuple(i, tmpTuple);
     
     for(int j=0; j<tmpFAarray->GetNumberOfComponents(); j++)
     {
     MITK_INFO << "FA Value: at index " << i << ": " << tmpTuple[j]; 
     }
     
     
     }
     */
    
    //since we have our FA values, lets replace the alpha values in colorT
    //we know each 4th entry is a A value of RGBA
    int hasArCV = tmpPointData->HasArray("ColorValues");
    if(!hasArCV)
      return;
    
    vtkUnsignedCharArray *colorsTtmp = dynamic_cast<vtkUnsignedCharArray*>  (tmpPointData->GetArray("ColorValues"));
    
    for(int i=0; i<colorsTtmp->GetNumberOfTuples(); i++)
    {
      
      //double *tmpTupleCV = colorsTtmp->GetTuple4(i);
      double tmpTupleFA = tmpFAarray->GetTuple1(i);
      tmpTupleFA = tmpTupleFA * 255.0;
      
      colorsTtmp->SetComponent(i,3, tmpTupleFA );
      
      //  MITK_INFO << "----" << i;
      //MITK_INFO << tmpTupleCV[0];
      //MITK_INFO << tmpTupleCV[1];
      //MITK_INFO << tmpTupleCV[2];
      //MITK_INFO << tmpTupleCV[3];
      //double *test = m_VtkFiberDataMapperGL->GetInput()->GetPointData()->GetArray("ColorValues")->GetTuple4(i);
      //MITK_INFO << test[0];
      //MITK_INFO << test[1];
      //MITK_INFO << test[2];
      //MITK_INFO << test[3];
      
      
    }
    
    m_VtkFiberDataMapperGL->SelectColorArray("");
    m_VtkFiberDataMapperGL->SelectColorArray("ColorValues");
    
    
  } else if (isCCd && nodeCC == 7){
    
    vtkPolyData *tmpPolyData = m_VtkFiberDataMapperGL->GetInput();
    vtkPointData *tmpPointData = tmpPolyData->GetPointData();
    int hasArCV = tmpPointData->HasArray("ColorValues");
    if(!hasArCV)
      return;
    
    vtkUnsignedCharArray *colorsTtmp = dynamic_cast<vtkUnsignedCharArray*>  (tmpPointData->GetArray("ColorValues"));
    
    for(int i=0; i<colorsTtmp->GetNumberOfTuples(); i++)
    {
      
      double tmpTupleFA = 255.0;
      colorsTtmp->SetComponent(i,3, tmpTupleFA );
    }
    m_VtkFiberDataMapperGL->SelectColorArray("");
    m_VtkFiberDataMapperGL->SelectColorArray("ColorValues");
    
    
  } else if (isCCd && nodeCC == 8) {
    /* something is still missing to activate smoothing or make it work.... */
    if (!renderer->GetRenderWindow()->GetLineSmoothing()) {
      renderer->GetRenderWindow()->SetLineSmoothing(1);
      renderer->GetRenderWindow()->Modified();
    }
    if (!renderer->GetRenderWindow()->GetPointSmoothing()) {
      renderer->GetRenderWindow()->SetPointSmoothing(1);
      renderer->GetRenderWindow()->Modified();
    }
    if (!renderer->GetRenderWindow()->GetPolygonSmoothing()) {
      renderer->GetRenderWindow()->SetPolygonSmoothing(1);
      renderer->GetRenderWindow()->Modified();
    }
    
    
  } else if (isCCd && nodeCC == 9) {
    if  (renderer->GetRenderWindow()->GetLineSmoothing()) {
      renderer->GetRenderWindow()->SetLineSmoothing(0);
      renderer->GetRenderWindow()->Modified();
    }
    if (renderer->GetRenderWindow()->GetPointSmoothing()) {
      renderer->GetRenderWindow()->SetPointSmoothing(0);
      renderer->GetRenderWindow()->Modified();
    }
    if (renderer->GetRenderWindow()->GetPolygonSmoothing()) {
      renderer->GetRenderWindow()->SetPolygonSmoothing(0);
      renderer->GetRenderWindow()->Modified();
    }
    
  } else if (isCCd && nodeCC == 10) {
    // manipulate X Coordinates of selected FiberBundle    
    int tmpXmove;
    
    bool isXmove = this->GetDataNode()->GetPropertyValue("Xmove",tmpXmove);
    
    if (!isXmove) 
      return;
    
    vtkPolyData *tmpPolyData = m_VtkFiberDataMapperGL->GetInput();
    vtkPoints* tmpVtkPnts = tmpPolyData->GetPoints();
    double PtmpPntVal[3];
    
    for (int i=0; i<tmpVtkPnts->GetNumberOfPoints(); ++i )
    {
      tmpVtkPnts->GetPoint(i,PtmpPntVal);
      
      PtmpPntVal[0] = PtmpPntVal[0] + (double) tmpXmove;
      tmpVtkPnts->SetPoint(i, PtmpPntVal);
      
      tmpPolyData->Modified();
      
    }
    
  } else if (isCCd && nodeCC == 11) {
    // manipulate Y Coordinates of selected FiberBundle    
    int tmpYmove;
    
    bool isYmove = this->GetDataNode()->GetPropertyValue("Ymove",tmpYmove);
    
    if (!isYmove) 
      return;
    
    vtkPolyData *tmpPolyData = m_VtkFiberDataMapperGL->GetInput();
    vtkPoints* tmpVtkPnts = tmpPolyData->GetPoints();
    double PtmpPntVal[3];
    
    for (int i=0; i<tmpVtkPnts->GetNumberOfPoints(); ++i )
    {
      tmpVtkPnts->GetPoint(i,PtmpPntVal);
      
      PtmpPntVal[1] = PtmpPntVal[1] + (double) tmpYmove;
      tmpVtkPnts->SetPoint(i, PtmpPntVal);
      
      tmpPolyData->Modified();
      
    }
    
  } else if (isCCd && nodeCC == 12) {
    // manipulate Z Coordinates of selected FiberBundle
    int tmpZmove;
    
    bool isZmove = this->GetDataNode()->GetPropertyValue("Zmove",tmpZmove);
    if (!isZmove) 
      return;
    
    vtkPolyData *tmpPolyData = m_VtkFiberDataMapperGL->GetInput();
    vtkPoints* tmpVtkPnts = tmpPolyData->GetPoints();
    double PtmpPntVal[3];
    
    for (int i=0; i<tmpVtkPnts->GetNumberOfPoints(); ++i )
    {
      tmpVtkPnts->GetPoint(i,PtmpPntVal);
      
      PtmpPntVal[2] = PtmpPntVal[2] + (double) tmpZmove;
      //PtmpPntVal[2] = PtmpPntVal[2] + 1;
      tmpVtkPnts->SetPoint(i, PtmpPntVal);
      
      tmpPolyData->Modified();
      
    }
    
  } else if (isCCd && nodeCC == 13) {
    int tmpTubeSides;
    bool isTubeSides = this->GetDataNode()->GetPropertyValue("TubeSides",tmpTubeSides);
    
    float tmpRadius;
    bool isRadius = this->GetDataNode()->GetPropertyValue("TubeRadius",tmpRadius);
    
    if (!isTubeSides) 
      return;
    
    vtkPolyData *tmpPolyData = m_VtkFiberDataMapperGL->GetInput();
    
    
  
    m_tubes->SetInput(tmpPolyData);
    m_tubes->SidesShareVerticesOn();
    m_tubes->SetRadius((double)(tmpRadius));
    m_tubes->SetNumberOfSides(tmpTubeSides);
    m_tubes->Modified();
    //    m_tubes->Update();
    
    
    m_vtkTubeMapper->SetInputConnection(m_tubes->GetOutputPort());
    m_vtkTubeMapper->ScalarVisibilityOn();
    m_vtkTubeMapper->SetScalarModeToUsePointFieldData();
    m_vtkTubeMapper->SelectColorArray("");
    m_vtkTubeMapper->SelectColorArray("ColorValues");
    
    
    
    m_TubeActor->SetMapper(m_vtkTubeMapper);
    m_TubeActor->GetProperty()->SetOpacity(1);
    m_TubeActor->GetProperty()->BackfaceCullingOn();
    
    m_FiberAssembly->AddPart(m_TubeActor);
    m_FiberAssembly->Modified();
    
     } else if (isCCd && nodeCC == 14) {
       
       float temprgb[3];
       this->GetDataNode()->GetColor( temprgb, NULL );
       double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
       m_TubeActor->GetProperty()->SetColor(trgb);
       
       if(m_vtkTubeMapper->GetScalarVisibility()) //can be 0 or 1, for scalarVis On or Off
       {
         m_vtkTubeMapper->ScalarVisibilityOff();
       }
    
  } else if (isCCd && nodeCC == 15) {
    m_TubeActor->GetProperty()->SetOpacity(0);
    m_FiberAssembly->RemovePart(m_TubeActor);
    m_FiberAssembly->Modified();
    
  }else if (isCCd && nodeCC == 16) {
    float tmpTubeOpacity;
    bool isTubeOpacity = this->GetDataNode()->GetPropertyValue("TubeOpacity",tmpTubeOpacity);
    
    m_TubeActor->GetProperty()->SetOpacity((double) tmpTubeOpacity);
    m_TubeActor->Modified();
    
  } else if (isCCd && nodeCC == 17) {
    m_FiberActor->GetProperty()->SetOpacity(0);
    m_FiberAssembly->RemovePart(m_FiberActor);
    m_FiberAssembly->Modified();
    
  }else if (isCCd && nodeCC == 18) {
    m_FiberActor->GetProperty()->SetOpacity(0);
    m_FiberAssembly->AddPart(m_FiberActor);
    m_FiberAssembly->Modified();
    
  }
  
  
  
  //MITK_INFO << m_VtkFiberDataMapperGL->GetArrayName();
  
  
  /* int displayIndex(0);
   this->GetDataNode()->GetIntProperty( "DisplayChannel", displayIndex, renderer );
   InputImageType *input = const_cast< InputImageType* >(
   this->GetInput()
   );
   mitk::DiffusionImage<TPixelType> *input2 = dynamic_cast< mitk::DiffusionImage<TPixelType>* >(
   input
   );
   input2->SetDisplayIndexForRendering(displayIndex);
   Superclass::GenerateDataForRenderer(renderer);
   */
}

//template<class TPixelType>
void mitk::FiberBundleMapper3D::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
//  MITK_INFO << "FiberBundleMapper3D SetDefault Properties(...)";
  node->AddProperty( "DisplayChannel", mitk::IntProperty::New( true ), renderer, overwrite );
  node->AddProperty( "LineWidth", mitk::IntProperty::New( true ), renderer, overwrite );
  node->AddProperty( "ColorCoding", mitk::IntProperty::New( 0 ), renderer, overwrite);
  node->AddProperty( "VertexOpacity_1", mitk::BoolProperty::New( false ), renderer, overwrite);
  node->AddProperty( "Set_FA_VertexAlpha", mitk::BoolProperty::New( false ), renderer, overwrite);
  node->AddProperty( "pointSize", mitk::FloatProperty::New(0.5), renderer, overwrite);
  node->AddProperty( "setShading", mitk::IntProperty::New(1), renderer, overwrite);
  node->AddProperty( "Xmove", mitk::IntProperty::New( 0 ), renderer, overwrite);
  node->AddProperty( "Ymove", mitk::IntProperty::New( 0 ), renderer, overwrite);
  node->AddProperty( "Zmove", mitk::IntProperty::New( 0 ), renderer, overwrite);
  node->AddProperty( "RepPoints", mitk::BoolProperty::New( false ), renderer, overwrite);
  node->AddProperty( "TubeSides", mitk::IntProperty::New( 8 ), renderer, overwrite);
  node->AddProperty( "TubeRadius", mitk::FloatProperty::New( 0.15 ), renderer, overwrite);
  node->AddProperty( "TubeOpacity", mitk::FloatProperty::New( 1.0 ), renderer, overwrite);
  
  node->AddProperty( "pickable", mitk::BoolProperty::New( true ), renderer, overwrite);

  Superclass::SetDefaultProperties(node, renderer, overwrite);
  
  
  
}

vtkProp* mitk::FiberBundleMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  
  //MITK_INFO << "FiberBundleMapper3D GetVtkProp(renderer)";
  return m_FiberAssembly;
  
}

void mitk::FiberBundleMapper3D::ApplyProperties(mitk::BaseRenderer* renderer)
{
  // MITK_INFO << "FiberBundleMapper3D ApplyProperties(renderer)";
}

void mitk::FiberBundleMapper3D::UpdateVtkObjects()
{
  // MITK_INFO << "FiberBundleMapper3D UpdateVtkObjects()";
  
  
}

void mitk::FiberBundleMapper3D::SetVtkMapperImmediateModeRendering(vtkMapper *)
{
  
  
  
}



