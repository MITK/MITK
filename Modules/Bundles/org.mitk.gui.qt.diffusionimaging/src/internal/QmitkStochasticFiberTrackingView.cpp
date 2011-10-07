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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include "berryIStructuredSelection.h"


// Qmitk
#include "QmitkStochasticFiberTrackingView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>

//MITK
//#include "mitkNodePredicateProperty.h"
//#include "mitkNodePredicateAND.h"
#include <mitkImageToItk.h>

#include <mitkPointSet.h>

#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyLine.h>



#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <mitkFiberBundle.h>
#include <itkTractsToProbabilityImageFilter.h>

const std::string QmitkStochasticFiberTrackingView::VIEW_ID = "org.mitk.views.stochasticfibertracking";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;

// Strings for Data handling
const QString qDiffStr = "DiffusionImage";
const QString qStatusErr = "[Err]";
const QString qStatusOk = "[OK]";
const QString qSeed = "Seed";


//*****PERSONAL REMINDER******
//##############################
/* THIS CLASS IS A MESS!!
 * CLEANUP IN PROGRESS...
 */
//##############################




QmitkStochasticFiberTrackingView::QmitkStochasticFiberTrackingView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
}

// Destructor
QmitkStochasticFiberTrackingView::~QmitkStochasticFiberTrackingView()
{

}


void QmitkStochasticFiberTrackingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkStochasticFiberTrackingViewControls;
    m_Controls->setupUi( parent );

    // set tableWidget column with
    m_Controls->tableWidget->setColumnWidth(0,38);
    m_Controls->tableWidget->setColumnWidth(1,250);
    m_Controls->tableWidget->setEnabled(false);
    m_Controls->tableWidget->setLineWidth(0);

    connect( m_Controls->commandLinkButton, SIGNAL(clicked()), this, SLOT(DoFiberTracking()) );
    //connect( m_Controls->comboBox_fiberAlgo, SIGNAL(selected()), this, SLOT(handleAlgoSelection() );


  }
}

void QmitkStochasticFiberTrackingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkStochasticFiberTrackingView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

/*  This method refreshes the Status-Message and its corresponding filename of tableWidget
 Input:  tmpVec: contains either dwi-image-DataNode or seedImage-DataNodes
 tmpStatus: flag if dataNode-vector contains correct nodes or not */
void QmitkStochasticFiberTrackingView::refreshTableWidget(std::vector<mitk::DataNode*> tmpVec , QString tmpStatus)
{

  for(int idw=0; idw<(int)tmpVec.size(); idw++){
    mitk::DataNode::Pointer tmpnode;
    QTableWidgetItem *itemStatus = new QTableWidgetItem;
    QTableWidgetItem *itemText = new QTableWidgetItem;
    tmpnode = tmpVec.at(idw);
    itemText->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    itemText->setText( tmpnode->GetName().c_str() );
    itemStatus->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    itemStatus->setText(tmpStatus);
    if(tmpStatus.compare(qStatusErr) == 0){
      itemText->setTextColor(QColor(90, 0, 3, 255));
      itemStatus->setTextColor(QColor(90, 0, 3, 255));
    }else{
      itemText->setTextColor(QColor(92, 198, 10, 255));
      itemStatus->setTextColor(QColor(92, 198, 10, 255));
    }
    /* take next available row */
    int row = m_Controls->tableWidget->rowCount();
    m_Controls->tableWidget->insertRow(row);
    m_Controls->tableWidget->setItem(row,0,itemStatus);
    m_Controls->tableWidget->setItem(row,1,itemText);

  }

}

/* This method checks if given node is validSeed image*/
bool QmitkStochasticFiberTrackingView::checkSeedROI( mitk::DataNode::Pointer node )
{
  bool isSeed = false;
  QString qClassName = node->GetData()->GetNameOfClass();

  /*  propValue contains the result after calling GetPropertyValue
   success contains the bool if desired propertyname is available or not */
  bool propValue = false;
  bool success = node->GetPropertyValue<bool>("binary",propValue);

  if(qClassName.compare("Image") == 0 && success == true && propValue == true)
    isSeed = true;

  return isSeed;
}
/* This method checks if given node is dwi image*/
bool QmitkStochasticFiberTrackingView::checkDWIType( mitk::DataNode::Pointer node )
{
  bool isDwi = false;
  QString qClassName = node->GetData()->GetNameOfClass(); // ie. TensorImage or DiffusionImage
  if(qClassName.compare(qDiffStr) == 0)
    isDwi = true;

  return isDwi;

}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
 implement SelectionService Listener explicitly */

void QmitkStochasticFiberTrackingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  //++++++++RESET GUI AND POINTER VECTORS+++++++++++++++
  /* clear GUI tableWidget */
  for( int i=m_Controls->tableWidget->rowCount(); i >= 0; --i)
  {
    m_Controls->tableWidget->removeRow(i);
  }

  /* reset DWI selection vector */
  vPselDWImg.clear();
  vSeedROI.clear();
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++

  //############### HOMELAND SECURITY ################################
  /* When selection not empty, extract data of interest,
   otherwise return */
  if(!nodes.empty()){

    /* Flag to set commandButton enabled or not
     keep flag true until selection is not DWIImage */
    bool flag_execFTComd = true;

    /* checkpoint for singe Seedpoint track */
    // singleSeedpoint is depricated
    // m_singleSeedpoint = false; //for debugging singleSeedpoint is init here, otherwise get boolean from GUI

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
        it != nodes.end(); ++it )
    {
      //flags needed for algorithm execution logic
      bool isDWI = false;
      bool isSeed = false;
      mitk::DataNode::Pointer node = *it;
      if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
      {
        /* Check and sort image types */
        isDWI = checkDWIType(node);
        if( isDWI ){
          vPselDWImg.push_back(*it);

        } else if(!m_singleSeedpoint){
          isSeed = checkSeedROI(node);
          if( isSeed )
            vSeedROI.push_back(*it);

        } //else single seedPoint

      } //end if node.IsNotNull && dynamic_cast mitkImage
    } //end node iterator


    //############### BORDER CONTROL ################################
    /* ++++++++ Logic to en/disable execution of Algorithm +++++++++++
     Requirements: 1 dwi Image, >=1 Seedimage(s) or 1 single seedpoint */
    if(vPselDWImg.size() > 1){
      flag_execFTComd = false;
      std::cout << "TODO GUI Warning, too much dwi images selected \n";
      refreshTableWidget(vPselDWImg, qStatusErr);

    }else if(vPselDWImg.empty()){
      flag_execFTComd = false;
      std::cout << "TODO GUI Warning, NO dwi image selected \n";

    }else{ // exactly 1 node in vector
      flag_execFTComd = true;
      refreshTableWidget(vPselDWImg, qStatusOk);
    }

    if(!m_singleSeedpoint){
      //no manual selection

      if(vSeedROI.empty()){
        flag_execFTComd = false;
        std::cout << "TODO GUI Warning, NO seed image selected \n";
      }else{
        //color seedImages green
        refreshTableWidget(vSeedROI, qSeed);
      }

    }

    /* Logic to Dis-/Enable Execute-FiberTracking-Button */
    if (m_Controls->commandLinkButton->isEnabled())
    {
      if(!flag_execFTComd)
        m_Controls->commandLinkButton->setEnabled(false);

    } else if(flag_execFTComd){

      m_Controls->commandLinkButton->setEnabled(true);
    }
  } else { // else statement of " if node.isEmpty() "

    /* Selection from datamanager is empty
     therefore set commandButton disable if necessary */
    if(m_Controls->commandLinkButton->isEnabled())
    {
      m_Controls->commandLinkButton->setEnabled(false);
      return;
    }
  }

}



void QmitkStochasticFiberTrackingView::DoFiberTracking()
{

  // for debugging use only first image node
  mitk::DataNode::Pointer SpDWI = vPselDWImg.at(0);




  // cast node to compatible type
  mitk::DiffusionImage<short>::Pointer dwiImg = dynamic_cast< mitk::DiffusionImage<short>* >( SpDWI->GetData() );

  /* get Gradients/Direction of dwi */
  itk::VectorContainer< unsigned int, vnl_vector_fixed<double,3> >::Pointer Pdir = dwiImg->GetDirections();

  /* bValueContainer, Container includes b-values according to corresponding gradient-direction*/
  PTFilterType::bValueContainerType::Pointer vecCont = PTFilterType::bValueContainerType::New();

  /* for each gradient set b-Value; for 0-gradient set b-value eq. 0 */
  for ( int i=0; i<(int)Pdir->size(); ++i)
  {
    vnl_vector_fixed<double,3> valsGrad = Pdir->at(i);
    if (valsGrad.get(0) == 0 && valsGrad.get(1) == 0 && valsGrad.get(2) == 0)
    {  //set 0-Gradient to bValue 0
      vecCont->InsertElement(i,0);
    }else{
      vecCont->InsertElement(i,dwiImg->GetB_Value());
    }
  }

  /* former variant without setting 0-Gradient to 0
   for(int i=0; i<dwiImg->GetDirections()->Size(); i++)
   vecCont->InsertElement(i,dwiImg->GetB_Value());
   */

  /* define measurement frame (identity-matrix 3x3) */
  PTFilterType::MeasurementFrameType measurement_frame;
  measurement_frame.set_identity();

  /* generate white matterImage */
  FloatImageType::Pointer wmImage = FloatImageType::New();
  wmImage->SetSpacing( dwiImg->GetVectorImage()->GetSpacing() );
  wmImage->SetOrigin( dwiImg->GetVectorImage()->GetOrigin() );
  wmImage->SetDirection( dwiImg->GetVectorImage()->GetDirection() );
  wmImage->SetLargestPossibleRegion( dwiImg->GetVectorImage()->GetLargestPossibleRegion() );
  wmImage->SetBufferedRegion( wmImage->GetLargestPossibleRegion() );
  wmImage->SetRequestedRegion( wmImage->GetLargestPossibleRegion() );
  wmImage->Allocate();

  itk::ImageRegionIterator<FloatImageType> ot(wmImage, wmImage->GetLargestPossibleRegion() );
  while (!ot.IsAtEnd())
  {
    ot.Set(1);
    ++ot;
  }

  /* init TractographyFilter, note: it's a smartpointer */
  PTFilterType::Pointer ptfilterPtr = PTFilterType::New();

  ptfilterPtr->SetInput(dwiImg->GetVectorImage().GetPointer());
  ptfilterPtr->SetbValues(vecCont);
  ptfilterPtr->SetGradients(Pdir);
  ptfilterPtr->SetMeasurementFrame(measurement_frame);
  ptfilterPtr->SetWhiteMatterProbabilityImageInput(wmImage);
  /*** get parameters from GUI ... TODO check validity during user input ***/
  ptfilterPtr->SetTotalTracts(m_Controls->lineEdit_totalTracts->text().toInt());
  ptfilterPtr->SetMaxLikelihoodCacheSize(m_Controls->lineEdit_likelihood_cache->text().toInt());
  ptfilterPtr->SetMaxTractLength(m_Controls->lineEdit_maxTractLength->text().toInt());


  /*   define seed Index manual */
  //PTFilterType::InputDWIImageType::IndexType seedIdx;
  //seedIdx[0] = m_Controls->lineEdit_seedIndex1->text().toInt();
  //seedIdx[1] = m_Controls->lineEdit_seedIndex2->text().toInt();
  //seedIdx[2] = m_Controls->lineEdit_seedIndex3->text().toInt();





  /* SEEDPOINT/REGION PREPARATION */
  m_tractcontainer = PTFilterType::TractContainerType::New();



  //int seedROIcnt = vSeedROI.size();

  // ############## SEED OPERATIONS ########
  // todo iterate over all seed ROIs
  /// for(int si=0; si<seedROIcnt; ++si)
  //{
  mitk::DataNode::Pointer SpSeedR = vSeedROI.at(0); //TODO merge all seed regions


  /* prepare seed Image */
  mitk::Image::Pointer mitkSeed = dynamic_cast<mitk::Image*>(SpSeedR->GetData());

  /* nice structure of fibers in datamanager */
/*  mitk::DataNode::Pointer roiParentNode;
  roiParentNode = mitk::DataNode::New();
  roiParentNode->SetName("FiberBundle_ROI_1");
  roiParentNode->SetData(mitkSeed);
  roiParentNode->SetVisibility(true);

  GetDataStorage()->Add(roiParentNode);*/



  //itk::Image< char, 3 >
  mitk::ImageToItk< itk::Image< unsigned char, 3 > >::Pointer  binaryImageToItk1 = mitk::ImageToItk< itk::Image< unsigned char, 3 > >::New();
  binaryImageToItk1->SetInput( mitkSeed );
  binaryImageToItk1->Update();



  itk::ImageRegionConstIterator< BinaryImageType > it(binaryImageToItk1->GetOutput(), binaryImageToItk1->GetOutput()->GetRequestedRegion());
  it.Begin();

  while(!it.IsAtEnd())
  {
    itk::ImageConstIterator<BinaryImageType>::PixelType tmpPxValue = it.Get();

    if(tmpPxValue != 0){
      itk::ImageRegionConstIterator< BinaryImageType >::IndexType seedIdx = it.GetIndex();
      ptfilterPtr->SetSeedIndex(seedIdx);
      ptfilterPtr->Update();

      /* get results from Filter */
      /* write each single tract into member container */
      PTFilterType::TractContainerType::Pointer container_tmp = ptfilterPtr->GetOutputTractContainer();
      CImageType::Pointer cmap = ptfilterPtr->GetOutput();

      PTFilterType::TractContainerType::Iterator elIt = container_tmp->Begin();
      PTFilterType::TractContainerType::Iterator end = container_tmp->End();

      while( elIt != end ){
        PTFilterType::TractContainerType::Element tract_tmp = elIt.Value();
        m_tractcontainer->InsertElement(m_tractcontainer->Size(),tract_tmp);
        ++elIt;
      }
    }
    ++it;

  }
  // }

  /* allocate the VTK Polydata to output the tracts */
  vtkSmartPointer<vtkPoints> pts = vtkSmartPointer<vtkPoints>::New();

  vtkSmartPointer<vtkCellArray> cells = vtkSmartPointer<vtkCellArray>::New();

  float bounds[] = {0,0,0};
  bounds[0] = dwiImg->GetLargestPossibleRegion().GetSize().GetElement(0);
  bounds[1] = dwiImg->GetLargestPossibleRegion().GetSize().GetElement(1);
  bounds[2] = dwiImg->GetLargestPossibleRegion().GetSize().GetElement(2);



  mitk::FiberBundle::Pointer transformer = mitk::FiberBundle::New();
  transformer->additkStochTractContainer(m_tractcontainer);
  transformer->initFiberGroup();
  transformer->SetGeometry(dwiImg->GetGeometry());
  transformer->SetBounds(bounds);


  //transformer->debug_members();

  //testing, get dti Fibers out of group
  mitk::FiberBundle::FiberGroupType::Pointer fiberBundle = transformer->GetGroupFiberBundle();
  mitk::FiberBundle::ChildrenListType *fibersList = fiberBundle->GetChildren();
  for(mitk::FiberBundle::ChildrenListType::iterator itFibers = fibersList->begin();
      itFibers != fibersList->end();
      ++itFibers)
  {
    //we know we have DTITubeSpatialObjects stored in List
    DTITubeType::Pointer dtiTract = dynamic_cast<DTITubeType * > (itFibers->GetPointer());


  }

  mitk::DataNode::Pointer fbNode;
  fbNode = mitk::DataNode::New();
  fbNode->SetData(transformer);
  fbNode->SetName("GroupFinberBundle");
  fbNode->SetVisibility(true);
  GetDataStorage()->Add(fbNode);

  for( int i=0; i<(int)m_tractcontainer->Size(); i++ )
  {

    /* for each iteration create a new dtiTube containing a list of its points */
    DTITubeType::Pointer dtiTube = DTITubeType::New();
    DTITubeType::PointListType list;

    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    mitk::Point3D point;

    PTFilterType::TractContainerType::Element tract =
    m_tractcontainer->GetElement(i);
    PTFilterType::TractContainerType::Element::ObjectType::VertexListType::ConstPointer vertexlist =
    tract->GetVertexList();

    /* create VTK points and PolyLine */

    vtkSmartPointer<vtkPolyLine> polyLine = vtkSmartPointer<vtkPolyLine>::New();
    polyLine->GetPointIds()->SetNumberOfIds((int)vertexlist->Size());

    unsigned long pntIdxHelper = pts->GetNumberOfPoints();

    for( int j=0; j<(int)vertexlist->Size(); j++){

      PTFilterType::TractContainerType::Element::ObjectType::VertexListType::Element vertex =
      vertexlist->GetElement(j);

            float vertex_f[3] = {(float)vertex[0],(float)vertex[1],(float)vertex[2]};
      mitk::Point3D world(vertex_f);

      dwiImg->GetGeometry()->IndexToWorld(world, point);

      /* VTK add Points */
      double vertex_d[3] = {(double)vertex[0],(double)vertex[1],(double)vertex[2]};
      pts->InsertNextPoint(vertex_d);
      polyLine->GetPointIds()->SetId(j,j+pntIdxHelper);


    }


    /* dtiTubes
    dtiTube->GetProperty()->SetName("dtiTube");
    dtiTube->SetId(i);
    dtiTube->SetPoints(list);
    myFiberScene->AddSpatialObject(dtiTube);*/


    /* VTK add line */
    cells->InsertNextCell(polyLine);



  /*mitk::DataNode::Pointer curveNode;
    curveNode = mitk::DataNode::New();
    curveNode->SetData(spline);
    curveNode->SetName("Fiber");
    curveNode->SetVisibility(true);
    GetDataStorage()->Add(curveNode, roiParentNode);
   */

    mitk::Point3D newIdx3D;


  //  std::cout << "cross position world: " << m_MultiWidget->GetCrossPosition() << "\n" ;
    // std::cout << "cross position index: " << newIdx3D << "\n" ;


  }


  /*
  //////////////
  // Klaus' Code

  //////////////////////
  // Generate RGBA Image
  typedef itk::RGBAPixel<unsigned char> OutPixType;

  // run generator
  typedef itk::TractsToProbabilityImageFilter<WMPImageType, OutPixType> ImageGeneratorType;
  ImageGeneratorType::Pointer generator = ImageGeneratorType::New();
  generator->SetInput(wmImage);
  generator->SetTractContainer(m_tractcontainer);
  generator->SetUpsamplingFactor(20);
  generator->SetMinTractLength(10);
  //generator->Update();

  // get result
  typedef itk::Image<OutPixType,3> OutType;
  OutType::Pointer outImg = generator->GetOutput();

  mitk::Image::Pointer img = mitk::Image::New();
  img->InitializeByItk(outImg.GetPointer());
  img->SetVolume(outImg->GetBufferPointer());

  // write to datastorage
  mitk::DataNode::Pointer node1 = mitk::DataNode::New();
  node1->SetData(img);
  node1->SetName("RgbImage");
  node1->SetVisibility(true);

  mitk::LevelWindow opaclevwin;
  opaclevwin.SetRangeMinMax(0,255);
  opaclevwin.SetWindowBounds(0,0);
  mitk::LevelWindowProperty::Pointer prop = mitk::LevelWindowProperty::New(opaclevwin);
  node1->AddProperty( "opaclevelwindow", prop );

  GetDataStorage()->Add(node1);

  ///////////////////////////////
  // Generate unsigned char Image
  typedef unsigned char OutPixType2;

  // run generator
  typedef itk::TractsToProbabilityImageFilter<WMPImageType, OutPixType2> ImageGeneratorType2;
  ImageGeneratorType2::Pointer generator2 = ImageGeneratorType2::New();
  generator2->SetInput(wmImage);
  generator2->SetTractContainer(m_tractcontainer);
  generator2->SetUpsamplingFactor(20);
  generator2->SetMinTractLength(10);
  generator2->Update();

  // get result
  typedef itk::Image<OutPixType2,3> OutType2;
  OutType2::Pointer outImg2 = generator2->GetOutput();

  mitk::Image::Pointer img2 = mitk::Image::New();
  img2->InitializeByItk(outImg2.GetPointer());
  img2->SetVolume(outImg2->GetBufferPointer());

  // to datastorage
  mitk::DataNode::Pointer node2 = mitk::DataNode::New();
  node2->SetData(img2);
  node2->SetName("ProbImage");
  node2->SetVisibility(true);

  mitk::LevelWindow opaclevwin2;
  opaclevwin2.SetRangeMinMax(0,255);
  opaclevwin2.SetWindowBounds(0,0);
  mitk::LevelWindowProperty::Pointer prop2 = mitk::LevelWindowProperty::New(opaclevwin2);
  node2->AddProperty( "opaclevelwindow", prop2 );

  GetDataStorage()->Add(node2);
  */

  GetDataStorage()->Modified();

  m_MultiWidget->RequestUpdate();


  /*
   vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();

   linesPolyData->SetPoints(pts);
   linesPolyData->SetLines(cells);

   //setup actor and mapper
   vtkSmartPointer<vtkPolyDataMapper> mapper =
   vtkSmartPointer<vtkPolyDataMapper>::New();
   mapper->SetInput(linesPolyData);

   vtkSmartPointer<vtkActor> actor =
   vtkSmartPointer<vtkActor>::New();
   actor->SetMapper(mapper);

   //setup render window, renderer, and interactor
   vtkSmartPointer<vtkRenderer> renderer =
   vtkSmartPointer<vtkRenderer>::New();
   vtkSmartPointer<vtkRenderWindow> renderWindow =
   vtkSmartPointer<vtkRenderWindow>::New();
   renderWindow->AddRenderer(renderer);
   vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
   vtkSmartPointer<vtkRenderWindowInteractor>::New();
   renderWindowInteractor->SetRenderWindow(renderWindow);
   renderer->AddActor(actor);

   renderWindow->Render();
   renderWindowInteractor->Start();

   */

}


