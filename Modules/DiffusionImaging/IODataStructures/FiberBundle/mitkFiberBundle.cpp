/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
 Version:   $Revision: 21975 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#include "mitkFiberBundle.h"
#include <mitkGeometry2D.h>
#include <mitkVector.h>
#include <mitkPlaneGeometry.h>
#include <mitkLine.h>


#include <mitkPlanarCircle.h>
#include <mitkPlanarRectangle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarFigureComposite.h>

#include <vtkPolygon.h>
//#include <vtkPoints.h>


/* statics values to define the position of a fiberTractPoint
 * related to the plane of a ROI
 */
const int mitk::FiberBundle::TRACTPOINT_BACKFACE = 0;
const int mitk::FiberBundle::TRACTPOINT_ON_PLANE = 1;
const int mitk::FiberBundle::TRACTPOINT_FRNTFACE = 2;

// implementation of all essential methods from superclass

mitk::FiberBundle::FiberBundle()
{
  m_GroupFiberBundle = FiberGroupType::New();

  m_TractContainer = ContainerType::New();

  //by default set a standard geometry, usually geometry is set by the user on initializing
  //a mitkFiberBundle Object
  mitk::Geometry3D::Pointer fbgeometry = mitk::Geometry3D::New();
  fbgeometry->SetIdentity();
  this->SetGeometry(fbgeometry);

  /* for debugging only */
  m_debugITKContainer = itkStochTractContainerType::New();


}

mitk::FiberBundle::~FiberBundle()
{

}

void mitk::FiberBundle::SetBounds(float* b)
{
  m_boundsFB[0] = b[0];
  m_boundsFB[1] = b[1];
  m_boundsFB[2] = b[2];
}

void mitk::FiberBundle::SetBounds(double* b)
{
  m_boundsFB[0] = b[0];
  m_boundsFB[1] = b[1];
  m_boundsFB[2] = b[2];
}

float* mitk::FiberBundle::GetBounds()
{
  return m_boundsFB;
}

void mitk::FiberBundle::PushPoint(int fiberIndex, ContainerPointType point)
{
  if( (unsigned)fiberIndex >= m_TractContainer->Size() )
  {
    fiberIndex = m_TractContainer->Size();
    ContainerTractType::Pointer tract = ContainerTractType::New();
    tract->InsertElement(tract->Size(),point);
    m_TractContainer->InsertElement(fiberIndex, tract);
  }
  else if(fiberIndex>=0)
  {
    m_TractContainer->ElementAt(fiberIndex)->InsertElement(m_TractContainer->ElementAt(fiberIndex)->Size(), point);
  }
}

void mitk::FiberBundle::PushTract(ContainerTractType::Pointer tract)
{
  m_TractContainer->InsertElement(m_TractContainer->Size(), tract);
}

mitk::FiberBundle::Pointer mitk::FiberBundle::JoinBundle(mitk::FiberBundle::Pointer bundle)
{
  mitk::FiberBundle::Pointer retval = mitk::FiberBundle::New();
  retval->SetGeometry(this->GetGeometry());
  retval->SetBounds(this->m_boundsFB);

  retval->InsertBundle(this);
  retval->InsertBundle(bundle);
  return retval;
}

void mitk::FiberBundle::InsertBundle(mitk::FiberBundle::Pointer bundle)
{

  // INCOMPLETE, SHOULD CHECK FOR DIFFERENT GEOMETRIES IN THIS+BUNDLE
  // DO INDEX1 -> WORLD -> INDEX2 TRANSFORMATION, IF THEY ARE DIFFERENT.

  int num = this->GetNumTracts();

  FiberGroupType::Pointer groupFiberBundle = bundle->GetGroupFiberBundle();
  ChildrenListType *fiberList = groupFiberBundle->GetChildren();
  for(ChildrenListType::iterator itLst = fiberList->begin();
      itLst != fiberList->end();
      ++itLst)
  {

    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());

    if (dtiTract.IsNull())
    {
      MITK_INFO << "DTI Tract is NULL!!!!! omg";
      continue;
    }

    dtiTract->SetId(num++);
    this->addSingleDTITract(dtiTract);
  }

  ContainerType::Pointer container = bundle->GetTractContainer();
  for(int i=0; i<container->Size();i++)
  {
    this->PushTract(container->ElementAt(i));
  }

}

int mitk::FiberBundle::FindTractByEndpoints(mitk::FiberBundle::DTITubeType::Pointer searchTract)
{
  int searchNrPnts = searchTract->GetNumberOfPoints();
  mitk::FiberBundle::DTITubeType::PointListType searchPntList = searchTract->GetPoints();

  typedef mitk::FiberBundle::DTITubeType::PointType PointType;

  DTITubePointType searchPointFirst = searchPntList.at(0);
  PointType searchPos1 = searchPointFirst.GetPosition();
  DTITubePointType searchPointLast = searchPntList.at(searchNrPnts-1);
  PointType searchPos2 = searchPointLast.GetPosition();

  FiberGroupType::Pointer groupFiberBundle = this->GetGroupFiberBundle();
  ChildrenListType *fiberList = groupFiberBundle->GetChildren();
  for(ChildrenListType::iterator itLst = fiberList->begin();
      itLst != fiberList->end();
      ++itLst)
  {

    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());

    if (dtiTract.IsNull())
    {
      MITK_INFO << "DTI Tract is NULL!!!!! omg";
      continue;
    }

    mitk::FiberBundle::DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();
    int fibrNrPnts = dtiTract->GetNumberOfPoints();

    DTITubePointType point_first = dtiPntList.at(0);
    PointType pos1 = point_first.GetPosition();
    DTITubePointType point_last = dtiPntList.at(fibrNrPnts-1);
    PointType pos2 = point_last.GetPosition();

    if( (pos1 == searchPos1 && pos2 == searchPos2) ||
        (pos2 == searchPos1 && pos1 == searchPos2) )
    {
      return dtiTract->GetId();
    }
  }

  return -1;
}

mitk::FiberBundle::Pointer mitk::FiberBundle::SubstractBundle(mitk::FiberBundle::Pointer bundle)
{

  mitk::FiberBundle::Pointer retval = mitk::FiberBundle::New();
  retval->SetGeometry(this->GetGeometry());

  mitk::FiberBundle::Pointer largeBundle = bundle;
  mitk::FiberBundle::Pointer smallBundle = this;

  MITK_INFO << "large children " << largeBundle->GetGroupFiberBundle()->GetNumberOfChildren() << "!="<<largeBundle->GetNumTracts();
  MITK_INFO << "small children " << smallBundle->GetGroupFiberBundle()->GetNumberOfChildren() << "!="<<smallBundle->GetNumTracts();

  if(this->GetGroupFiberBundle()->GetNumberOfChildren() > bundle->GetGroupFiberBundle()->GetNumberOfChildren())
  {
    MITK_INFO << "this is large (" << this->GetNumTracts() << ">" << bundle->GetNumTracts() << ")";
    largeBundle = this;
    smallBundle = bundle;
  }


  ContainerType::Pointer container = largeBundle->GetTractContainer();
  int counter = 0;

  FiberGroupType::Pointer groupFiberBundle = largeBundle->GetGroupFiberBundle();
  ChildrenListType *fiberList = groupFiberBundle->GetChildren();
  MITK_INFO << "large number children " << groupFiberBundle->GetNumberOfChildren();
  for(ChildrenListType::iterator itLst = fiberList->begin();
      itLst != fiberList->end();
      ++itLst)
  {

    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());

    if (dtiTract.IsNull())
    {
      MITK_INFO << "DTI Tract is NULL!!!!! omg";
      continue;
    }

    int delId = smallBundle->FindTractByEndpoints(dtiTract);
    if( delId == -1 )
    {
      retval->addSingleDTITract(dtiTract);
      retval->PushTract(container->ElementAt(counter));
    }

    MITK_INFO << "Counter " << counter++;
  }

  return retval;
}

mitk::FiberBundle::ContainerPointType mitk::FiberBundle::GetPoint(int tractIndex, int pointIndex)
{
  if (tractIndex>=0 && tractIndex<GetNumTracts() && pointIndex>=0 && pointIndex<GetNumPoints(tractIndex))
  {
    return m_TractContainer->GetElement(tractIndex)->GetElement(pointIndex);
  }
  return NULL;
}

int mitk::FiberBundle::GetNumTracts()
{
  return m_TractContainer->Size();
}

int mitk::FiberBundle::GetNumPoints(int tractIndex)
{
  if ((unsigned)tractIndex>=0 && (unsigned)tractIndex<m_TractContainer->Size())
  {
    return m_TractContainer->GetElement(tractIndex)->Size();

  } else {
    //added by igi
    return NULL;
  }

}

mitk::FiberBundle::ContainerTractType::Pointer mitk::FiberBundle::GetTract(int tractIndex)
{
  ContainerTractType::Pointer out;
  if ((unsigned)tractIndex>0 && (unsigned)tractIndex<m_TractContainer->Size())
    return m_TractContainer->GetElement(tractIndex);
  return out;
}

void mitk::FiberBundle::additkStochTractContainer(itkStochTractContainerType::Pointer itkStochCont)
{
  //for debugging only
  m_debugITKContainer = itkStochCont;
  //********************


  /* transform itkStochContainer to standardized TractContainer */

  for (itkStochTractContainerType::ConstIterator itCont = itkStochCont->Begin();
       itCont != itkStochCont->End();
       ++itCont)
  {

    // get fiber of itkContainer
    itkStochTractType::Pointer tract = itCont->Value();
    itkStochTractType::VertexListType::ConstPointer vertexlist = tract->GetVertexList();

    //init a desired containerFiber
    ContainerTractType::Pointer contTract = ContainerTractType::New();

    //get points of fiber
    for( int j=0; j<(int)vertexlist->Size(); ++j)
    {

      // put the point of vertex pointList in itkContainer
      itkStochTractType::VertexType vertex = vertexlist->GetElement(j);

      //prepare for dtiPoint
      ContainerPointType contPnt;
      contPnt[0] = (float) vertex[0];
      contPnt[1] = (float) vertex[1];
      contPnt[2] = (float) vertex[2];

      contTract->InsertElement(contTract->Size(), contPnt);

      /* coordinate testing*/
      /*if ((float)vertex[0] == contPnt[0]) {
       MITK_INFO << " [OK] ... X ";
       }else {
       MITK_INFO << "[FAIL] ... itkX: " << vertex[0] << " " << contPnt[0] << "\n" ;
       }

       if ((float)vertex[1] == contPnt[1]) {
       MITK_INFO << " [OK] ... Y ";
       }else {
       MITK_INFO << "[FAIL] ... itkY: " << vertex[1] << " " << contPnt[1] << "\n" ;
       }
       if ((float)vertex[2] == contPnt[2]) {
       MITK_INFO << " [OK] ... Z " << "\n" ;
       MITK_INFO << " Values X Y Z: " << contPnt[0] << " " << contPnt[1] << " " << contPnt[2] << "\n";
       }else {
       MITK_INFO << "[FAIL] ... itkZ: " << vertex[2] << " " << contPnt[2] << "\n" ;
       }
       */



    }

    // add filled fiber to container
    m_TractContainer->InsertElement(m_TractContainer->Size(), contTract);

  }


}

void mitk::FiberBundle::addTractContainer( ContainerType::Pointer tractContainer )
{
  m_TractContainer = tractContainer;
}


void mitk::FiberBundle::initFiberGroup()
{
  /* iterate through the tractContainer and store fibers in DTISpatialObjects */
  for (ContainerType::ConstIterator itCont = m_TractContainer->Begin();
       itCont != m_TractContainer->End();
       ++itCont)
  {
    // init new dtiTube and
    DTITubeType::Pointer dtiTube = DTITubeType::New();
    DTITubeType::PointListType list;

    // get current tract of container
    ContainerTractType::Pointer contTract = itCont->Value();

    // iterate through the number of points ... use iterator, no index-output is needed anyway
    for(ContainerTractType::Iterator itContTrct = contTract->Begin();
        itContTrct != contTract->End();
        ++itContTrct)
    {
      // init DTITube point
      DTITubePointType p;
      ContainerPointType cntp = itContTrct->Value();
      p.SetPosition(cntp[0], cntp[1], cntp[2]);
      p.SetRadius(1);

      mitk::FiberBundle::fiberPostprocessing_setTensorMatrix( &p );


      mitk::FiberBundle::fiberPostprocessing_FA( &p );
      //mitk::FiberBundle::fiberPostprocessing_setTensorMatrix( &p );

      p.AddField(DTITubePointType::GA, 3);

      p.SetColor(1,0,0,1);
      list.push_back(p);
    }
    // dtiTubes
    dtiTube->GetProperty()->SetName("dtiTube");
    dtiTube->SetId(itCont->Index());
    dtiTube->SetPoints(list);
    m_GroupFiberBundle->AddSpatialObject(dtiTube);
  }
}

void mitk::FiberBundle::fiberPostprocessing_setTensorMatrix(DTITubePointType *dtiP)
{
  float tMatrix[6];
  tMatrix[0]=0.0f;
  tMatrix[1]=1.0f;
  tMatrix[2]=2.0f;
  tMatrix[3]=3.0f;
  tMatrix[4]=4.0f;
  tMatrix[5]=5.0f;

  dtiP->SetTensorMatrix(tMatrix);


}

/* this method is a HowTo method, used for debugging only */
void mitk::FiberBundle::fiberPostprocessing_setPoint(DTITubePointType *dtiP, ContainerPointType vrtx)
{
  /* get values of variable referenced by a ponter
   double vtxIdx1, vtxIdx2, vtxIdx3;
   vtxIdx1 = * (double*) &vrtx[0];
   vtxIdx2 = * (double*) &vrtx[1];
   vtxIdx3 = * (double*) &vrtx[2];

   dtiP->SetPosition(vtxIdx1, vtxIdx2, vtxIdx3);
   */

  dtiP->SetPosition(vrtx[0], vrtx[1], vrtx[2]);
}

void mitk::FiberBundle::fiberPostprocessing_FA(DTITubePointType *dtiP)
{
  debug_PrototypeCounter ++;
  float valFA;

  if (debug_PrototypeCounter % 10 < 5) {
    valFA = 1.0;
  } else if (debug_PrototypeCounter % 10 == 7) {
    valFA = 0.3;
  } else if (debug_PrototypeCounter % 10 == 8) {
    valFA = 0;
  } else {
    valFA = 0.5;
  }

  dtiP->AddField(DTITubePointType::FA, valFA);

}


/* methods for high speed perfoermance dispay or live-monitoring of fiber results */
void mitk::FiberBundle::addContainer4speedDisplay(ContainerType::Pointer speedTractContainer)
{

}




void mitk::FiberBundle::addSingleDTITract(mitk::FiberBundle::DTITubeType::Pointer dtiFbrTrct)
{

  //MITK_INFO << " start addSingleDTITract(): " << m_GroupFiberBundle->GetNumberOfChildren();
  m_GroupFiberBundle->AddSpatialObject(dtiFbrTrct);
  //MITK_INFO << "end addSingleDTITract(): " << m_GroupFiberBundle->GetNumberOfChildren();


}

mitk::FiberBundle::FiberGroupType::Pointer mitk::FiberBundle::getGroupFiberBundle()
{
  return m_GroupFiberBundle;
}

std::vector<int> mitk::FiberBundle::getAllIDsInFiberBundle()
{
  std::vector<int> allFBIds;

  FiberGroupType::Pointer fiberGroup = this->getGroupFiberBundle();
  ChildrenListType *FiberList;
  FiberList = fiberGroup->GetChildren();

  for(ChildrenListType::iterator itLst = FiberList->begin();
      itLst != FiberList->end();
      ++itLst)
  {

    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());
    if (dtiTract.IsNull()) {
      MITK_INFO << "DTI Tract is NULL!!!!! omg";
      continue; }

    allFBIds.push_back(dtiTract->GetId());

  }

  return allFBIds;


}


mitk::FiberBundle::Pointer mitk::FiberBundle::extractFibersById(std::vector<int> idSet)
{


  mitk::FiberBundle::Pointer resultingFibers = mitk::FiberBundle::New();
  resultingFibers->SetGeometry(this->GetGeometry());
  resultingFibers->SetBounds(this->GetBounds());

  //get Fiber by ID
  //get childrenList
  //compare if current FiberID is in idSet

  FiberGroupType::Pointer fiberGroup = this->getGroupFiberBundle();
  ChildrenListType *FiberList;
  FiberList = fiberGroup->GetChildren();
  MITK_INFO << "writing fibers into datastructure:....";

  for (int cg=0; cg<idSet.size(); ++cg) {

    if(cg % 200 == 0){
      MITK_INFO << "stored: " << cg << " of " << idSet.size();
    }

    int trctId = idSet.at(cg);
    resultingFibers->PushTract( m_TractContainer->GetElement(trctId) );

  }
   MITK_INFO << "init new fiberBundle...";
   resultingFibers->initFiberGroup();
  MITK_INFO << "init new fiberBundle [DONE]";

  /*
   for(ChildrenListType::iterator itLst = FiberList->begin();
   itLst != FiberList->end();
   ++itLst)
   {

   itk::SpatialObject<3>::Pointer tmp_fbr;
   tmp_fbr = *itLst;
   mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());
   if (dtiTract.IsNull()) {
   MITK_INFO << "DTI Tract is NULL!!!!! omg";
   continue; }

   // MITK_INFO << "current DTI tract id: " << dtiTract->GetId();

   std::vector<int>::iterator retFibers = find(idSet.begin(), idSet.end(), dtiTract->GetId());


   if (retFibers != idSet.end()) {
   //MITK_INFO << "Fiber and ID equal: ";


   DTITubeType::Pointer copyDTITract = this->copySingleDTITract(dtiTract);

   MITK_INFO << "fibercontainer before adding new tract" << resultingFibers->GetNumTracts();
   resultingFibers->addSingleDTITract(copyDTITract);
   MITK_INFO << "fibercontainer after adding new tract" << resultingFibers->GetNumTracts();


   }
   //else {
   //  MITK_INFO << "Fiber and ID not ident!";
   //}

   //std::set<int>::iterator retFibers
   //retFibers = idSet.find(dtiTract->GetId());

   } */

  return resultingFibers;

}


/* extract fibers using planar Figures */
//mitk::FiberBundle::Pointer mitk::FiberBundle::extractFibersPF(mitk::PlanarFigure::Pointer pf)
std::vector<int> mitk::FiberBundle::extractFibersByPF(mitk::PlanarFigure::Pointer pf, std::vector<int>* smaller_set)
{

  // if incoming pf is a pfc
  mitk::PlanarFigureComposite::Pointer pfcomp= dynamic_cast<mitk::PlanarFigureComposite*>(pf.GetPointer());
  if (!pfcomp.IsNull()) {
    //PFC


    switch (pfcomp->getOperationType()) {
      case 0:
      {
        //AND
        std::vector<int> childResults = this->extractFibersByPF(pfcomp->getChildAt(0), smaller_set);
        std::vector<int> tmpChild = childResults;
        for (int i=1; i<pfcomp->getNumberOfChildren(); ++i)
        {
          tmpChild = extractFibersByPF(pfcomp->getChildAt(i),&tmpChild);
        }
        std::vector<int> AND_Assamblage(tmpChild.begin(), tmpChild.end());
        return AND_Assamblage;
      }
      case 1:
      {
        //OR
        std::vector<int> childResults = this->extractFibersByPF(pfcomp->getChildAt(0), smaller_set);
        std::sort(childResults.begin(), childResults.end());
        std::vector<int> tmp_container(m_TractContainer->Size());
        std::vector<int>::iterator end;
        for (int i=1; i<pfcomp->getNumberOfChildren(); ++i)
        {
          std::vector<int> tmpChild = extractFibersByPF(pfcomp->getChildAt(i), smaller_set);
          sort(tmpChild.begin(), tmpChild.end());

          end = std::set_union(childResults.begin(), childResults.end(),
                               tmpChild.begin(), tmpChild.end(),
                               tmp_container.begin() );

          childResults.assign(tmp_container.begin(), end);

        }

        std::vector<int> OR_Assamblage(childResults.begin(), childResults.end());
        return OR_Assamblage;
      }
      case 2:
      {
        //NOT

        // FIRST AN AND OPERATION
        std::vector<int> childResults = this->extractFibersByPF(pfcomp->getChildAt(0), smaller_set);
        std::sort(childResults.begin(), childResults.end());
        std::vector<int> tmpChild = childResults;
        for (int i=1; i<pfcomp->getNumberOfChildren(); ++i)
        {
          tmpChild = extractFibersByPF(pfcomp->getChildAt(i),&tmpChild);
        }
        std::vector<int> AND_Assamblage(tmpChild.begin(), tmpChild.end());

        // get IDs of interesting fibers
        std::vector<int> interesting_fibers;
        if(!smaller_set)
          interesting_fibers = this->getAllIDsInFiberBundle();
        else
          interesting_fibers.assign(smaller_set->begin(), smaller_set->end());
        std::sort(interesting_fibers.begin(), interesting_fibers.end());

        // all interesting fibers that are not in the AND assemblage
        std::vector<int> tmp_not(interesting_fibers.size());
        std::vector<int>::iterator end;
        end = std::set_difference(interesting_fibers.begin(), interesting_fibers.end(),
                                  AND_Assamblage.begin(), AND_Assamblage.end(),
                                  tmp_not.begin() );
        std::vector<int> NOT_Assamblage(tmp_not.begin(),end);
        return NOT_Assamblage;

      }
      default:
        MITK_INFO << "we have an UNDEFINED composition... ERROR" ;
        break;


    }

  } else {

    mitk::PlanarCircle::Pointer circleName = mitk::PlanarCircle::New();
    mitk::PlanarRectangle::Pointer rectName = mitk::PlanarRectangle::New();
    mitk::PlanarPolygon::Pointer polyName = mitk::PlanarPolygon::New();

    if (pf->GetNameOfClass() == circleName->GetNameOfClass() )
    {
      //MITK_INFO << "We have a circle :-) " ;


      int cntGaps = 0;
      //init output FiberBundle
      mitk::FiberBundle::Pointer FB_Clipped = mitk::FiberBundle::New();
      FB_Clipped->SetGeometry(this->GetGeometry());

      //get geometry containing the plane which we need to calculate the xrossingPoints
      mitk::Geometry2D::ConstPointer pfgeometry = pf->GetGeometry2D();
      const mitk::PlaneGeometry* planeGeometry = dynamic_cast<const mitk::PlaneGeometry*> (pfgeometry.GetPointer());
      std::vector<mitk::Point3D> XingPoints;
      std::vector<int> fiberIDs;


      //#################################################################
      //############### Find Gap, FrontFace BackFace ####################
      //#################################################################

      //get fibercontainer and iterate through the fibers
      FiberGroupType::Pointer fiberGroup = this->getGroupFiberBundle();

      //extract single fibers
      ChildrenListType *FiberList;
      FiberList = fiberGroup->GetChildren();



      // iterate through each single tract
      //int cntFibId = -1;
      for(ChildrenListType::iterator itLst = FiberList->begin();
          itLst != FiberList->end();
          ++itLst)
      {

        //MITK_INFO << "***************** NEW FIBER *********************";
        //cntFibId++;
        itk::SpatialObject<3>::Pointer tmp_fbr;
        tmp_fbr = *itLst;
        mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());
        if (dtiTract.IsNull()) {
          MITK_INFO << " could not cast groupFiberBundle child into dtiTract!... LEVEL 4 ERROR";
          continue; }

        if (smaller_set && std::find(smaller_set->begin(), smaller_set->end(), dtiTract->GetId())==smaller_set->end())
          continue;

        //get list of points
        int fibrNrPnts = dtiTract->GetNumberOfPoints();
        mitk::FiberBundle::DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();

        // if one fiber is represented by just one point....
        // check if this point is on the plane
        if (fibrNrPnts <= 0)
        {
          MITK_INFO << "HyperERROR in mitkFiberBundle.cpp, No point in fiber....check ur algorithm:";
          continue;
        }


        /* for finding a gap we need to have knowledge of the previous normal
         * be aware that there can be more than 1 gaps!
         */

        int prevPntFacing = -999999;
        int currPntFacing = -999999;
        //double prevFacing = -99999999099;
        mitk::Point3D prevFiberPntmm;

        for(int i=0; i<fibrNrPnts; ++i)
        {

          DTITubePointType tmpFiberPntLst = dtiPntList.at(i);
          DTITubePointType::PointType tmpFiberPnt = tmpFiberPntLst.GetPosition();

          mitk::Point3D tmpFiberVec;
          tmpFiberVec[0] = (float)tmpFiberPnt[0];
          tmpFiberVec[1] = (float)tmpFiberPnt[1];
          tmpFiberVec[2] = (float)tmpFiberPnt[2];


          //calculation has to be done always
          mitk::Point3D currentFiberPntmm;
          this->GetGeometry()->IndexToWorld(tmpFiberVec, currentFiberPntmm);
          double faceing = pfgeometry->SignedDistance(currentFiberPntmm);

          //MITK_INFO << "FacingOutput: " << faceing;

          ///////////////////////////////////////

          if (faceing < 0) {
            //backface
            currPntFacing = TRACTPOINT_BACKFACE;
            //MITK_INFO << "is BACKFACE" << currentFiberPntmm ;

          } else if (faceing > 0) {
            //frontface
            currPntFacing = TRACTPOINT_FRNTFACE;
            // MITK_INFO << "is FRONTFACE" << currentFiberPntmm ;

          } else if (faceing == 0) {
            //onplane
            currPntFacing = TRACTPOINT_ON_PLANE;
            //MITK_INFO << "is on PLANE" << currentFiberPntmm ;
          }


          ////////////////////////////////////////


          if (currPntFacing == TRACTPOINT_ON_PLANE)
          { //strike,
            //calculate distance
            //TODO SOURCE THIS CONTROLSTRUCTURE OUT
            if(isPointInSelection(tmpFiberVec,pf))
            {
              DTITubeType::Pointer copyDTITract = copySingleDTITract(dtiTract);
              //MITK_INFO << "GroupFB extract after copy of Children: " << fiberGroup->GetNumberOfChildren();

              FB_Clipped->addSingleDTITract(copyDTITract);
            }


          } else {
            // Point is BACKFACE or FRONTFACE
            if (i > 0)
            {
              //check if there is a gap between previous and current
              bool isGap = checkForGap(currPntFacing, prevPntFacing);

              if (isGap == true) {
                ++cntGaps;

                //
                mitk::Vector3D LineDirection;
                LineDirection[0] = currentFiberPntmm[0] - prevFiberPntmm[0];
                LineDirection[1] = currentFiberPntmm[1] - prevFiberPntmm[1];
                LineDirection[2] = currentFiberPntmm[2] - prevFiberPntmm[2];


                mitk::Line3D Xingline( prevFiberPntmm, LineDirection   );

                mitk::Point3D intersectionPoint;

                Vector3D planeNormal = planeGeometry->GetNormal();
                planeNormal.Normalize();

                Vector3D lineDirection = Xingline.GetDirection();
                lineDirection.Normalize();

                double t = planeNormal * lineDirection;
                if ( fabs( t ) < eps )
                {

                }

                Vector3D diff;
                diff = planeGeometry->GetOrigin() - Xingline.GetPoint();
                t = ( planeNormal * diff ) / t;

                intersectionPoint = Xingline.GetPoint() + lineDirection * t;
                // bool successXing = planeGeometry->IntersectionPoint( Xingline, intersectionPoint );
                //mitk::Point3D intersectionPointmm;
                //planeGeometry->IndexToWorld(intersectionPoint,intersectionPointmm );

                //  MITK_INFO << "PlaneIntersectionPoint: " << intersectionPoint;

                if (false) {
                  MITK_INFO << " ERROR CALCULATING INTERSECTION POINT.....should not happen!! ";
                }
                //TODO more nice if this if is outside...
                bool pntInROI = isPointInSelection(intersectionPoint,pf);
                if(pntInROI)
                {
                  // MITK_INFO << "POINT ALSO in ROI";

                  // MITK_INFO << "GroupFB extract before copy new object. of Children: " << fiberGroup->GetNumberOfChildren();
                  /* dtiTubeSpatial::Pointer can not occur in 2 GroupSpatialObjects, therefore we have to copy the whole dtiTract of current List and add the copy to the desired FiberBundle */

                  // DTITubeType::Pointer copyDTITract = copySingleDTITract(dtiTract);
                  // MITK_INFO << "GroupFB extract after copy of Children: " << fiberGroup->GetNumberOfChildren();

                  //FB_Clipped->addSingleDTITract(copyDTITract);

                  //MITK_INFO << "GroupFB extract after adding dtiTract to now FB NR. of Children: " << fiberGroup->GetNumberOfChildren();
                  //MITK_INFO << "DTI Tract ID: " << dtiTract->GetId();
                  // MITK_INFO << "size of Vector before pushing: " << fiberIDs.size();
                  fiberIDs.push_back(dtiTract->GetId());
                  // MITK_INFO << "size of Vector after pushing: " << fiberIDs.size();
                  //MITK_INFO << "GroupFB extract after adding dtiTract to now FB NR. of Children: " << fiberGroup->GetNumberOfChildren();
                  break;


                }

              }

              // MITK_INFO << "---no gap---";

            } //endif i>0


          } //endif Facing
          // MITK_INFO << "GroupFB extract end Facing, NR. of Children: " << fiberGroup->GetNumberOfChildren();


          //update point flag
          prevPntFacing = currPntFacing;
          prevFiberPntmm = currentFiberPntmm;
        } //end for fiberPoints
        //MITK_INFO ;<< "GroupFB extract end of single tract, NR. of Children: " << fiberGroup->GetNumberOfChildren();

      } //end for fiberTracts
      // MITK_INFO << "GroupFB extract end of iterating through fiberBundle, NR. of Children: " << fiberGroup->GetNumberOfChildren();

      // MITK_INFO << "selected " << fiberIDs.size() << " fibers " << " | counted gaps: " << cntGaps;
      return fiberIDs;






    } else if (pf->GetNameOfClass() == rectName->GetNameOfClass() ){

      MITK_INFO << "We have a rectangle :-) " ;

    } else if (pf->GetNameOfClass() == polyName->GetNameOfClass() ) {

      //MITK_INFO << "We have a polygon :-) " ;
      int cntGaps = 0;
      //init output FiberBundle
      mitk::FiberBundle::Pointer FB_Clipped = mitk::FiberBundle::New();
      FB_Clipped->SetGeometry(this->GetGeometry());

      //get geometry containing the plane which we need to calculate the xrossingPoints
      mitk::Geometry2D::ConstPointer pfgeometry = pf->GetGeometry2D();
      const mitk::PlaneGeometry* planeGeometry = dynamic_cast<const mitk::PlaneGeometry*> (pfgeometry.GetPointer());
      std::vector<mitk::Point3D> XingPoints;
      std::vector<int> fiberIDs;


      //#################################################################
      //############### Find Gap, FrontFace BackFace ####################
      //#################################################################

      //get fibercontainer and iterate through the fibers
      FiberGroupType::Pointer fiberGroup = this->getGroupFiberBundle();

      //extract single fibers
      ChildrenListType *FiberList;
      FiberList = fiberGroup->GetChildren();
      int nrFibrs = fiberGroup->GetNumberOfChildren();



      // iterate through each single tract
      int cntFib = 1;
      for(ChildrenListType::iterator itLst = FiberList->begin();
          itLst != FiberList->end();
          ++itLst)
      {
          if (cntFib % 500 == 0) {
          MITK_INFO << "================\n prosessed fibers: " << cntFib << " of " << nrFibrs;;
        }
        ++cntFib;



        //MITK_INFO << "***************** NEW FIBER *********************";
        //cntFibId++;
        itk::SpatialObject<3>::Pointer tmp_fbr;
        tmp_fbr = *itLst;
        mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());
        if (dtiTract.IsNull()) {
          MITK_INFO << " could not cast groupFiberBundle child into dtiTract!... LEVEL 4 ERROR";
          continue; }

        if (smaller_set && std::find(smaller_set->begin(), smaller_set->end(), dtiTract->GetId())==smaller_set->end())
          continue;



        //get list of points
        int fibrNrPnts = dtiTract->GetNumberOfPoints();
        mitk::FiberBundle::DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();

        // if one fiber is represented by just one point....
        // check if this point is on the plane
        if (fibrNrPnts <= 0)
        {
          MITK_INFO << "HyperERROR in mitkFiberBundle.cpp, No point in fiber....check ur algorithm:";
          continue;
        }


        /* for finding a gap we need to have knowledge of the previous normal
         * be aware that there can be more than 1 gaps!
         */

        int prevPntFacing = -999999;
        int currPntFacing = -999999;
        //double prevFacing = -99999999099;
        mitk::Point3D prevFiberPntmm;



        for(int i=0; i<fibrNrPnts; ++i)
        {

          DTITubePointType tmpFiberPntLst = dtiPntList.at(i);
          DTITubePointType::PointType tmpFiberPnt = tmpFiberPntLst.GetPosition();

          mitk::Point3D tmpFiberVec;
          tmpFiberVec[0] = (float)tmpFiberPnt[0];
          tmpFiberVec[1] = (float)tmpFiberPnt[1];
          tmpFiberVec[2] = (float)tmpFiberPnt[2];


          //calculation has to be done always
          mitk::Point3D currentFiberPntmm;
          this->GetGeometry()->IndexToWorld(tmpFiberVec, currentFiberPntmm);
          double faceing = pfgeometry->SignedDistance(currentFiberPntmm);

          //MITK_INFO << "FacingOutput: " << faceing;

          ///////////////////////////////////////

          if (faceing < 0) {
            //backface
            currPntFacing = TRACTPOINT_BACKFACE;
            //MITK_INFO << "is BACKFACE" << currentFiberPntmm ;

          } else if (faceing > 0) {
            //frontface
            currPntFacing = TRACTPOINT_FRNTFACE;
            // MITK_INFO << "is FRONTFACE" << currentFiberPntmm ;

          } else if (faceing == 0) {
            //onplane
            currPntFacing = TRACTPOINT_ON_PLANE;
            //MITK_INFO << "is on PLANE" << currentFiberPntmm ;
          }


          ////////////////////////////////////////


          if (currPntFacing == TRACTPOINT_ON_PLANE)
          { //strike,
            //calculate distance
            //TODO SOURCE THIS CONTROLSTRUCTURE OUT
            if(isPointInSelection(tmpFiberVec,pf))
            {
              DTITubeType::Pointer copyDTITract = copySingleDTITract(dtiTract);
              //MITK_INFO << "GroupFB extract after copy of Children: " << fiberGroup->GetNumberOfChildren();

              FB_Clipped->addSingleDTITract(copyDTITract);
            }


          } else {
            // Point is BACKFACE or FRONTFACE
            if (i > 0)
            {
              //check if there is a gap between previous and current
              bool isGap = checkForGap(currPntFacing, prevPntFacing);

              if (isGap == true) {
                ++cntGaps;

                //
                mitk::Vector3D LineDirection;
                LineDirection[0] = currentFiberPntmm[0] - prevFiberPntmm[0];
                LineDirection[1] = currentFiberPntmm[1] - prevFiberPntmm[1];
                LineDirection[2] = currentFiberPntmm[2] - prevFiberPntmm[2];


                mitk::Line3D Xingline( prevFiberPntmm, LineDirection   );

                mitk::Point3D intersectionPoint;

                Vector3D planeNormal = planeGeometry->GetNormal();
                planeNormal.Normalize();

                Vector3D lineDirection = Xingline.GetDirection();
                lineDirection.Normalize();

                double t = planeNormal * lineDirection;
                if ( fabs( t ) < eps )
                {

                }

                Vector3D diff;
                diff = planeGeometry->GetOrigin() - Xingline.GetPoint();
                t = ( planeNormal * diff ) / t;

                intersectionPoint = Xingline.GetPoint() + lineDirection * t;
                // bool successXing = planeGeometry->IntersectionPoint( Xingline, intersectionPoint );
                //mitk::Point3D intersectionPointmm;
                //planeGeometry->IndexToWorld(intersectionPoint,intersectionPointmm );

                //  MITK_INFO << "PlaneIntersectionPoint: " << intersectionPoint;

                if (false) {
                  MITK_INFO << " ERROR CALCULATING INTERSECTION POINT.....should not happen!! ";
                }
                //TODO more nice if this if is outside...
                bool pntInROI = isPointInSelection(intersectionPoint,pf);
                if(pntInROI)
                {
                  // MITK_INFO << "POINT ALSO in ROI";

                  // MITK_INFO << "GroupFB extract before copy new object. of Children: " << fiberGroup->GetNumberOfChildren();
                  /* dtiTubeSpatial::Pointer can not occur in 2 GroupSpatialObjects, therefore we have to copy the whole dtiTract of current List and add the copy to the desired FiberBundle */

                  // DTITubeType::Pointer copyDTITract = copySingleDTITract(dtiTract);
                  // MITK_INFO << "GroupFB extract after copy of Children: " << fiberGroup->GetNumberOfChildren();

                  //FB_Clipped->addSingleDTITract(copyDTITract);

                  //MITK_INFO << "GroupFB extract after adding dtiTract to now FB NR. of Children: " << fiberGroup->GetNumberOfChildren();
                  //MITK_INFO << "DTI Tract ID: " << dtiTract->GetId();
                  // MITK_INFO << "size of Vector before pushing: " << fiberIDs.size();
                  fiberIDs.push_back(dtiTract->GetId());
                  // MITK_INFO << "size of Vector after pushing: " << fiberIDs.size();
                  //MITK_INFO << "GroupFB extract after adding dtiTract to now FB NR. of Children: " << fiberGroup->GetNumberOfChildren();
                  break;


                }

              }

              // MITK_INFO << "---no gap---";

            } //endif i>0


          } //endif Facing
          // MITK_INFO << "GroupFB extract end Facing, NR. of Children: " << fiberGroup->GetNumberOfChildren();


          //update point flag
          prevPntFacing = currPntFacing;
          prevFiberPntmm = currentFiberPntmm;
        } //end for fiberPoints
        //MITK_INFO ;<< "GroupFB extract end of single tract, NR. of Children: " << fiberGroup->GetNumberOfChildren();

      } //end for fiberTracts
      // MITK_INFO << "GroupFB extract end of iterating through fiberBundle, NR. of Children: " << fiberGroup->GetNumberOfChildren();

      // MITK_INFO << "selected " << fiberIDs.size() << " fibers " << " | counted gaps: " << cntGaps;
      return fiberIDs;


    } else {

      MITK_INFO << "[ERROR] unhandled PlanarFigureType found!";
    }

  }










}


mitk::FiberBundle::DTITubeType::Pointer mitk::FiberBundle::copySingleDTITract(mitk::FiberBundle::DTITubeType::Pointer currentDtiFiber)
{

  DTITubeType::Pointer newCopyDtiFiber = DTITubeType::New();
  //*** get ID
  newCopyDtiFiber->SetId( currentDtiFiber->GetId() );

  //*** get points
  DTITubeType::PointListType list;


  //get list of dtiPoints
  int fibrNrPnts = currentDtiFiber->GetNumberOfPoints();
  mitk::FiberBundle::DTITubeType::PointListType dtiPntList = currentDtiFiber->GetPoints();

  for(int i=0; i<fibrNrPnts; ++i)
  {

    DTITubePointType tmpFiberPntLst = dtiPntList.at(i);
    //coordinate Point
    //DTITubePointType::PointType tmpFiberPnt = tmpFiberPntLst.GetPosition();

    DTITubePointType p;
    //p.SetPosition(tmpFiberPnt[0], tmpFiberPnt[1], tmpFiberPnt[2]);
    p.SetPosition(tmpFiberPntLst.GetPosition());
    p.SetRadius(tmpFiberPntLst.GetRadius());
    p.AddField(DTITubePointType::GA, tmpFiberPntLst.GetField(DTITubePointType::GA));
    p.AddField(DTITubePointType::FA, tmpFiberPntLst.GetField(DTITubePointType::FA));
    p.AddField(DTITubePointType::ADC, tmpFiberPntLst.GetField(DTITubePointType::ADC));
    p.SetTensorMatrix(tmpFiberPntLst.GetTensorMatrix());
    p.SetColor(tmpFiberPntLst.GetColor());
    list.push_back(p);

  }

  newCopyDtiFiber->SetPoints(list);
  std::string dtiname = currentDtiFiber->GetProperty()->GetName();
  newCopyDtiFiber->GetProperty()->SetName(dtiname.c_str());



  return newCopyDtiFiber;


}




/* This Method checks
 * To recognize a gap between the 2 points, the previous must differ from the current
 * but if the previous is on the plane and the current one not, then mark this situation as no gap
 * if both, previous and current are on plane, you'll never jump into this Method
 */
bool mitk::FiberBundle::checkForGap(int crntPntFacing, int prevPntFacing)
{

  bool isGap = false;
  if (prevPntFacing != TRACTPOINT_ON_PLANE && crntPntFacing != prevPntFacing)
  {
    isGap = true;

  } else if (prevPntFacing == TRACTPOINT_ON_PLANE && crntPntFacing == TRACTPOINT_ON_PLANE) {
    MITK_INFO << "###########################################";
    MITK_INFO << "$$$ HYPER ERROR, LOGIC MALFUNCTION!! previous point and current point in a fiber are recognized as a potential GAP! THIS IS NOT ALLOWED $$$";
    MITK_INFO << "###########################################";
  }

  return isGap;

}

mitk::Point3D mitk::FiberBundle::calculateCrossingPoint(mitk::Point3D pntFrnt, mitk::Point3D pntbck, mitk::PlanarFigure::Pointer pf)
{

  mitk::Point3D pntXing;

  //#################################################################
  //######### Calculate intersection of plane and fiber #############
  //#################################################################

  // transform in space :-)

  // y = k*x +d
  // k = (y1 - y0)/(x1 - x0)
  // d = ok

  // z adoption, take xy ratio to plane intersection and adopt it to z coordinate
  // z_intersx =  (x1 - intersX)/(x1 - x0) * (z1 - z0) + z1

  double y;
  double k; //slope
  double d;

  double x0 = pntFrnt[0];
  double y0 = pntFrnt[1];
  double z0 = pntFrnt[2];
  double x1 = pntbck[0];
  double y1 = pntbck[1];
  double z1 = pntbck[2];

  k = (y1 - y0) / (x1 - x0);

  // if slope == 0 then leave y as it is, change x

  // if slope == 1 then leave x as it is, change y

  // if z of p0 and p1 is the same, just take z


  /*

   mitk::PlanarCircle::Pointer circleName = mitk::PlanarCircle::New();
   mitk::PlanarRectangle::Pointer rectName = mitk::PlanarRectangle::New();
   mitk::PlanarPolygon::Pointer polyName = mitk::PlanarPolygon::New();

   if (pf->GetNameOfClass() == circleName->GetNameOfClass() )
   {
   MITK_INFO << "We have a circle :-) " ;
   //controlpoint 1 is middlepoint
   //controlpoint 2 is radiuspoint
   mitk::Vector3D V1w = pf->GetWorldControlPoint(0); //centerPoint
   mitk::Vector3D V2w  = pf->GetWorldControlPoint(1); //radiusPoint
   mitk::Vector3D V1;
   mitk::Vector3D V2;
   this->GetGeometry()->WorldToIndex(V1w, V1);
   this->GetGeometry()->WorldToIndex(V2w, V2);

   //calculate distance between those 2 and
   mitk::Point3D distV;
   distV =  V2 - V1;
   distV[0] = sqrt( pow(distV[0], 2.0) );
   distV[1] = sqrt( pow(distV[1], 2.0) );
   distV[2] = sqrt( pow(distV[2], 2.0) );

   mitk::Point3D distPnt;
   distPnt = pnt3D - V1;
   distPnt[0] = sqrt( pow(distPnt[0], 2.0) );
   distPnt[1] = sqrt( pow(distPnt[1], 2.0) );
   distPnt[2] = sqrt( pow(distPnt[2], 2.0) );

   if ( (distPnt <= distV) ) {
   pntIsInside = true;


   }

   return pntIsInside;

   //compare the result to the distance of all points an a fiber





   } else if (pf->GetNameOfClass() == rectName->GetNameOfClass() ){

   MITK_INFO << "We have a rectangle :-) " ;

   } else if (pf->GetNameOfClass() == polyName->GetNameOfClass() ) {

   MITK_INFO << "We have a polygon :-) " ;
   }

   */

  return pntXing;


}


bool mitk::FiberBundle::isPointInSelection(mitk::Point3D pnt3D, mitk::PlanarFigure::Pointer pf)
{
  /* TODO needs to be redesigned.... each time because in planarPolygonsection VTK object will be initialized for each point!...PERFORMANCE LACK!!!!!!! */


  //calculate distances
  mitk::PlanarCircle::Pointer circleName = mitk::PlanarCircle::New();
  mitk::PlanarRectangle::Pointer rectName = mitk::PlanarRectangle::New();
  mitk::PlanarPolygon::Pointer polyName = mitk::PlanarPolygon::New();

  if (pf->GetNameOfClass() == circleName->GetNameOfClass() )
  {
    //MITK_INFO << "We have a circle :-) " ;
    //controlpoint 1 is middlepoint
    //controlpoint 2 is radiuspoint
    mitk::Point3D V1w = pf->GetWorldControlPoint(0); //centerPoint
    mitk::Point3D V2w  = pf->GetWorldControlPoint(1); //radiusPoint
    mitk::Vector3D V1;
    mitk::Vector3D V2;
    // this->GetGeometry()->WorldToIndex(V1w, V1);
    // this->GetGeometry()->WorldToIndex(V2w, V2);

    //calculate distance between those 2 and
    double distPF;
    distPF =  sqrt((double)  (V2w[0] - V1w[0]) * (V2w[0] - V1w[0]) +
                   (V2w[1] - V1w[1]) * (V2w[1] - V1w[1]) +
                   (V2w[2] - V1w[2]) * (V2w[2] - V1w[2]));

    double XdistPnt =  sqrt((double) (pnt3D[0] - V1w[0]) * (pnt3D[0] - V1w[0]) +
                            (pnt3D[1] - V1w[1]) * (pnt3D[1] - V1w[1]) +
                            (pnt3D[2] - V1w[2]) * (pnt3D[2] - V1w[2])) ;



    if ( (distPF > XdistPnt) ) {
      return true;

    }

    return false;

    //compare the result to the distance of all points an a fiber


  }



  else if (pf->GetNameOfClass() == rectName->GetNameOfClass() ){

    MITK_INFO << "We have a rectangle :-) " ;


  } else if (pf->GetNameOfClass() == polyName->GetNameOfClass() ) {


    //create vtkPolygon using controlpoints from planarFigure polygon
    vtkSmartPointer<vtkPolygon> polygonVtk = vtkPolygon::New();


    //get the control points from pf and insert them to vtkPolygon
    unsigned int nrCtrlPnts = pf->GetNumberOfControlPoints();
    //  MITK_INFO << "We have a polygon with " << nrCtrlPnts << " controlpoints: " ;

    for (int i=0; i<nrCtrlPnts; ++i)
    {
      polygonVtk->GetPoints()->InsertNextPoint((double)pf->GetWorldControlPoint(i)[0], (double)pf->GetWorldControlPoint(i)[1], (double)pf->GetWorldControlPoint(i)[2] );


    }

    //prepare everything for using pointInPolygon function
    double n[3];
    polygonVtk->ComputeNormal(polygonVtk->GetPoints()->GetNumberOfPoints(),
                              static_cast<double*>(polygonVtk->GetPoints()->GetData()->GetVoidPointer(0)), n);

    double bounds[6];
    polygonVtk->GetPoints()->GetBounds(bounds);

    double checkIn[3] = {pnt3D[0], pnt3D[1], pnt3D[2]};

    int isInPolygon = polygonVtk->PointInPolygon(checkIn, polygonVtk->GetPoints()->GetNumberOfPoints()
                                                 , static_cast<double*>(polygonVtk->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n);

    //  MITK_INFO << "======IsPointOnPolygon:========\n" << isInPolygon << "\n ======================== ";

    polygonVtk->Delete();

    if (isInPolygon == 1)
    {
      //     MITK_INFO << "return true";
      return true;


    } else if (isInPolygon == 0)
    {
      //   MITK_INFO << "return false";
      return false;

    } else {

      MITK_INFO << "&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*& \n YOUR DRAWN POLYGON DOES IS DEGENERATED AND NOT ACCEPTED \n DRAW A NEW ONE!! HAI CAPITO \n &*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&";
    }





  }


}


void mitk::FiberBundle::debug_members()
{
  /* Debug Workflow description
   * Test1: check if both, groupSpatialObject and Tract container have the same amount of fiberTracts
   * Test2: check if points in tracts contain the same coordinates
   * next... to be continued
   */


  /* ######### TEST 1  START ######### */
  //*************************************
  MITK_INFO << " ############### \n *** TEST Equal Amount of Fibers *** \n ";

  unsigned int groupEls = m_GroupFiberBundle->GetNumberOfChildren();
  unsigned int contEls  = m_TractContainer->Size();
  unsigned int itkcontEls = m_debugITKContainer->Size();

  if (groupEls == contEls && contEls == itkcontEls)
  {
    MITK_INFO << "[OK] .... Test1 passed. # of Fibers: " << groupEls << " \n ******************** ";
  } else {

    MITK_INFO << "[FAIL]: Container and FiberGroup do not contain same amount of fibers! \n ";
    // MITK_INFO << " Container # of Fibers: " << contEls << " | FiberBundle # of Fibers: " << groupEls << "\n";
    MITK_INFO << " # of Fibers m_Cont: " << contEls << " | GroupFibers: " << groupEls <<  " | ITKCont: " << itkcontEls << "\n";
  }
  /* ######### TEST 1  END ######### */
  //***********************************



  /* ######### TEST 2  START ######### */
  //*************************************

  /* iterate through itkcontainer*/
  itkStochTractContainerType::ConstIterator itITKCnt;
  itITKCnt = m_debugITKContainer->Begin();

  /* extract DTIFiberTracts of the GroupFiberBundle Object */
  // all smartPointers to fibers stored in in a ChildrenList
  ChildrenListType * FiberList;
  FiberList =  m_GroupFiberBundle->GetChildren();
  /* iterate through container, itkcontainer groupFiberBundle in one iteration step */
  ChildrenListType::iterator itLst; //STL "list" itself provides no index output of current iteration step.
  itLst = FiberList->begin();


  ContainerType::ConstIterator vecIter;
  for ( vecIter = m_TractContainer->Begin();
       vecIter != m_TractContainer->End();
       vecIter++ )
  {

    unsigned int itIdx = vecIter->Index();
    MITK_INFO << "FiberIteration: " << itIdx <<  "\n" ;

    //get single tract of container
    ContainerTractType::Pointer contTract = vecIter->Value();
    int contNrPnts = contTract->Size();

    //get singel tract of itkContainer
    itkStochTractType::Pointer itkcontTract = itITKCnt->Value();
    itkStochTractType::VertexListType::ConstPointer itkcontVrtx = itkcontTract->GetVertexList();
    int itkcontNrPnts = itkcontVrtx->Size();

    /* lists output is SpatialObject, we know we have DTITubeSpacialObjects
     dynamic cast only likes pointers, no smartpointers, so each dsmartpointer has membermethod .GetPointer() */
    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    DTITubeType::Pointer dtiTract = dynamic_cast<DTITubeType * > (tmp_fbr.GetPointer());
    if (dtiTract.IsNull()) {
      MITK_INFO << " ############### *** ERRROR *** ############### \n ############### *** ERRROR *** ############### \n ############### *** ERRROR *** ############### \n ";
      return; }

    //get points of tract
    int fibrNrPnts = dtiTract->GetNumberOfPoints();
    DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();

    MITK_INFO << " ############### \n *** TEST Equal Amount of Points in Tract *** \n ";

    if (contNrPnts == itkcontNrPnts && itkcontNrPnts == fibrNrPnts) {
      MITK_INFO << "[OK] .... Test2 passed. # of Points in fiber: " << fibrNrPnts << " \n ******************** ";

    } else {
      MITK_INFO << "[FAIL]: Tracts do not contain same amount of points! \n ";
      MITK_INFO << " # of Points m_Cont: " << contNrPnts << " | GroupFibers: " << fibrNrPnts <<  " | ITKCont: " << itkcontNrPnts << "\n";
    }




    //use for()loop with index instead of iterator cuz of accessing more elements, std vectors provide no index output
    for(int ip=0; ip<contNrPnts; ++ip)
    {

      //get point from container
      ContainerPointType tmpcontPnt = contTract->GetElement(ip);

      //get point from itkStochContainer
      itkStochTractType::VertexType itkPnt = itkcontVrtx->GetElement(ip);

      //get point from dtiGroup
      DTITubePointType tmpDtiPnt = dtiPntList.at(ip);
      DTITubePointType::PointType dtiPoint = tmpDtiPnt.GetPosition();


      if (tmpcontPnt[0] == (float)itkPnt[0] && (float)itkPnt[0] == (float)dtiPoint[0]) {
        MITK_INFO << "TractCont | ITKCont | DTIGroup X: " << tmpcontPnt[0] << "...TEST [OK] " << "\n";
      }else{
        MITK_INFO << "TractCont | ITKCont | DTIGroup X: " << tmpcontPnt[0] << " " << itkPnt[0] << " " <<  dtiPoint[0] << "...TEST ##### [FAIL] \n";
      }

      if (tmpcontPnt[1] == (float)itkPnt[1] && (float)itkPnt[1] == (float)dtiPoint[1]) {
        MITK_INFO << "TractCont | ITKCont | DTIGroup Y: " << tmpcontPnt[1] << "...TEST [OK] " << "\n";
      }else{
        MITK_INFO << "TractCont | ITKCont | DTIGroup Y: " << tmpcontPnt[1] << " " << itkPnt[1] << " " <<  dtiPoint[1] << "\n";
      }

      if (tmpcontPnt[2] == (float)itkPnt[2] && (float)itkPnt[2] == (float)dtiPoint[2]) {
        MITK_INFO << "TractCont | ITKCont | DTIGroup Z: " << tmpcontPnt[2] << "...TEST [OK] " << "\n";
      }else{
        MITK_INFO << "TractCont | ITKCont | DTIGroup Z: " << tmpcontPnt[2] << " " << itkPnt[2] << " " <<  dtiPoint[2] << "\n";
      }

    }
    ++itITKCnt;
    ++itLst;
  }

}

vtkSmartPointer<vtkPolyData> mitk::FiberBundle::GeneratePolydata()
{
  MITK_INFO << "writing polydata";
  //extractn single fibers
  //in the groupFiberBundle all smartPointers to single fibers are stored in in a ChildrenList
  mitk::FiberBundle::ChildrenListType * FiberList;
  FiberList =  this->m_GroupFiberBundle->GetChildren();

  /* ######## FIBER PREPARATION END ######### */

  /* ######## VTK FIBER REPRESENTATION  ######## */
  //create a vtkPoints object and store the all the brainFiber points in it
  vtkSmartPointer<vtkPoints> vtkpoints =  vtkPoints::New();

  //in vtkcells all polylines are stored, actually all id's of them are stored
  vtkSmartPointer<vtkCellArray> vtkcells = vtkCellArray::New();

  //in some cases a fiber includes just 1 point, so put it in here
  vtkSmartPointer<vtkCellArray> vtkVrtxs = vtkCellArray::New();

  //colors and alpha value for each single point, RGBA = 4 components
  vtkSmartPointer<vtkUnsignedCharArray> colorsT = vtkUnsignedCharArray::New();
  colorsT->SetNumberOfComponents(4);
  colorsT->SetName("ColorValues");

  vtkSmartPointer<vtkDoubleArray> faColors = vtkDoubleArray::New();
  faColors->SetName("FaColors");

  //vtkDoubleArray *tubeRadius = vtkDoubleArray::New();
  //tubeRadius->SetName("TubeRadius");


  // iterate through FiberList
  for(mitk::FiberBundle::ChildrenListType::iterator itLst = FiberList->begin();
      itLst != FiberList->end();
      ++itLst)
  {
    //all points are stored in one vtkpoints list, soooooooo that the lines find their point id to start and end we need some kind of helper index who monitors the current ids for a polyline
    unsigned long pntIdxHelper = vtkpoints->GetNumberOfPoints();

    // lists output is SpatialObject, we know we have DTITubeSpacialObjects
    // dynamic cast only likes pointers, no smartpointers, so each dsmartpointer has membermethod .GetPointer()
    itk::SpatialObject<3>::Pointer tmp_fbr;
    tmp_fbr = *itLst;
    mitk::FiberBundle::DTITubeType::Pointer dtiTract = dynamic_cast< mitk::FiberBundle::DTITubeType * > (tmp_fbr.GetPointer());
    if (dtiTract.IsNull()) {
      return NULL; }

    //get list of points
    int fibrNrPnts = dtiTract->GetNumberOfPoints();
    mitk::FiberBundle::DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();

    //create a new polyline for a dtiTract
    //smartpointer
    vtkSmartPointer<vtkPolyLine> polyLine = vtkPolyLine::New();
    polyLine->GetPointIds()->SetNumberOfIds(fibrNrPnts);
    unsigned char rgba[4] = {255,255,255,255};

    //tubeRadius->SetNumberOfTuples(fibrNrPnts);
    //double tbradius = 1;//default value for radius

    if (fibrNrPnts <= 0) { //this should never occour! but who knows
      MITK_INFO << "HyperERROR in fiberBundleMapper3D.cpp ...no point in fiberBundle!!! .. check ur trackingAlgorithm";
      continue;
    }


    for (int i=0; i<fibrNrPnts; ++i)
    {
      mitk::FiberBundle::DTITubePointType tmpFiberPntLst = dtiPntList.at(i);
      mitk::FiberBundle::DTITubePointType::PointType tmpFiberPnt = tmpFiberPntLst.GetPosition();

      float tmpvtkPnt[3] = {(float)tmpFiberPnt[0], (float)tmpFiberPnt[1], (float)tmpFiberPnt[2]};

      mitk::Point3D indexPnt(tmpvtkPnt);
      mitk::Point3D worldPnt;

     // MITK_INFO << tmpFiberPnt[0] << " " << tmpFiberPnt[1] << " " << tmpFiberPnt[2];
     // MITK_INFO << worldPnt[0] << " " << worldPnt[1] << " " << worldPnt[2];

      this->GetGeometry()->IndexToWorld(indexPnt, worldPnt);
      double worldFbrPnt[3] = {worldPnt[0], worldPnt[1], worldPnt[2]};
      vtkpoints->InsertNextPoint(worldFbrPnt);

     // tubeRadius->SetTuple1(i,tbradius); //tuple with 1 argument


      if (fibrNrPnts == 1) {
        // if there ist just 1 point in a fiber...wtf, but however represent it as a point
        vtkSmartPointer<vtkVertex> vrtx = vtkVertex::New();
        vrtx->GetPointIds()->SetNumberOfIds(1);
        vrtx->GetPointIds()->SetId(i,i+pntIdxHelper);
        colorsT->InsertNextTupleValue(rgba);
        vtkVrtxs->InsertNextCell(vrtx);

      } else {

        polyLine->GetPointIds()->SetId(i,i+pntIdxHelper);

        //colorcoding orientation based
        if (i<fibrNrPnts-1 && i>0)
        {
        //nimm nur diff1
        mitk::FiberBundle::DTITubePointType nxttmpFiberPntLst = dtiPntList.at(i+1);
        mitk::FiberBundle::DTITubePointType::PointType nxttmpFiberPnt = nxttmpFiberPntLst.GetPosition();
        //double nxttmpvtkPnt[3] = {nxttmpFiberPnt[0], nxttmpFiberPnt[1], nxttmpFiberPnt[2]};

        vnl_vector_fixed< double, 3 > tmpPntvtk((double)tmpvtkPnt[0], (double)tmpvtkPnt[1],(double)tmpvtkPnt[2]);
        vnl_vector_fixed< double, 3 > nxttmpPntvtk(nxttmpFiberPnt[0], nxttmpFiberPnt[1], nxttmpFiberPnt[2]);

        vnl_vector_fixed< double, 3 > diff;
        diff = tmpPntvtk - nxttmpPntvtk;
        diff.normalize();

       rgba[0] = (unsigned char) (255.0 * std::abs(diff[0]));
        rgba[1] = (unsigned char) (255.0 * std::abs(diff[1]));
        rgba[2] = (unsigned char) (255.0 * std::abs(diff[2]));
        rgba[3] = (unsigned char) (255.0);



      } else if(i==0) {
        //explicit handling of startpoint of line

        //nimm nur diff1
        mitk::FiberBundle::DTITubePointType nxttmpFiberPntLst = dtiPntList.at(i+1);
        mitk::FiberBundle::DTITubePointType::PointType nxttmpFiberPnt = nxttmpFiberPntLst.GetPosition();
        //double nxttmpvtkPnt[3] = {nxttmpFiberPnt[0], nxttmpFiberPnt[1], nxttmpFiberPnt[2]};

        vnl_vector_fixed< double, 3 > tmpPntvtk((double)tmpvtkPnt[0], (double)tmpvtkPnt[1],(double)tmpvtkPnt[2]);
        vnl_vector_fixed< double, 3 > nxttmpPntvtk(nxttmpFiberPnt[0], nxttmpFiberPnt[1], nxttmpFiberPnt[2]);

        vnl_vector_fixed< double, 3 > diff;
        diff = tmpPntvtk - nxttmpPntvtk;
        diff.normalize();

        rgba[0] = (unsigned char) (255.0 * std::abs(diff[0]));
         rgba[1] = (unsigned char) (255.0 * std::abs(diff[1]));
         rgba[2] = (unsigned char) (255.0 * std::abs(diff[2]));
         rgba[3] = (unsigned char) (255.0);




      } else if(i==fibrNrPnts) {
        // nimm nur diff2
        mitk::FiberBundle::DTITubePointType nxttmpFiberPntLst = dtiPntList.at(i-1);
        mitk::FiberBundle::DTITubePointType::PointType nxttmpFiberPnt = nxttmpFiberPntLst.GetPosition();

        vnl_vector_fixed< double, 3 > tmpPntvtk((double)tmpvtkPnt[0], (double)tmpvtkPnt[1],(double)tmpvtkPnt[2]);
        vnl_vector_fixed< double, 3 > nxttmpPntvtk(nxttmpFiberPnt[0], nxttmpFiberPnt[1], nxttmpFiberPnt[2]);

        vnl_vector_fixed< double, 3 > diff;
        diff = tmpPntvtk - nxttmpPntvtk;
        diff.normalize();

         rgba[0] = (unsigned char) (255.0 * std::abs(diff[0]));
         rgba[1] = (unsigned char) (255.0 * std::abs(diff[1]));
         rgba[2] = (unsigned char) (255.0 * std::abs(diff[2]));
         rgba[3] = (unsigned char) (255.0);




      }

        colorsT->InsertNextTupleValue(rgba);

        //get FA value
        float faVal = tmpFiberPntLst.GetField(mitk::FiberBundle::DTITubePointType::FA);
        //use insertNextValue cuz FA Values are reperesented as a single number (1 Tuple containing 1 parameter)
        faColors->InsertNextValue((double) faVal);


      }
    }
      vtkcells->InsertNextCell(polyLine);

  }

  //vtkcells->InitTraversal();

  // Put points and lines together in one polyData structure
  m_PolyData = vtkPolyData::New();
  m_PolyData->SetPoints(vtkpoints);
  m_PolyData->SetLines(vtkcells);
  if (vtkVrtxs->GetSize() > 0) {
    m_PolyData->SetVerts(vtkVrtxs);
  }
  m_PolyData->GetPointData()->AddArray(colorsT);
  m_PolyData->GetPointData()->AddArray(faColors);
  return m_PolyData;
}

/* NECESSARY IMPLEMENTATION OF SUPERCLASS METHODS */
void mitk::FiberBundle::UpdateOutputInformation()
{

}
void mitk::FiberBundle::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::FiberBundle::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}
bool mitk::FiberBundle::VerifyRequestedRegion()
{
  return true;
}
void mitk::FiberBundle::SetRequestedRegion( itk::DataObject *data )
{

}


/* TUTORIAL INSERT POINTS / FIBERS to TRACTCONTAINER */
// points and vectors do not need to be initiated but itkVectorContainer
/*ContainerPointType pnt1;
 pnt1[0] = 1.0;
 pnt1[1] = 2.0;
 pnt1[2] = 3.0;

 ContainerPointType pnt2;
 pnt2[0] = 4.0;
 pnt2[1] = 5.0;
 pnt2[2] = 6.0;

 ContainerTractType tract1;
 tract1.push_back(pnt1);
 tract1.push_back(pnt2);

 ContainerType::Pointer testContainer = ContainerType::New();
 unsigned int freeIdx = testContainer->Size();
 MITK_INFO << freeIdx << "\n";
 testContainer->InsertElement(freeIdx, tract1);

 //iterate through all fibers stored in container
 for(ContainerType::ConstIterator itCont = testContainer->Begin();
 itCont != testContainer->End();
 itCont++)
 {
 //get single tract
 ContainerTractType tmp_fiber = itCont->Value();
 //  MITK_INFO << tmp_fiber << "\n";

 //iterate through all points within a fibertract
 for(ContainerTractType::iterator itPnt = tmp_fiber.begin();
 itPnt != tmp_fiber.end();
 ++itPnt)
 {
 // get single point with its coordinates
 ContainerPointType tmp_pntEx = *itPnt;
 MITK_INFO << tmp_pntEx[0] << "\n";
 MITK_INFO << tmp_pntEx[1] << "\n";
 MITK_INFO << tmp_pntEx[2] << "\n";
 }

 }
 ################### DTI FIBERs TUTORIAL ###########################
 TUTORIAL HOW TO READ POINTS / FIBERS from DTIGroupSpatialObjectContainer
 assume our dti fibers are stored in m_GroupFiberBundle

 // all smartPointers to fibers stored in in a ChildrenList
 ChildrenListType * FiberList;
 FiberList =  m_GroupFiberBundle->GetChildren();

 // iterate through container, itkcontainer groupFiberBundle in one iteration step
 for(ChildrenListType::iterator itLst = FiberList->begin();
 itLst != FiberList->end();
 ++FiberList)
 {
 // lists output is SpatialObject, we know we have DTITubeSpacialObjects
 // dynamic cast only likes pointers, no smartpointers, so each dsmartpointer has membermethod .GetPointer()
 itk::SpatialObject<3>::Pointer tmp_fbr;
 tmp_fbr = *itLst;
 DTITubeType::Pointer dtiTract = dynamic_cast<DTITubeType * > (tmp_fbr.GetPointer());
 if (dtiTract.IsNull()) {
 return; }

 //get list of points
 int fibrNrPnts = dtiTract->GetNumberOfPoints();
 DTITubeType::PointListType dtiPntList = dtiTract->GetPoints();

 }


 */

