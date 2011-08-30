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


// Blueberry application and interaction service
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberBundleDeveloperView.h"
#include <QmitkStdMultiWidget.h> 

// Qt
#include <QTimer>

// MITK
//#include <mitkFiberBundleX.h> //for fiberStructure

//===needed when timeSlicedGeometry is null to invoke rendering mechansims ====
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>


// VTK
#include <vtkPointSource.h> //for randomized FiberStructure
#include <vtkPolyLine.h>  //for fiberStructure
#include <vtkCellArray.h> //for fiberStructure
#include <vtkMatrix4x4.h> //for geometry

//ITK
#include <itkTimeProbe.h>



/*===================================================================================
 * THIS METHOD IMPLEMENTS THE ACTIONS WHICH SHALL BE EXECUTED by the according THREAD
 */
QmitkFiberIDWorker::QmitkFiberIDWorker(QThread* hostingThread, Package4WorkingThread itemPackage)
: m_itemPackage(itemPackage),
m_hostingThread(hostingThread)
{
  
}
void QmitkFiberIDWorker::run()
{

  //accurate time measurement using ITK timeProbe
  itk::TimeProbe clock;
  clock.Start();
  
  m_itemPackage.st_FBX->DoGenerateFiberIds();
  m_itemPackage.st_idGenerateTimer->stop(); //stop fancy Qt-timer in GUI
    
  clock.Stop();
  m_itemPackage.st_Controls->infoTimerGenerateFiberIds->setText( QString::number(clock.GetTotal()) );
  m_hostingThread->quit();

}








// ========= HERE STARTS THE ACTUAL FIBERBUNDLE DEVELOPER VIEW IMPLEMENTATION ======

const std::string QmitkFiberBundleDeveloperView::VIEW_ID = "org.mitk.views.fiberbundledeveloper";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;


QmitkFiberBundleDeveloperView::QmitkFiberBundleDeveloperView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
  m_hostThread = new QThread;
  m_threadInProgress = false;
  
}

// Destructor
QmitkFiberBundleDeveloperView::~QmitkFiberBundleDeveloperView()
{
  m_FiberBundleX->Delete();
  delete m_hostThread;
  delete m_FiberIDGenerator;
  
  // m_idGenerateTimer: no need to delete, is not initialized using "new"
  
}


void QmitkFiberBundleDeveloperView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done in QtDesigner, etc.
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkFiberBundleDeveloperViewControls;
    m_Controls->setupUi( parent );
    
    /*=========INITIALIZE BUTTON CONFIGURATION ================*/
    m_Controls->radioButton_directionX->setEnabled(false);
    m_Controls->radioButton_directionY->setEnabled(false);
    m_Controls->radioButton_directionZ->setEnabled(false);
    m_Controls->buttonGenerateFiberIds->setEnabled(false);
    m_Controls->buttonGenerateFibers->setEnabled(true);
    
    m_Controls->buttonColorFibers->setEnabled(false);//not yet implemented
    m_Controls->buttonSMFibers->setEnabled(false);//not yet implemented
    m_Controls->buttonVtkDecimatePro->setEnabled(false);//not yet implemented
    m_Controls->buttonVtkSmoothPD->setEnabled(false);//not yet implemented
    m_Controls->buttonGenerateTubes->setEnabled(false);//not yet implemented

    
    connect( m_Controls->buttonGenerateFibers, SIGNAL(clicked()), this, SLOT(DoGenerateFibers()) );
    connect( m_Controls->buttonGenerateFiberIds, SIGNAL(pressed()), this, SLOT(DoGenerateFiberIDs()) );
    
    connect( m_Controls->radioButton_directionRandom, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
    connect( m_Controls->radioButton_directionX, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
    connect( m_Controls->radioButton_directionY, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
    connect( m_Controls->radioButton_directionZ, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
    connect( m_Controls->toolBox, SIGNAL(currentChanged ( int ) ), this, SLOT(SelectionChangedToolBox(int)) );
    
    
    
  }
  
  //  Checkpoint for fiber ORIENTATION
  if ( m_DirectionRadios.empty() )
  {
    m_DirectionRadios.insert(0, m_Controls->radioButton_directionRandom);
    m_DirectionRadios.insert(1, m_Controls->radioButton_directionX);
    m_DirectionRadios.insert(2, m_Controls->radioButton_directionY);
    m_DirectionRadios.insert(3, m_Controls->radioButton_directionZ);
  }
  
  // set GUI elements of FiberGenerator to according configuration
  DoUpdateGenerateFibersWidget();
  
  
}
/* THIS METHOD UPDATES ALL GUI ELEMENTS OF QGroupBox DEPENDING ON CURRENTLY SELECTED
 * RADIO BUTTONS 
 */
void QmitkFiberBundleDeveloperView::DoUpdateGenerateFibersWidget()
{
  //get selected radioButton
  QString fibDirection; //stores the object_name of selected radiobutton 
  QVector<QRadioButton*>::const_iterator i;
  for (i = m_DirectionRadios.begin(); i != m_DirectionRadios.end(); ++i) 
  {
    QRadioButton* rdbtn = *i;
    if (rdbtn->isChecked())
      fibDirection = rdbtn->objectName();
  }
  
  if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_RANDOM ) {
    // disable radiobuttons
    if (m_Controls->boxFiberMinLength->isEnabled())
      m_Controls->boxFiberMinLength->setEnabled(false);
    
    if (m_Controls->labelFiberMinLength->isEnabled())
      m_Controls->labelFiberMinLength->setEnabled(false);
    
    if (m_Controls->boxFiberMaxLength->isEnabled())
      m_Controls->boxFiberMaxLength->setEnabled(false);
    
    if (m_Controls->labelFiberMaxLength->isEnabled())
      m_Controls->labelFiberMaxLength->setEnabled(false);
    
    //enable radiobuttons
    if (!m_Controls->labelFibersTotal->isEnabled())
      m_Controls->labelFibersTotal->setEnabled(true);
    
    if (!m_Controls->boxFiberNumbers->isEnabled())
      m_Controls->boxFiberNumbers->setEnabled(true);
    
    if (!m_Controls->labelDistrRadius->isEnabled())
      m_Controls->labelDistrRadius->setEnabled(true);
    
    if (!m_Controls->boxDistributionRadius->isEnabled())
      m_Controls->boxDistributionRadius->setEnabled(true);
    
    
  } else {
    // disable radiobuttons    
    if (m_Controls->labelDistrRadius->isEnabled())
      m_Controls->labelDistrRadius->setEnabled(false);
    
    if (m_Controls->boxDistributionRadius->isEnabled())
      m_Controls->boxDistributionRadius->setEnabled(false);
    
    
    //enable radiobuttons
    if (!m_Controls->labelFibersTotal->isEnabled())
      m_Controls->labelFibersTotal->setEnabled(true);
    
    if (!m_Controls->boxFiberNumbers->isEnabled())
      m_Controls->boxFiberNumbers->setEnabled(true);
    
    if (!m_Controls->boxFiberMinLength->isEnabled())
      m_Controls->boxFiberMinLength->setEnabled(true);
    
    if (!m_Controls->labelFiberMinLength->isEnabled())
      m_Controls->labelFiberMinLength->setEnabled(true);
    
    if (!m_Controls->boxFiberMaxLength->isEnabled())
      m_Controls->boxFiberMaxLength->setEnabled(true);
    
    if (!m_Controls->labelFiberMaxLength->isEnabled())
      m_Controls->labelFiberMaxLength->setEnabled(true);
    
  }   
  
}

void QmitkFiberBundleDeveloperView::DoGenerateFibers()
{
  
  // GET SELECTED FIBER DIRECTION
  QString fibDirection; //stores the object_name of selected radiobutton 
  QVector<QRadioButton*>::const_iterator i;
  for (i = m_DirectionRadios.begin(); i != m_DirectionRadios.end(); ++i) 
  {
    QRadioButton* rdbtn = *i;
    if (rdbtn->isChecked())
      fibDirection = rdbtn->objectName();
  }
  
  vtkPolyData* output; // FiberPD stores the generated PolyData
  if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_RANDOM ) {
    //    build polydata with random lines and fibers
    output = GenerateVtkFibersRandom();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_X ) {
    //    build polydata with XDirection fibers
    //output = GenerateVtkFibersDirectionX();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_Y ) {
    //    build polydata with YDirection fibers
    // output = GenerateVtkFibersDirectionY();
    
  } else if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_Z ) {
    //    build polydata with ZDirection fibers
    //  output = GenerateVtkFibersDirectionZ();
    
  }
  
  mitk::FiberBundleX::Pointer FB = mitk::FiberBundleX::New();
  FB->SetFibers(output);
  FB->SetGeometry(this->GenerateStandardGeometryForMITK());
  
  mitk::DataNode::Pointer FBNode;
  FBNode = mitk::DataNode::New();
  FBNode->SetName("FiberBundleX");
  FBNode->SetData(FB);
  FBNode->SetVisibility(true);
  
  GetDataStorage()->Add(FBNode);
  //output->Delete();
  
  const mitk::PlaneGeometry * tsgeo = m_MultiWidget->GetTimeNavigationController()->GetCurrentPlaneGeometry();	
  if (tsgeo == NULL) {
    /* GetDataStorage()->Modified etc. have no effect, therefore proceed as followed below */
    // get all nodes that have not set "includeInBoundingBox" to false
    mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
                                                                                                        , mitk::BoolProperty::New(false)));
    mitk::DataStorage::SetOfObjects::ConstPointer rs = GetDataStorage()->GetSubset(pred);
    // calculate bounding geometry of these nodes
    mitk::TimeSlicedGeometry::Pointer bounds = GetDataStorage()->ComputeBoundingGeometry3D(rs);
    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
  } else {
    
    GetDataStorage()->Modified();
    m_MultiWidget->RequestUpdate(); //necessary??
  }
} 


/*
 * Generate polydata of random fibers
 */
vtkPolyData* QmitkFiberBundleDeveloperView::GenerateVtkFibersRandom()
{
  int numOfFibers = m_Controls->boxFiberNumbers->value();
  int distrRadius = m_Controls->boxDistributionRadius->value();
  int numOfPoints = numOfFibers * distrRadius;
  
  
  std::vector< std::vector<int> > fiberStorage;
  for (int i=0; i<numOfFibers; ++i) {
    std::vector<int> a;
    fiberStorage.push_back( a );
    
  }
  
  /* Generate Point Cloud */
  vtkSmartPointer<vtkPointSource> randomPoints = vtkSmartPointer<vtkPointSource>::New();
  randomPoints->SetCenter(0.0, 0.0, 0.0);
  randomPoints->SetNumberOfPoints(numOfPoints);
  randomPoints->SetRadius(distrRadius);
  randomPoints->Update();
  
  vtkPoints* pnts = randomPoints->GetOutput()->GetPoints();
  
  /*====== checkpoint: compare initialized and required points =============================*/
  if (numOfPoints != pnts->GetNumberOfPoints()) {
    MITK_INFO << "VTK POINT ERROR, WRONG AMOUNT OF GENRERATED POINTS... COULD BE VTK BUG OR BITFLIP DUE TO COSMIC RADIATION!";
    return NULL;
  }/* ================================= */
  
  /* ASSIGN EACH POINT TO A RANDOM FIBER */
  srand((unsigned)time(0)); // init randomizer
  for (int i=0; i<pnts->GetNumberOfPoints(); ++i) {
    
    //generate random number between 0 and numOfFibers-1
    int random_integer; 
    random_integer = (rand()%numOfFibers); 
    
    //add current point to random fiber
    fiberStorage.at(random_integer).push_back(i); 
    //    MITK_INFO << "point" << i << " |" << pnts->GetPoint(random_integer)[0] << "|" << pnts->GetPoint(random_integer)[1]<< "|" << pnts->GetPoint(random_integer)[2] << "| into fiber" << random_integer;
  } 
  
  //=====timer measurement====
  itk::TimeProbe clock;
  clock.Start();
  //==========================
  
  
  /* GENERATE VTK POLYLINES OUT OF FIBERSTORAGE */
  vtkSmartPointer<vtkCellArray> linesCell = vtkSmartPointer<vtkCellArray>::New(); // Host vtkPolyLines
  linesCell->Allocate(pnts->GetNumberOfPoints()*2); //allocate for each cellindex also space for the pointId, e.g. [idx | pntID]
  for (unsigned long i=0; i<fiberStorage.size(); ++i)
  {
    
    std::vector<int> singleFiber = fiberStorage.at(i);
    vtkSmartPointer<vtkPolyLine> fiber = vtkSmartPointer<vtkPolyLine>::New();
    fiber->GetPointIds()->SetNumberOfIds((int)singleFiber.size());
    
    for (unsigned long si=0; si<singleFiber.size(); ++si) 
    {  //hopefully unsigned long to double works fine in VTK ;-)
      fiber->GetPointIds()->SetId( si, singleFiber.at(si) );
    }
    
    linesCell->InsertNextCell(fiber);
  }  
  
  /* =======checkpoint for cellarray allocation ==========*/
  if ( (linesCell->GetSize()/pnts->GetNumberOfPoints()) != 2 ) 
  {
    MITK_INFO << "RANDOM FIBER ALLOCATION CAN NOT BE TRUSTED ANYMORE! Correct leak or remove command: linesCell->Allocate(pnts->GetNumberOfPoints()*2) but be aware of possible loss in performance.";
  }/* ====================================================*/
  
  
  
  /* HOSTING POLYDATA FOR RANDOM FIBERSTRUCTURE */
  vtkPolyData* PDRandom = vtkPolyData::New();
  PDRandom->SetPoints(pnts);
  PDRandom->SetLines(linesCell);
  
  //====timer measurement========
  clock.Stop();
  m_Controls->infoTimerGenerateFiberBundle->setText( QString::number(clock.GetTotal()) );
 // MITK_INFO << "=====Assambling random Fibers to Polydata======\nMean: " << clock.GetMean() << std::endl;
 // MITK_INFO << "Total: " << clock.GetTotal() << std::endl;  
  //=============================
  
  
  return PDRandom;
}


vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionX()
{
  int numOfFibers = m_Controls->boxFiberNumbers->value();  
  vtkSmartPointer<vtkCellArray> linesCell = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  
  //insert Origin point, this point has index 0 in point array
  double originX = 0.0;
  double originY = 0.0;
  double originZ = 0.0;
  
  //after each iteration the origin of the new fiber increases
  //here you set which direction is affected.
  double increaseX = 0.0;
  double increaseY = 1.0;
  double increaseZ = 0.0;
  
  
  //walk along X axis
  //length of fibers increases in each iteration
  for (int i=0; i<numOfFibers; ++i) {
    
    vtkSmartPointer<vtkPolyLine> newFiber = vtkSmartPointer<vtkPolyLine>::New();
    newFiber->GetPointIds()->SetNumberOfIds(i+2);
    
    //create starting point and add it to pointset
    points->InsertNextPoint(originX + (double)i * increaseX , originY + (double)i * increaseY, originZ + (double)i * increaseZ);
    
    //add starting point to fiber
    newFiber->GetPointIds()->SetId(0,points->GetNumberOfPoints()-1);
    
    //insert remaining points for fiber
    for (int pj=0; pj<=i ; ++pj) 
    { //generate next point on X axis
      points->InsertNextPoint( originX + (double)pj+1 , originY + (double)i * increaseY, originZ + (double)i * increaseZ );
      newFiber->GetPointIds()->SetId(pj+1,points->GetNumberOfPoints()-1);
    }
    
    linesCell->InsertNextCell(newFiber);
  }
  
  vtkSmartPointer<vtkPolyData> PDX = vtkSmartPointer<vtkPolyData>::New(); 
  PDX->SetPoints(points);
  PDX->SetLines(linesCell);
  return PDX;
}

vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionY()
{
  vtkSmartPointer<vtkPolyData> PDY = vtkSmartPointer<vtkPolyData>::New();
  //todo
  
  
  
  return PDY;
}


vtkSmartPointer<vtkPolyData> QmitkFiberBundleDeveloperView::GenerateVtkFibersDirectionZ()
{
  vtkSmartPointer<vtkPolyData> PDZ = vtkSmartPointer<vtkPolyData>::New();
  //todo
  
  
  
  return PDZ;
}

/* === OutSourcedMethod: THIS METHOD GENERATES ESSENTIAL GEOMETRY PARAMETERS FOR THE MITK FRAMEWORK ===
 * WITHOUT, the rendering mechanism will ignore objects without valid Geometry 
 * for each object, MITK requires: ORIGIN, SPACING, TRANSFORM MATRIX, BOUNDING-BOX */
mitk::Geometry3D::Pointer QmitkFiberBundleDeveloperView::GenerateStandardGeometryForMITK()
{
  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  
  // generate origin
  mitk::Point3D origin;
  origin[0] = 0;
  origin[1] = 0;
  origin[2] = 0;
  geometry->SetOrigin(origin);
  
  // generate spacing
  float spacing[3] = {1,1,1};
  geometry->SetSpacing(spacing);
  
  
  // generate identity transform-matrix
  vtkMatrix4x4* m = vtkMatrix4x4::New();
  geometry->SetIndexToWorldTransformByVtkMatrix(m);
  
  // generate boundingbox
  // for an usable bounding-box use gui parameters to estimate the boundingbox
  float bounds[] = {500, 500, 500, -500, -500, -500};
  
  // GET SELECTED FIBER DIRECTION
  QString fibDirection; //stores the object_name of selected radiobutton 
  QVector<QRadioButton*>::const_iterator i;
  for (i = m_DirectionRadios.begin(); i != m_DirectionRadios.end(); ++i) 
  {
    QRadioButton* rdbtn = *i;
    if (rdbtn->isChecked())
      fibDirection = rdbtn->objectName();
  }
  
  if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_RANDOM ) {
    // use information about distribution parameter to calculate bounding box
    int distrRadius = m_Controls->boxDistributionRadius->value();
    bounds[0] = distrRadius; 
    bounds[1] = distrRadius;
    bounds[2] = distrRadius;
    bounds[3] = -distrRadius;
    bounds[4] = -distrRadius;
    bounds[5] = -distrRadius;
    
  } else {
    // so far only X,Y,Z directions are available
    MITK_INFO << "_______GEOMETRY ISSUE_____\n***BoundingBox for X, Y, Z fiber directions are not optimized yet!***";
    
    int maxFibLength = m_Controls->boxFiberMaxLength->value();
    bounds[0] = maxFibLength;
    bounds[1] = maxFibLength;
    bounds[2] = maxFibLength;
    bounds[3] = -maxFibLength;
    bounds[4] = -maxFibLength;
    bounds[5] = -maxFibLength;
  }
  
  geometry->SetFloatBounds(bounds);
  geometry->SetImageGeometry(true); //??
  
  return geometry;
  
  
}

void QmitkFiberBundleDeveloperView::UpdateFiberIDTimer()
{
  //MAKE SURE by yourself THAT NOTHING ELSE THAN A NUMBER IS SET IN THAT LABEL
  QString crntValue = m_Controls->infoTimerGenerateFiberIds->text();
  int tmpVal = crntValue.toInt();
  m_Controls->infoTimerGenerateFiberIds->setText(QString::number(++tmpVal));  
  m_Controls->infoTimerGenerateFiberIds->update();
  
}

/* Initialie ID dataset in FiberBundleX */
void QmitkFiberBundleDeveloperView::DoGenerateFiberIDs()
{
  
  struct Package4WorkingThread FiberIdPackage;
  FiberIdPackage.st_FBX = m_FiberBundleX;
  FiberIdPackage.st_idGenerateTimer = &m_idGenerateTimer;
  FiberIdPackage.st_Controls = m_Controls;
  
  /* ===== TIMER CONFIGURATIONS for visual effect ======
   * start and stop is called in Thread pre- and postprocessing methods */
  m_idGenerateTimer.setInterval( 10 );
  connect( &m_idGenerateTimer, SIGNAL(timeout()), this, SLOT(UpdateFiberIDTimer()) );
  
  // THREAD CONFIGURATION
  m_FiberIDGenerator = new QmitkFiberIDWorker(m_hostThread, FiberIdPackage);
  m_FiberIDGenerator->moveToThread(m_hostThread);
  connect(m_hostThread, SIGNAL(started()), this, SLOT( BeforeThread_IdGenerate()) );
  connect(m_hostThread, SIGNAL(started()), m_FiberIDGenerator, SLOT(run()));
  connect(m_hostThread, SIGNAL(finished()), this, SLOT(AfterThread_IdGenerate()));
  connect(m_hostThread, SIGNAL(terminated()), this, SLOT(AfterThread_IdGenerate()));
  m_hostThread->start(QThread::LowestPriority);
  
  
  
  // m_Controls->infoTimerGenerateFiberIds->setText(QString::number(clock.GetTotal()));
  
}

void QmitkFiberBundleDeveloperView::BeforeThread_IdGenerate()
{
  m_Controls->infoTimerGenerateFiberIds->setText(QString::number(0)); //set GUI representation of timer to 0
  m_threadInProgress = true;
  m_idGenerateTimer.start();
  
}

void QmitkFiberBundleDeveloperView::AfterThread_IdGenerate()
{
  //  m_idGenerateTimer.stop(); //not smart to set timer.stop here, cuz this will override the actual measurements of ITK:PROBE in GUI 
  m_threadInProgress = false;
  
  
}

void  QmitkFiberBundleDeveloperView::ResetFiberInfoWidget()
{
  if (m_Controls->infoAnalyseNumOfFibers->isEnabled()) {
    m_Controls->infoAnalyseNumOfFibers->setText("-");
    m_Controls->infoAnalyseNumOfPoints->setText("-");
    m_Controls->infoAnalyseNumOfFibers->setEnabled(false);
  }
}

void  QmitkFiberBundleDeveloperView::FeedFiberInfoWidget()
{
  if (!m_Controls->infoAnalyseNumOfFibers->isEnabled())
    m_Controls->infoAnalyseNumOfFibers->setEnabled(true);
  
  QString numOfFibers;
  numOfFibers.setNum( m_FiberBundleX->GetFibers()->GetNumberOfLines() );
  QString numOfPoints;
  numOfPoints.setNum( m_FiberBundleX->GetFibers()->GetNumberOfPoints() );
  
  m_Controls->infoAnalyseNumOfFibers->setText( numOfFibers );
  m_Controls->infoAnalyseNumOfPoints->setText( numOfPoints );
}

void QmitkFiberBundleDeveloperView::SelectionChangedToolBox(int idx)
{
  MITK_INFO << "printtoolbox: " << idx;
  if (m_Controls->page_FiberInfo->isVisible() && m_FiberBundleX != NULL)
  {
    FeedFiberInfoWidget();
    
  } else {
    //if infolables are disabled: return
    //else set info back to - and set label and info to disabled
    
    if (!m_Controls->page_FiberInfo->isVisible()) {
      return;
    } else {
      ResetFiberInfoWidget();
    }
  }
  
}

void QmitkFiberBundleDeveloperView::FBXDependendGUIElementsConfigurator(bool isVisible)
{
  // ==== FIBER PROCESSING ELEMENTS ======
  m_Controls->buttonGenerateFiberIds->setEnabled(isVisible);
  
  
}



void QmitkFiberBundleDeveloperView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkFiberBundleDeveloperView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

/* OnSelectionChanged is registered to SelectionService, therefore no need to
 implement SelectionService Listener explicitly */
void QmitkFiberBundleDeveloperView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{ 
  
  /* ==== reset everyhing related to FiberBundleX ======
   * - variable m_FiberBundleX
   * - visualization of analysed fiberbundle
   */
  m_FiberBundleX = NULL; //reset pointer, so that member does not point to depricated locations
  ResetFiberInfoWidget();
  FBXDependendGUIElementsConfigurator(false);
  m_Controls->infoTimerGenerateFiberIds->setText("-"); //set GUI representation of timer to -
  m_Controls->infoTimerGenerateFiberBundle->setText( "-" );
  //====================================================
  
  
  if (nodes.empty())
    return;
  
  
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;
    
    /* CHECKPOINT: FIBERBUNDLE*/
    if( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
    {
      m_FiberBundleX = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
      if (m_FiberBundleX == NULL)
        MITK_INFO << "========ATTENTION=========\n unable to load selected FiberBundleX to FiberBundleDeveloper-plugin \n";
      
      // ==== FIBERBUNDLE_INFO ELEMENTS ====
      if ( m_Controls->page_FiberInfo->isVisible() )
        FeedFiberInfoWidget(); 
      
      
      // enable FiberBundleX related Gui Elements, such as buttons etc.
      FBXDependendGUIElementsConfigurator(true);
      
    }
    
    
    
    
  }
}
void QmitkFiberBundleDeveloperView::Activated()
{
  
  MITK_INFO << "FB DevelopersV ACTIVATED()";
  
  
}



