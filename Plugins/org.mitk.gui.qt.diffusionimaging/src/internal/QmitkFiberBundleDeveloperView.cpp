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
//=========FOR TESTING==========
//random generation, number of points equal requested points



// Blueberry application and interaction service
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkFiberBundleDeveloperView.h"
#include <QmitkStdMultiWidget.h>

// Qt
#include <QTimer>

// MITK
#include <mitkPlanarCircle.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkGlobalInteraction.h>

//===needed when timeGeometry is null to invoke rendering mechansims ====
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include "mitkModuleRegistry.h"


// VTK
#include <vtkPointSource.h> //for randomized FiberStructure
#include <vtkPolyLine.h>  //for fiberStructure
#include <vtkCellArray.h> //for fiberStructure
#include <vtkMatrix4x4.h> //for geometry

//ITK
#include <itkTimeProbe.h>

//==============================================
//======== W O R K E R S ____ S T A R T ========
//==============================================
/*===================================================================================
 * THIS METHOD IMPLEMENTS THE ACTIONS WHICH SHALL BE EXECUTED by the according THREAD
 * --generate FiberIDs--*/

QmitkFiberIDWorker::QmitkFiberIDWorker(QThread* hostingThread, Package4WorkingThread itemPackage)
    : m_itemPackage(itemPackage),
      m_hostingThread(hostingThread)
{

}
void QmitkFiberIDWorker::run()
{
    if(m_itemPackage.st_Controls->checkBoxMonitorFiberThreads->isChecked())
        m_itemPackage.st_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_RUNNING);

    /* MEASUREMENTS AND FANCY GUI EFFECTS
   * accurate time measurement using ITK timeProbe*/
    itk::TimeProbe clock;
    clock.Start();
    //set GUI representation of timer to 0, is essential for correct timer incrementation
    m_itemPackage.st_Controls->infoTimerGenerateFiberIds->setText(QString::number(0));
    m_itemPackage.st_FancyGUITimer1->start();

    //do processing
    m_itemPackage.st_FBX->GenerateFiberIds();

    /* MEASUREMENTS AND FANCY GUI EFFECTS CLEANUP */
    clock.Stop();
    m_itemPackage.st_FancyGUITimer1->stop();
    m_itemPackage.st_Controls->infoTimerGenerateFiberIds->setText( QString::number(clock.GetTotal()) );
    delete m_itemPackage.st_FancyGUITimer1; // fancy timer is not needed anymore
    m_hostingThread->quit();

}

/*===================================================================================
 * THIS METHOD IMPLEMENTS THE ACTIONS WHICH SHALL BE EXECUTED by the according THREAD
 * -- extract fibers by given PlanarFigure --*/
QmitkFiberExtractorWorker::QmitkFiberExtractorWorker(QThread* hostingThread, Package4WorkingThread itemPackage)
    : m_itemPackage(itemPackage),
      m_hostingThread(hostingThread)
{

}
void QmitkFiberExtractorWorker::run()
{
    if(m_itemPackage.st_Controls->checkBoxMonitorFiberThreads->isChecked())
        m_itemPackage.st_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_RUNNING);

    /* MEASUREMENTS AND FANCY GUI EFFECTS
   * accurate time measurement using ITK timeProbe*/
    itk::TimeProbe clock;
    clock.Start();
    //set GUI representation of timer to 0, is essential for correct timer incrementation
    m_itemPackage.st_Controls->infoTimerExtractFibers->setText(QString::number(0));
    m_itemPackage.st_FancyGUITimer1->start();

    //do processing
    std::vector<long> fibIds = m_itemPackage.st_FBX->ExtractFiberIdSubset(m_itemPackage.st_PlanarFigure);

    //generate new fiberbundle by fiber iDs
    vtkSmartPointer<vtkPolyData> newFBPolyData = m_itemPackage.st_FBX->GeneratePolyDataByIds(fibIds);

    // call function to convert fiberstructure into fiberbundleX and pass it to datastorage
    (m_itemPackage.st_host->*m_itemPackage.st_pntr_to_Method_PutFibersToDataStorage)(newFBPolyData);


    /* MEASUREMENTS AND FANCY GUI EFFECTS CLEANUP */
    clock.Stop();
    m_itemPackage.st_FancyGUITimer1->stop();
    m_itemPackage.st_Controls->infoTimerExtractFibers->setText( QString::number(clock.GetTotal()) );
    delete m_itemPackage.st_FancyGUITimer1; // fancy timer is not needed anymore
    m_hostingThread->quit();

}

/*===================================================================================
 * THIS METHOD IMPLEMENTS THE ACTIONS WHICH SHALL BE EXECUTED by the according THREAD
 * --set FA values to fiberbundle--*/
QmitkFiberColoringWorker::QmitkFiberColoringWorker(QThread* hostingThread, Package4WorkingThread itemPackage)
    : m_itemPackage(itemPackage)
    , m_hostingThread(hostingThread)
{

}
void QmitkFiberColoringWorker::run()
{
    if(m_itemPackage.st_Controls->checkBoxMonitorFiberThreads->isChecked())
        m_itemPackage.st_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_RUNNING);

    /* MEASUREMENTS AND FANCY GUI EFFECTS
   * accurate time measurement using ITK timeProbe*/
    itk::TimeProbe clock;
    clock.Start();

    //set GUI representation of timer to 0, is essential for correct timer incrementation
    m_itemPackage.st_Controls->infoTimerColorCoding->setText(QString::number(0));
    m_itemPackage.st_FancyGUITimer1->start();

    //do processing
    if(m_itemPackage.st_Controls->radioButton_ColorOrient->isChecked()) {
        m_itemPackage.st_FBX->DoColorCodingOrientationBased();

    } else if(m_itemPackage.st_Controls->radioButton_ColorFA->isChecked()) {
        m_itemPackage.st_FBX->DoColorCodingFaBased();

    } else if(m_itemPackage.st_Controls->radioButton_OpacityFA->isChecked()) {
        //        m_itemPackage.st_FBX->SetColorCoding("");
        m_itemPackage.st_PassedDataNode->SetOpacity(0.999);
        m_itemPackage.st_FBX->DoUseFaFiberOpacity();
    } else if(m_itemPackage.st_Controls->radioButton_ColorCustom->isChecked()){
        m_itemPackage.st_FBX->SetColorCoding(mitk::FiberBundleX::COLORCODING_CUSTOM);
    }


    /* MEASUREMENTS AND FANCY GUI EFFECTS CLEANUP */
    clock.Stop();
    m_itemPackage.st_FancyGUITimer1->stop();
    m_itemPackage.st_Controls->infoTimerColorCoding->setText( QString::number(clock.GetTotal()) );
    delete m_itemPackage.st_FancyGUITimer1; // fancy timer is not needed anymore
    m_hostingThread->quit();

}


QmitkFiberFeederFAWorker::QmitkFiberFeederFAWorker(QThread* hostingThread, Package4WorkingThread itemPackage)
    : m_itemPackage(itemPackage),
      m_hostingThread(hostingThread)
{

}
void QmitkFiberFeederFAWorker::run()
{
    if(m_itemPackage.st_Controls->checkBoxMonitorFiberThreads->isChecked())
        m_itemPackage.st_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_RUNNING);

    /* MEASUREMENTS AND FANCY GUI EFFECTS
     * accurate time measurement using ITK timeProbe */
    itk::TimeProbe clock;
    clock.Start();
    //set GUI representation of timer to 0, is essential for correct timer incrementation
    m_itemPackage.st_Controls->infoTimerSetFA->setText(QString::number(0));
    m_itemPackage.st_FancyGUITimer1->start();

    //do processing
    mitk::Image::Pointer FAImg = dynamic_cast<mitk::Image*>(m_itemPackage.st_PassedDataNode->GetData());
    if(FAImg.IsNotNull())
        m_itemPackage.st_FBX->SetFAMap(FAImg);

    /* MEASUREMENTS AND FANCY GUI EFFECTS CLEANUP */
    clock.Stop();
    m_itemPackage.st_FancyGUITimer1->stop();
    m_itemPackage.st_Controls->infoTimerSetFA->setText( QString::number(clock.GetTotal()) );

    disconnect(m_itemPackage.st_FancyGUITimer1);
    delete m_itemPackage.st_FancyGUITimer1; // fancy timer is not needed anymore
    m_hostingThread->quit();

}


/*===================================================================================
 * THIS METHOD IMPLEMENTS THE ACTIONS WHICH SHALL BE EXECUTED by the according THREAD
 * --generate random fibers--*/
QmitkFiberGenerateRandomWorker::QmitkFiberGenerateRandomWorker(QThread* hostingThread, Package4WorkingThread itemPackage)
    : m_itemPackage(itemPackage),
      m_hostingThread(hostingThread)
{

}
void QmitkFiberGenerateRandomWorker::run()
{
    if(m_itemPackage.st_Controls->checkBoxMonitorFiberThreads->isChecked())
        m_itemPackage.st_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_RUNNING);

    /* MEASUREMENTS AND FANCY GUI EFFECTS */
    //MAKE SURE by yourself THAT NOTHING ELSE THAN A NUMBER IS SET IN THAT LABEL
    m_itemPackage.st_Controls->infoTimerGenerateFiberBundle->setText(QString::number(0));
    m_itemPackage.st_FancyGUITimer1->start();

    //do processing, generateRandomFibers
    int numOfFibers = m_itemPackage.st_Controls->boxFiberNumbers->value();
    int distrRadius = m_itemPackage.st_Controls->boxDistributionRadius->value();
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

    // initialize accurate time measurement
    itk::TimeProbe clock;
    clock.Start();

    /* GENERATE VTK POLYLINES OUT OF FIBERSTORAGE */
    vtkSmartPointer<vtkCellArray> linesCell = vtkSmartPointer<vtkCellArray>::New(); // Host vtkPolyLines
    linesCell->Allocate(pnts->GetNumberOfPoints()*2); //allocate for each cellindex also space for the pointId, e.g. [idx | pntID]
    for (long i=0; i<fiberStorage.size(); ++i)
    {
        std::vector<int> singleFiber = fiberStorage.at(i);
        vtkSmartPointer<vtkPolyLine> fiber = vtkSmartPointer<vtkPolyLine>::New();
        fiber->GetPointIds()->SetNumberOfIds((int)singleFiber.size());

        for (long si=0; si<singleFiber.size(); ++si)
        {  //hopefully long to double works fine in VTK ;-)
            fiber->GetPointIds()->SetId( si, singleFiber.at(si) );
        }

        linesCell->InsertNextCell(fiber);
    }

    /* checkpoint for cellarray allocation */
    if ( (linesCell->GetSize()/pnts->GetNumberOfPoints()) != 2 ) //e.g. size: 12, number of points:6 .... each cell hosts point ids (6 ids) + cell index for each idPoint. 6 * 2 = 12
    {
        MITK_INFO << "RANDOM FIBER ALLOCATION CAN NOT BE TRUSTED ANYMORE! Correct leak or remove command: linesCell->Allocate(pnts->GetNumberOfPoints()*2) but be aware of possible loss in performance.";
    }

    /* HOSTING POLYDATA FOR RANDOM FIBERSTRUCTURE */
    vtkSmartPointer<vtkPolyData> PDRandom = vtkSmartPointer<vtkPolyData>::New(); //could also be a standard pointer instead of smartpointer cuz ther is no need to delete because data is managed in datastorage.
    PDRandom->SetPoints(pnts);
    PDRandom->SetLines(linesCell);

    // accurate timer measurement stop
    clock.Stop();
    //MITK_INFO << "=====Assambling random Fibers to Polydata======\nMean: " << clock.GetMean() << " Total: " << clock.GetTotal() << std::endl;

    // call function to convert fiberstructure into fiberbundleX and pass it to datastorage
    (m_itemPackage.st_host->*m_itemPackage.st_pntr_to_Method_PutFibersToDataStorage)(PDRandom);

    /* MEASUREMENTS AND FANCY GUI EFFECTS CLEANUP */
    m_itemPackage.st_FancyGUITimer1->stop();
    m_itemPackage.st_Controls->infoTimerGenerateFiberBundle->setText( QString::number(clock.GetTotal()) );
    delete m_itemPackage.st_FancyGUITimer1; // fancy timer is not needed anymore
    m_hostingThread->quit();

}


/*===================================================================================
 * THIS METHOD IMPLEMENTS THE ACTIONS WHICH SHALL BE EXECUTED by the according THREAD
 * --update GUI elements of thread monitor--
 * implementation not thread safe, not needed so far because
 * there exists only 1 thread for fiberprocessing
 * for threadsafety, you need to implement checking mechanisms in methods "::threadFor...." */
QmitkFiberThreadMonitorWorker::QmitkFiberThreadMonitorWorker( QThread* hostingThread, Package4WorkingThread itemPackage )
    : m_itemPackage(itemPackage)
    , m_hostingThread(hostingThread)
    , m_pixelstepper(10) //for next rendering call, move object 10px
    , m_steppingDistance(220) //use only a multiple value of pixelstepper, x-axis border for fancy stuff
{


    //set timers
    m_thtimer_initMonitor = new QTimer;
    m_thtimer_initMonitor->setInterval(10);

    m_thtimer_initMonitorSetFinalPosition = new QTimer;
    m_thtimer_initMonitorSetFinalPosition->setInterval(10);

    m_thtimer_initMonitorSetMasks = new QTimer;
    m_thtimer_initMonitorSetFinalPosition->setInterval(10);


    m_thtimer_threadStarted  = new QTimer;
    m_thtimer_threadStarted->setInterval(50);

    m_thtimer_threadFinished = new QTimer;
    m_thtimer_threadFinished->setInterval(50);

    m_thtimer_threadTerminated = new QTimer;
    m_thtimer_threadTerminated->setInterval(50);



    connect (m_thtimer_initMonitor, SIGNAL( timeout()), this, SLOT( fancyMonitorInitialization() ) );
    connect ( m_thtimer_initMonitorSetFinalPosition, SIGNAL( timeout() ), this, SLOT( fancyMonitorInitializationFinalPos() ) );
    connect ( m_thtimer_initMonitorSetMasks, SIGNAL( timeout() ), this, SLOT( fancyMonitorInitializationMask() ) );

    connect (m_thtimer_threadStarted, SIGNAL( timeout()), this, SLOT( fancyTextFading_threadStarted() ) );
    connect (m_thtimer_threadFinished, SIGNAL( timeout()), this, SLOT( fancyTextFading_threadFinished() ) );
    connect (m_thtimer_threadTerminated, SIGNAL( timeout()), this, SLOT( fancyTextFading_threadTerminated() ) );

    //first, the current text shall turn transparent
    m_decreaseOpacity_threadStarted = true;
    m_decreaseOpacity_threadFinished = true;
    m_decreaseOpacity_threadTerminated = true;




}
void QmitkFiberThreadMonitorWorker::run()
{

}

void QmitkFiberThreadMonitorWorker::initializeMonitor()
{
    //fancy configuration of animation start
    mitk::Point2D pntOpen;
    pntOpen[0] = 118;
    pntOpen[1] = 10;

    mitk::Point2D headPos;
    headPos[0] = 19;
    headPos[1] = 10;

    mitk::Point2D statusPos;
    statusPos[0] = 105;
    statusPos[1] = 23;

    mitk::Point2D startedPos;
    startedPos[0] = 68;
    startedPos[1] = 10;

    mitk::Point2D finishedPos;
    finishedPos[0] = 143;
    finishedPos[1] = 10;

    mitk::Point2D terminatedPos;
    terminatedPos[0] = 240;
    terminatedPos[1] = 10;

    m_itemPackage.st_FBX_Monitor->setBracketClosePosition(pntOpen);
    m_itemPackage.st_FBX_Monitor->setBracketOpenPosition(pntOpen);
    m_itemPackage.st_FBX_Monitor->setHeadingPosition(headPos);
    m_itemPackage.st_FBX_Monitor->setMaskPosition(headPos);
    m_itemPackage.st_FBX_Monitor->setStatusPosition(statusPos);
    m_itemPackage.st_FBX_Monitor->setStartedPosition(startedPos);
    m_itemPackage.st_FBX_Monitor->setFinishedPosition(finishedPos);
    m_itemPackage.st_FBX_Monitor->setTerminatedPosition(terminatedPos);


    m_thtimer_initMonitor->start();
}

void QmitkFiberThreadMonitorWorker::setThreadStatus(QString status)
{
    m_itemPackage.st_FBX_Monitor->setStatus(status);
    m_itemPackage.st_ThreadMonitorDataNode->Modified();
    m_itemPackage.st_MultiWidget->RequestUpdate();
}

/* Methods to set status of running threads
 * Following three methods are usually called
 - before a thread starts and
 - a thread is finished or terminated */
void QmitkFiberThreadMonitorWorker::threadForFiberProcessingStarted()
{
    if(!m_thtimer_threadStarted->isActive())  {
        m_thtimer_threadStarted->start();
    } else {
        //fast change without fancy stuff, needed to keep threaddebugger info up to date
        int counter = m_itemPackage.st_FBX_Monitor->getStarted();
        m_itemPackage.st_FBX_Monitor->setStarted(++counter);

    }


}

void QmitkFiberThreadMonitorWorker::threadForFiberProcessingFinished()
{
    if(!m_thtimer_threadFinished->isActive())  {
        m_thtimer_threadFinished->start();
    } else {
        //fast change without fancy stuff
        int counter = m_itemPackage.st_FBX_Monitor->getFinished();
        m_itemPackage.st_FBX_Monitor->setFinished(++counter);

    }

}

void QmitkFiberThreadMonitorWorker::threadForFiberProcessingTerminated()
{
    if(!m_thtimer_threadTerminated->isActive())  {
        m_thtimer_threadTerminated->start();
    } else {
        //fast change without fancy stuff
        int counter = m_itemPackage.st_FBX_Monitor->getTerminated();
        m_itemPackage.st_FBX_Monitor->setTerminated(++counter);

    }

}


/* Helper methods for fancy fading efx for thread monitor */
void QmitkFiberThreadMonitorWorker::fancyTextFading_threadStarted()
{

    if (m_decreaseOpacity_threadStarted) {
        int startedOpacity = m_itemPackage.st_FBX_Monitor->getStartedOpacity();
        m_itemPackage.st_FBX_Monitor->setStartedOpacity( --startedOpacity );

        if (startedOpacity == 0) {
            int counter = m_itemPackage.st_FBX_Monitor->getStarted();
            m_itemPackage.st_FBX_Monitor->setStarted(++counter);
            m_decreaseOpacity_threadStarted = false;
        }
        m_itemPackage.st_ThreadMonitorDataNode->Modified();
        m_itemPackage.st_MultiWidget->RequestUpdate();

    } else {

        int startedOpacity = m_itemPackage.st_FBX_Monitor->getStartedOpacity();
        m_itemPackage.st_FBX_Monitor->setStartedOpacity( ++startedOpacity );

        if (startedOpacity >= 10) {
            m_thtimer_threadStarted->stop();
            m_decreaseOpacity_threadStarted = true; //set back to true, cuz next iteration shall decrease opacity as well
        }

        m_itemPackage.st_ThreadMonitorDataNode->Modified();
        m_itemPackage.st_MultiWidget->RequestUpdate();

    }


}

void QmitkFiberThreadMonitorWorker::fancyTextFading_threadFinished()
{
    if (m_decreaseOpacity_threadFinished) {
        int finishedOpacity = m_itemPackage.st_FBX_Monitor->getFinishedOpacity();
        m_itemPackage.st_FBX_Monitor->setFinishedOpacity( --finishedOpacity );

        if (finishedOpacity == 0) {
            int counter = m_itemPackage.st_FBX_Monitor->getFinished();
            m_itemPackage.st_FBX_Monitor->setFinished(++counter);
            m_decreaseOpacity_threadFinished = false;
        }
        m_itemPackage.st_ThreadMonitorDataNode->Modified();
        m_itemPackage.st_MultiWidget->RequestUpdate();

    } else {

        int finishedOpacity = m_itemPackage.st_FBX_Monitor->getFinishedOpacity();
        m_itemPackage.st_FBX_Monitor->setFinishedOpacity( ++finishedOpacity );

        if (finishedOpacity >= 10) {
            m_thtimer_threadFinished->stop();
            m_decreaseOpacity_threadFinished = true; //set back to true, cuz next iteration shall decrease opacity as well
        }

        m_itemPackage.st_ThreadMonitorDataNode->Modified();
        m_itemPackage.st_MultiWidget->RequestUpdate();

    }

}

void QmitkFiberThreadMonitorWorker::fancyTextFading_threadTerminated()
{
    if (m_decreaseOpacity_threadTerminated) {
        int terminatedOpacity = m_itemPackage.st_FBX_Monitor->getTerminatedOpacity();
        m_itemPackage.st_FBX_Monitor->setTerminatedOpacity( --terminatedOpacity );

        if (terminatedOpacity == 0) {
            int counter = m_itemPackage.st_FBX_Monitor->getTerminated();
            m_itemPackage.st_FBX_Monitor->setTerminated(++counter);
            m_decreaseOpacity_threadTerminated = false;
        }
        m_itemPackage.st_ThreadMonitorDataNode->Modified();
        m_itemPackage.st_MultiWidget->RequestUpdate();

    } else {

        int terminatedOpacity = m_itemPackage.st_FBX_Monitor->getTerminatedOpacity();
        m_itemPackage.st_FBX_Monitor->setTerminatedOpacity( ++terminatedOpacity );

        if (terminatedOpacity >= 10) {
            m_thtimer_threadTerminated->stop();
            m_decreaseOpacity_threadTerminated = true; //set back to true, cuz next iteration shall decrease opacity as well
        }

        m_itemPackage.st_ThreadMonitorDataNode->Modified();
        m_itemPackage.st_MultiWidget->RequestUpdate();

    }



}

void QmitkFiberThreadMonitorWorker::fancyMonitorInitialization()
{

    mitk::Point2D pntClose = m_itemPackage.st_FBX_Monitor->getBracketClosePosition(); //possible bottleneck, set pntClose to member
    mitk::Point2D pntOpen = m_itemPackage.st_FBX_Monitor->getBracketOpenPosition(); //possible bottleneck, set pntClose to member

    pntClose[0] += m_pixelstepper;
    pntOpen[0] -= m_pixelstepper;
    //MITK_INFO << pntClose[0] << " " << pntOpen[0];

    m_itemPackage.st_FBX_Monitor->setBracketClosePosition(pntClose);
    m_itemPackage.st_FBX_Monitor->setBracketOpenPosition(pntOpen);

    int opacity = m_itemPackage.st_FBX_Monitor->getHeadingOpacity() + 1;
    if (opacity > 10)
        opacity = 10;
    m_itemPackage.st_FBX_Monitor->setHeadingOpacity(opacity);


    if (pntClose[0] >= m_steppingDistance)
    {
        if (m_itemPackage.st_FBX_Monitor->getHeadingOpacity() != 10 )
        {
            m_itemPackage.st_FBX_Monitor->setHeadingOpacity(10);
            m_itemPackage.st_ThreadMonitorDataNode->Modified();
            m_itemPackage.st_MultiWidget->RequestUpdate();
        }

        m_thtimer_initMonitor->stop();

        //position them to obt y=25
        m_thtimer_initMonitorSetFinalPosition->start();
    }

    m_itemPackage.st_ThreadMonitorDataNode->Modified();
    m_itemPackage.st_MultiWidget->RequestUpdate();


}

void QmitkFiberThreadMonitorWorker::fancyMonitorInitializationFinalPos()
{
    //get y pos of
    mitk::Point2D pntClose = m_itemPackage.st_FBX_Monitor->getBracketClosePosition();
    mitk::Point2D pntOpen = m_itemPackage.st_FBX_Monitor->getBracketOpenPosition();
    mitk::Point2D pntHead = m_itemPackage.st_FBX_Monitor->getHeadingPosition();

    pntClose[1] += 5;
    pntOpen[1] += 5;
    pntHead[1] += 5;

    m_itemPackage.st_FBX_Monitor->setBracketClosePosition(pntClose);
    m_itemPackage.st_FBX_Monitor->setBracketOpenPosition(pntOpen);
    m_itemPackage.st_FBX_Monitor->setHeadingPosition(pntHead);


    if (pntClose[1] >= 35) { //35 = y position
        m_thtimer_initMonitorSetFinalPosition->stop();
        //now init mask of labels
        m_thtimer_initMonitorSetMasks->start();
    }

    m_itemPackage.st_ThreadMonitorDataNode->Modified();
    m_itemPackage.st_MultiWidget->RequestUpdate();

}

void QmitkFiberThreadMonitorWorker::fancyMonitorInitializationMask()
{
    //increase opacity
    int opacity = m_itemPackage.st_FBX_Monitor->getMaskOpacity();
    opacity++;
    m_itemPackage.st_FBX_Monitor->setMaskOpacity(opacity);
    m_itemPackage.st_FBX_Monitor->setStartedOpacity(opacity);
    m_itemPackage.st_FBX_Monitor->setFinishedOpacity(opacity);
    m_itemPackage.st_FBX_Monitor->setTerminatedOpacity(opacity);
    m_itemPackage.st_FBX_Monitor->setStatusOpacity(opacity);

    if (opacity >=10) {
        m_thtimer_initMonitorSetMasks->stop();
    }

    m_itemPackage.st_ThreadMonitorDataNode->Modified();
    m_itemPackage.st_MultiWidget->RequestUpdate();

}

//==============================================
//======== W O R K E R S ________ E N D ========
//==============================================

//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################
//                   HERE STARTS THE ACTUAL FIBERBUNDLE DEVELOPER VIEW IMPLEMENTATION
//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################
//========#########################===============###########################=====================#########################



const std::string QmitkFiberBundleDeveloperView::VIEW_ID = "org.mitk.views.fiberbundledeveloper";
const std::string id_DataManager = "org.mitk.views.datamanager";
using namespace berry;


QmitkFiberBundleDeveloperView::QmitkFiberBundleDeveloperView()
    : QmitkFunctionality()
    , m_Controls( 0 )
    , m_MultiWidget( NULL )
    , m_FiberIDGenerator( NULL)
    , m_GeneratorFibersRandom( NULL )
    , m_FiberFeederFASlave( NULL )
    , m_FiberColoringSlave(NULL)
    , m_FiberExtractor(NULL)
    , m_fiberMonitorIsOn( false )
    , m_CircleCounter( 0 )
    , m_suppressSignal(false)
{
    m_hostThread = new QThread;
    m_threadInProgress = false;

}

// Destructor
QmitkFiberBundleDeveloperView::~QmitkFiberBundleDeveloperView()
{
    //m_FiberBundleX->Delete(); using weakPointer, therefore no delete necessary
    delete m_hostThread;
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
        m_Controls->buttonColorFibers->setEnabled(false);
        m_Controls->ddAvailableColorcodings->setEnabled(false);
        m_Controls->buttonExtractFibers->setEnabled(false);
        m_Controls->button_FAMap->setEnabled(true);

        m_Controls->buttonSMFibers->setEnabled(false);//not yet implemented
        m_Controls->buttonVtkDecimatePro->setEnabled(false);//not yet implemented
        m_Controls->buttonVtkSmoothPD->setEnabled(false);//not yet implemented
        m_Controls->buttonGenerateTubes->setEnabled(false);//not yet implemented


        connect( m_Controls->buttonGenerateFibers, SIGNAL(clicked()), this, SLOT(DoGenerateFibers()) );
        connect( m_Controls->buttonGenerateFiberIds, SIGNAL(clicked()), this, SLOT(DoGenerateFiberIDs()) );
        connect( m_Controls->button_FAMapExecute, SIGNAL(clicked()), this, SLOT(DoSetFAValues()) );
        connect( m_Controls->button_FAMap, SIGNAL(clicked()), this, SLOT(DoSetFAMap()) );
        connect( m_Controls->buttonExtractFibers, SIGNAL(clicked()), this, SLOT(DoExtractFibers()) );
        connect( m_Controls->radioButton_directionRandom, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
        connect( m_Controls->radioButton_directionX, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
        connect( m_Controls->radioButton_directionY, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
        connect( m_Controls->radioButton_directionZ, SIGNAL(clicked()), this, SLOT(DoUpdateGenerateFibersWidget()) );
        connect( m_Controls->toolBox, SIGNAL(currentChanged ( int ) ), this, SLOT(SelectionChangedToolBox(int)) );


        connect( m_Controls->m_CircleButton, SIGNAL( clicked() ), this, SLOT( ActionDrawEllipseTriggered() ) );
        connect( m_Controls->buttonColorFibers, SIGNAL(clicked()), this, SLOT(DoColorFibers()) );
        //        connect( m_Controls->ddAvailableColorcodings, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCurrentColorCoding(int) ));
        connect( m_Controls->ddAvailableColorcodings, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCurrentColorCoding(int) ));

        connect( m_Controls->checkBoxMonitorFiberThreads, SIGNAL(stateChanged(int)), this, SLOT(DoMonitorFiberThreads(int)) );



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

    //  vtkPolyData* output; // FiberPD stores the generated PolyData... going to be generated in thread
    if ( fibDirection == FIB_RADIOBUTTON_DIRECTION_RANDOM ) {
        //    build polydata with random lines and fibers
        //    output =
        GenerateVtkFibersRandom();

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


}

void QmitkFiberBundleDeveloperView::DoExtractFibers()
{

    /* ===== TIMER CONFIGURATIONS for visual effect ======
   * start and stop is called in Thread */
    QTimer *localTimer = new QTimer; // timer must be initialized here, otherwise timer is not fancy enough
    localTimer->setInterval( 10 );
    connect( localTimer, SIGNAL(timeout()), this, SLOT( UpdateExtractFibersTimer()) );

    struct Package4WorkingThread ItemPackageForExtractor;
    ItemPackageForExtractor.st_FBX = m_FiberBundleX;
    ItemPackageForExtractor.st_Controls = m_Controls;
    ItemPackageForExtractor.st_FancyGUITimer1 = localTimer;
    ItemPackageForExtractor.st_host = this; //needed to access method "PutFibersToDataStorage()"
    ItemPackageForExtractor.st_pntr_to_Method_PutFibersToDataStorage = &QmitkFiberBundleDeveloperView::PutFibersToDataStorage; //actual functor calling method putFibersToDataStorage
    ItemPackageForExtractor.st_PlanarFigure = m_PlanarFigure;

    //set element for thread monitoring
    if (m_fiberMonitorIsOn)
        ItemPackageForExtractor.st_fiberThreadMonitorWorker = m_fiberThreadMonitorWorker;


    if (m_threadInProgress)
        return; //maybe popup window saying, working thread still in progress...pls wait

    m_FiberExtractor = new QmitkFiberExtractorWorker(m_hostThread, ItemPackageForExtractor);
    m_FiberExtractor->moveToThread(m_hostThread);

    //connections
    connect(m_hostThread, SIGNAL(started()), this, SLOT( BeforeThread_FiberExtraction() ));
    connect(m_hostThread, SIGNAL(started()), m_FiberExtractor, SLOT( run() ));
    connect(m_hostThread, SIGNAL(finished()), this, SLOT( AfterThread_FiberExtraction() ));
    connect(m_hostThread, SIGNAL(terminated()), this, SLOT( AfterThread_FiberExtraction() ));

    m_hostThread->start(QThread::HighestPriority) ;
}

void QmitkFiberBundleDeveloperView::UpdateExtractFibersTimer()
{
    // Make sure that thread has set according info-label to number! here we do not check if value is numeric! shall be done in beforeThreadstarted()
    QString crntValue = m_Controls->infoTimerExtractFibers->text();
    int tmpVal = crntValue.toInt();
    m_Controls->infoTimerExtractFibers->setText(QString::number(++tmpVal));
    m_Controls->infoTimerExtractFibers->update();
}

void QmitkFiberBundleDeveloperView::BeforeThread_FiberExtraction()
{
    m_threadInProgress = true;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingStarted();
        //m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_STARTED);
    }
}

void QmitkFiberBundleDeveloperView::AfterThread_FiberExtraction()
{
    m_threadInProgress = false;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingFinished();
        m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_IDLE);
    }
    //    disconnect(m_hostThread, 0, 0, 0);
    m_hostThread->disconnect();

    //    m_FiberExtractor->disconnect();
    delete m_FiberExtractor;

    m_FiberBundleNode->Modified();
    m_MultiWidget->RequestUpdate();

}

void QmitkFiberBundleDeveloperView::PutFibersToDataStorage( vtkSmartPointer<vtkPolyData> threadOutput)
{

    MITK_INFO << "lines: " << threadOutput->GetNumberOfLines() << "pnts: " << threadOutput->GetNumberOfPoints();
    //qthread mutex lock
    mitk::FiberBundleX::Pointer FB = mitk::FiberBundleX::New(threadOutput);
    mitk::DataNode::Pointer FBNode;
    FBNode = mitk::DataNode::New();
    FBNode->SetName("FiberBundleX");
    FBNode->SetData(FB);
    FBNode->SetVisibility(true);
    FBNode->SetOpacity(1.0);

    GetDataStorage()->Add(FBNode);
    FBNode->Modified();

    const mitk::PlaneGeometry * tsgeo = m_MultiWidget->GetTimeNavigationController()->GetCurrentPlaneGeometry();
    if (tsgeo == NULL) {
        /* GetDataStorage()->Modified etc. have no effect, therefore proceed as followed below */
        // get all nodes that have not set "includeInBoundingBox" to false
        mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
                                                                                                            , mitk::BoolProperty::New(false)));
        mitk::DataStorage::SetOfObjects::ConstPointer rs = GetDataStorage()->GetSubset(pred);
        // calculate bounding geometry of these nodes
        mitk::TimeGeometry::Pointer bounds = GetDataStorage()->ComputeBoundingGeometry3D(rs);
        // initialize the views to the bounding geometry
        mitk::RenderingManager::GetInstance()->InitializeViews(bounds);

    } else {

        GetDataStorage()->Modified();
        m_MultiWidget->RequestUpdate(); //necessary??
    }

    //qthread mutex unlock
}

void QmitkFiberBundleDeveloperView::PutFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name)
{
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetName(name.toStdString());
    newNode->SetData(figure);



    std::vector<mitk::DataNode*> selectedNodes = GetDataManagerSelection();
    for(unsigned int i = 0; i < selectedNodes.size(); i++)
    {
        selectedNodes[i]->SetSelected(false);
    }

    newNode->SetSelected(true);

    newNode->AddProperty( "planarfigure.default.line.color", mitk::ColorProperty::New(1.0,0.0,0.0));
    newNode->AddProperty( "planarfigure.line.width", mitk::FloatProperty::New(2.0));
    newNode->AddProperty( "planarfigure.drawshadow", mitk::BoolProperty::New(true));

    newNode->AddProperty( "selected", mitk::BoolProperty::New(true) );
    newNode->AddProperty( "planarfigure.ishovering", mitk::BoolProperty::New(true) );
    newNode->AddProperty( "planarfigure.drawoutline", mitk::BoolProperty::New(true) );
    newNode->AddProperty( "planarfigure.drawquantities", mitk::BoolProperty::New(false) );
    newNode->AddProperty( "planarfigure.drawshadow", mitk::BoolProperty::New(true) );

    newNode->AddProperty( "planarfigure.line.width", mitk::FloatProperty::New(3.0) );
    newNode->AddProperty( "planarfigure.shadow.widthmodifier", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.outline.width", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.helperline.width", mitk::FloatProperty::New(2.0) );

    newNode->AddProperty( "planarfigure.default.line.color", mitk::ColorProperty::New(1.0,1.0,1.0) );
    newNode->AddProperty( "planarfigure.default.line.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.default.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.default.outline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.default.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.default.helperline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.default.markerline.color", mitk::ColorProperty::New(0.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.default.markerline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.default.marker.color", mitk::ColorProperty::New(1.0,1.0,1.0)  );
    newNode->AddProperty( "planarfigure.default.marker.opacity",mitk::FloatProperty::New(2.0) );

    newNode->AddProperty( "planarfigure.hover.line.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.hover.line.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.hover.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.hover.outline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.hover.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.hover.helperline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.hover.markerline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.hover.markerline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.hover.marker.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.hover.marker.opacity", mitk::FloatProperty::New(2.0) );

    newNode->AddProperty( "planarfigure.selected.line.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.selected.line.opacity",mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.selected.outline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.selected.outline.opacity", mitk::FloatProperty::New(2.0));
    newNode->AddProperty( "planarfigure.selected.helperline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.selected.helperline.opacity",mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.selected.markerline.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.selected.markerline.opacity", mitk::FloatProperty::New(2.0) );
    newNode->AddProperty( "planarfigure.selected.marker.color", mitk::ColorProperty::New(1.0,0.0,0.0)  );
    newNode->AddProperty( "planarfigure.selected.marker.opacity",mitk::FloatProperty::New(2.0));

    // figure drawn on the topmost layer / image
    this->GetDataStorage()->Add(newNode);
}

/*
 * Generate polydata of random fibers
 */
void QmitkFiberBundleDeveloperView::GenerateVtkFibersRandom()
{

    /* ===== TIMER CONFIGURATIONS for visual effect ======
   * start and stop is called in Thread */
    QTimer *localTimer = new QTimer; // timer must be initialized here, otherwise timer is not fancy enough
    localTimer->setInterval( 10 );
    connect( localTimer, SIGNAL(timeout()), this, SLOT(UpdateGenerateRandomFibersTimer()) );

    struct Package4WorkingThread ItemPackageForRandomGenerator;
    ItemPackageForRandomGenerator.st_FBX = m_FiberBundleX;
    ItemPackageForRandomGenerator.st_Controls = m_Controls;
    ItemPackageForRandomGenerator.st_FancyGUITimer1 = localTimer;
    ItemPackageForRandomGenerator.st_host = this; //needed to access method "PutFibersToDataStorage()"
    ItemPackageForRandomGenerator.st_pntr_to_Method_PutFibersToDataStorage = &QmitkFiberBundleDeveloperView::PutFibersToDataStorage; //actual functor calling method putFibersToDataStorage

    //set element for thread monitoring
    if (m_fiberMonitorIsOn)
        ItemPackageForRandomGenerator.st_fiberThreadMonitorWorker = m_fiberThreadMonitorWorker;


    if (m_threadInProgress)
        return; //maybe popup window saying, working thread still in progress...pls wait

    m_GeneratorFibersRandom = new QmitkFiberGenerateRandomWorker(m_hostThread, ItemPackageForRandomGenerator);
    m_GeneratorFibersRandom->moveToThread(m_hostThread);

    connect(m_hostThread, SIGNAL(started()), this, SLOT( BeforeThread_GenerateFibersRandom()) );
    connect(m_hostThread, SIGNAL(started()), m_GeneratorFibersRandom, SLOT(run()) );
    connect(m_hostThread, SIGNAL(finished()), this, SLOT(AfterThread_GenerateFibersRandom()) );
    connect(m_hostThread, SIGNAL(terminated()), this, SLOT(AfterThread_GenerateFibersRandom()) );

    m_hostThread->start(QThread::LowestPriority);

}

void QmitkFiberBundleDeveloperView::UpdateColorFibersTimer()
{
    // Make sure that thread has set according info-label to number! here we do not check if value is numeric!
    QString crntValue = m_Controls->infoTimerColorCoding->text();
    int tmpVal = crntValue.toInt();
    m_Controls->infoTimerColorCoding->setText(QString::number(++tmpVal));
    m_Controls->infoTimerColorCoding->update();
}

void QmitkFiberBundleDeveloperView::UpdateGenerateRandomFibersTimer()
{
    // Make sure that thread has set according info-label to number! here we do not check if value is numeric!
    QString crntValue = m_Controls->infoTimerGenerateFiberBundle->text();
    int tmpVal = crntValue.toInt();
    m_Controls->infoTimerGenerateFiberBundle->setText(QString::number(++tmpVal));
    m_Controls->infoTimerGenerateFiberBundle->update();

}

void QmitkFiberBundleDeveloperView::UpdateSetFAValuesTimer()
{
    // Make sure that thread has set according info-label to number! here we do not check if value is numeric!
    QString crntValue = m_Controls->infoTimerSetFA->text();
    int tmpVal = crntValue.toInt();
    m_Controls->infoTimerSetFA->setText(QString::number(++tmpVal));
    m_Controls->infoTimerSetFA->update();

}

void QmitkFiberBundleDeveloperView::BeforeThread_GenerateFibersRandom()
{
    m_threadInProgress = true;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingStarted();
        //m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_STARTED);
    }
}

void QmitkFiberBundleDeveloperView::AfterThread_GenerateFibersRandom()
{
    m_threadInProgress = false;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingFinished();
        m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_IDLE);
    }
    //    disconnect(m_hostThread, 0, 0, 0);
    m_hostThread->disconnect();
    delete m_GeneratorFibersRandom;


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

void QmitkFiberBundleDeveloperView::DoSetFAValues()
{

    QTimer *localTimer = new QTimer; // timer must be initialized here, otherwise timer is not fancy enough
    localTimer->setInterval( 10 );
    connect( localTimer, SIGNAL(timeout()), this, SLOT( UpdateSetFAValuesTimer() ) );

    // pack items which are needed by thread processing
    struct Package4WorkingThread ItemPackageToSetFAMap;
    ItemPackageToSetFAMap.st_FBX = m_FiberBundleX;
    ItemPackageToSetFAMap.st_FancyGUITimer1 = localTimer;
    ItemPackageToSetFAMap.st_PassedDataNode = m_FANode;
    ItemPackageToSetFAMap.st_Controls = m_Controls;

    if (m_fiberMonitorIsOn)
        ItemPackageToSetFAMap.st_fiberThreadMonitorWorker = m_fiberThreadMonitorWorker;

    if (m_threadInProgress)
        return; //maybe popup window saying, working thread still in progress...pls wait

    m_FiberFeederFASlave = new QmitkFiberFeederFAWorker(m_hostThread, ItemPackageToSetFAMap);
    m_FiberFeederFASlave->moveToThread(m_hostThread);

    connect(m_hostThread, SIGNAL(started()), this, SLOT( BeforeThread_FiberSetFA()) );
    connect(m_hostThread, SIGNAL(started()), m_FiberFeederFASlave, SLOT(run()) );
    connect(m_hostThread, SIGNAL(finished()), this, SLOT(AfterThread_FiberSetFA()));
    connect(m_hostThread, SIGNAL(terminated()), this, SLOT(AfterThread_FiberSetFA()));
    m_hostThread->start(QThread::LowestPriority);
}

void QmitkFiberBundleDeveloperView::DoSetFAMap()
{
    std::vector<mitk::DataNode*> nodes = GetDataManagerSelection();
    if (nodes.empty())
    {
        m_Controls->lineEdit_FAMap->setText("N/A");
        return;
    }

    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
         it != nodes.end();
         ++it )
    {
        mitk::DataNode::Pointer node = *it;

        if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()))
        {
            // this node is what we want
            m_FANode = node;
            m_Controls->lineEdit_FAMap->setText(node->GetName().c_str());
            return;
        }
    }
}

void QmitkFiberBundleDeveloperView::BeforeThread_FiberSetFA()
{
    m_threadInProgress = true;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingStarted();
    }
}

void QmitkFiberBundleDeveloperView::AfterThread_FiberSetFA()
{
    m_threadInProgress = false;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingFinished();
        m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_IDLE);
    }
    disconnect(m_hostThread, 0, 0, 0);
    m_hostThread->disconnect();
    //update renderer
    m_FiberBundleNode->Modified();
    m_MultiWidget->ForceImmediateUpdate();

    //update QComboBox(dropDown menu) in view of available ColorCodings
    this->DoGatherColorCodings();

    delete m_FiberFeederFASlave;
}

void QmitkFiberBundleDeveloperView::DoColorFibers()
{
    //
    MITK_INFO << "call fibercoloring in fiberBundleX";
    QTimer *localTimer = new QTimer; // timer must be initialized here, otherwise timer is not fancy enough
    localTimer->setInterval( 10 );
    connect( localTimer, SIGNAL(timeout()), this, SLOT( UpdateColorFibersTimer() ) );

    // pack items which are needed by thread processing
    struct Package4WorkingThread ItemPackageForFiberColoring;
    ItemPackageForFiberColoring.st_FBX = m_FiberBundleX;
    ItemPackageForFiberColoring.st_PassedDataNode = m_FiberBundleNode;
    ItemPackageForFiberColoring.st_FancyGUITimer1 = localTimer;
    ItemPackageForFiberColoring.st_Controls = m_Controls; //needed to catch up some selections and set options in GUI

    if (m_fiberMonitorIsOn)
        ItemPackageForFiberColoring.st_fiberThreadMonitorWorker = m_fiberThreadMonitorWorker;

    if (m_threadInProgress)
        return; //maybe popup window saying, working thread still in progress...pls wait

    m_FiberColoringSlave = new QmitkFiberColoringWorker(m_hostThread, ItemPackageForFiberColoring);
    m_FiberColoringSlave->moveToThread(m_hostThread);
    connect(m_hostThread, SIGNAL(started()), this, SLOT( BeforeThread_FiberColorCoding()) );
    connect(m_hostThread, SIGNAL(started()), m_FiberColoringSlave, SLOT(run()) );
    connect(m_hostThread, SIGNAL(finished()), this, SLOT(AfterThread_FiberColorCoding()));
    connect(m_hostThread, SIGNAL(terminated()), this, SLOT(AfterThread_FiberColorCoding()));
    m_hostThread->start(QThread::LowestPriority);

}

void QmitkFiberBundleDeveloperView::BeforeThread_FiberColorCoding()
{
    m_threadInProgress = true;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingStarted();
    }
}


void QmitkFiberBundleDeveloperView::AfterThread_FiberColorCoding()
{
    m_threadInProgress = false;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingFinished();
        m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_IDLE);
    }
    disconnect(m_hostThread, 0, 0, 0);
    m_hostThread->disconnect();
    //update renderer
    m_FiberBundleNode->Modified();
    m_MultiWidget->ForceImmediateUpdate();


    //update QComboBox(dropDown menu) in view of available ColorCodings
    this->DoGatherColorCodings();

    delete m_FiberColoringSlave;
}


void QmitkFiberBundleDeveloperView::DoGatherColorCodings()
{
    QStringList fbxColorCodings = m_FiberBundleX->GetAvailableColorCodings();

    //update dropDown Menu
    //remove all items from menu
    m_suppressSignal = true;
    int ddItems = m_Controls->ddAvailableColorcodings->count();
    for(int i=ddItems-1; i>=0; i--)
    {   //note, after each item remove, index in QComboBox is updated. sending signal: index changed
        m_Controls->ddAvailableColorcodings->removeItem(i);
    }
    //fill new data into menu
    m_Controls->ddAvailableColorcodings->addItems(fbxColorCodings);
    m_Controls->ddAvailableColorcodings->addItem(m_FiberBundleX->COLORCODING_CUSTOM);

    //highlight current colorcoding
    QString cc = m_FiberBundleX->GetCurrentColorCoding();
    MITK_INFO << cc.toStdString().c_str() << " is at idx: " << m_Controls->ddAvailableColorcodings->findText(cc);
    m_Controls->ddAvailableColorcodings->setCurrentIndex( m_Controls->ddAvailableColorcodings->findText(cc) );
    m_Controls->ddAvailableColorcodings->update();
    m_suppressSignal = false;
}


void QmitkFiberBundleDeveloperView::SetCurrentColorCoding(int idx)
{
    if(!m_suppressSignal){
        QString selectedColorCoding = m_Controls->ddAvailableColorcodings->itemText(idx);
        m_FiberBundleX->SetColorCoding(selectedColorCoding.toStdString().c_str() ); //QString to char
        // update rendering
        m_FiberBundleNode->Modified();
        m_MultiWidget->ForceImmediateUpdate();
    }
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
    vtkSmartPointer<vtkMatrix4x4> m = vtkSmartPointer<vtkMatrix4x4>::New();
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

    /* ===== TIMER CONFIGURATIONS for visual effect ======
   * start and stop is called in Thread */
    QTimer *localTimer = new QTimer; // timer must be initialized here, otherwise timer is not fancy enough
    localTimer->setInterval( 10 );
    connect( localTimer, SIGNAL(timeout()), this, SLOT(UpdateFiberIDTimer()) );


    // pack items which are needed by thread processing
    struct Package4WorkingThread FiberIdPackage;
    FiberIdPackage.st_FBX = m_FiberBundleX;
    FiberIdPackage.st_FancyGUITimer1 = localTimer;
    FiberIdPackage.st_Controls = m_Controls;

    //set element for thread monitoring
    if (m_fiberMonitorIsOn)
        FiberIdPackage.st_fiberThreadMonitorWorker = m_fiberThreadMonitorWorker;


    if (m_threadInProgress)
        return; //maybe popup window saying, working thread still in progress...pls wait

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
    m_threadInProgress = true;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingStarted();
        m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_STARTED);
    }
}

void QmitkFiberBundleDeveloperView::AfterThread_IdGenerate()
{
    m_threadInProgress = false;
    if (m_fiberMonitorIsOn){
        m_fiberThreadMonitorWorker->threadForFiberProcessingFinished();
        m_fiberThreadMonitorWorker->setThreadStatus(FBX_STATUS_IDLE);
    }
    disconnect(m_hostThread, 0, 0, 0);
    m_hostThread->disconnect();
    delete m_FiberIDGenerator;

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
    numOfFibers.setNum( m_FiberBundleX->GetFiberPolyData()->GetNumberOfLines() );
    QString numOfPoints;
    numOfPoints.setNum( m_FiberBundleX->GetFiberPolyData()->GetNumberOfPoints() );

    m_Controls->infoAnalyseNumOfFibers->setText( numOfFibers );
    m_Controls->infoAnalyseNumOfPoints->setText( numOfPoints );
}

void QmitkFiberBundleDeveloperView::SelectionChangedToolBox(int idx)
{
    // show/reset items of selected toolbox page FiberInfo
    if (m_Controls->page_FiberInfo->isVisible())
    {
        if (m_FiberBundleX != NULL) {
            FeedFiberInfoWidget();

        } else {
            //if infolables are disabled: return
            //else set info back to - and set label and info to disabled
            ResetFiberInfoWidget();
        }
    }

    // show/reset items of selected toolbox page FiberProcessing
    if (m_Controls->page_FiberProcessing->isVisible())
    {
        if (m_FiberBundleX.IsNotNull() && m_PlanarFigure.IsNotNull() )
        {
            //show fiber extraction button
            m_Controls->buttonExtractFibers->setEnabled(true);

        } else {
            m_Controls->buttonExtractFibers->setEnabled(false);
        }

        if (m_FiberBundleX.IsNotNull())
        {
            //show button colorCoding
            m_Controls->buttonColorFibers->setEnabled(true);
            m_Controls->ddAvailableColorcodings->setEnabled(true);
            m_Controls->buttonGenerateFiberIds->setEnabled(true);

            //       m_Controls->buttonSMFibers->setEnabled(true);
            //       m_Controls->buttonVtkDecimatePro->setEnabled(true);
            //       m_Controls->buttonVtkSmoothPD->setEnabled(true);
            //       m_Controls->buttonGenerateTubes->setEnabled(true);

        } else {
            m_Controls->buttonColorFibers->setEnabled(false);
            m_Controls->ddAvailableColorcodings->setEnabled(false);
            m_Controls->buttonGenerateFiberIds->setEnabled(false);
            m_Controls->buttonSMFibers->setEnabled(false);
            m_Controls->buttonVtkDecimatePro->setEnabled(false);
            m_Controls->buttonVtkSmoothPD->setEnabled(false);
            m_Controls->buttonGenerateTubes->setEnabled(true);

        }


    }

}

void QmitkFiberBundleDeveloperView::FBXDependendGUIElementsConfigurator()
{
    // ==== FIBER PROCESSING ELEMENTS and ALL ELEMENTS WHICH NEED A FBX DATANODE======
    //  m_Controls->buttonGenerateFiberIds->setEnabled(isVisible); moved to selectionChangedToolBox

    SelectionChangedToolBox(-1); //set gui elements with respect to active tab, widget, etc. -1 has no effect

}


void QmitkFiberBundleDeveloperView::DoMonitorFiberThreads(int checkStatus)
{
    //check if in datanode exists already a node of type mitkFiberBundleXThreadMonitor
    //if not then put node to datastorage

    //if checkStatus is 1 then start qtimer using fading in starting text in datanode
    //if checkStatus is 0 then fade out dataNode using qtimer

    if (checkStatus)
    {
        m_fiberMonitorIsOn = true;
        // Generate Node hosting thread information
        mitk::FiberBundleXThreadMonitor::Pointer FBXThreadMonitor = mitk::FiberBundleXThreadMonitor::New();
        FBXThreadMonitor->SetGeometry(this->GenerateStandardGeometryForMITK());

        m_MonitorNode = mitk::DataNode::New();
        m_MonitorNode->SetName("FBX_threadMonitor");
        m_MonitorNode->SetData(FBXThreadMonitor);
        m_MonitorNode->SetVisibility(true);
        m_MonitorNode->SetOpacity(1.0);

        GetDataStorage()->Add(m_MonitorNode);

        //following code is needed for rendering text in mitk! without geometry nothing is rendered
        const mitk::PlaneGeometry * tsgeo = m_MultiWidget->GetTimeNavigationController()->GetCurrentPlaneGeometry();
        if (tsgeo == NULL) {
            /* GetDataStorage()->Modified etc. have no effect, therefore proceed as followed below */
            // get all nodes that have not set "includeInBoundingBox" to false
            mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New( "includeInBoundingBox"
                                                                                                                 , mitk::BoolProperty::New(false)));
            mitk::DataStorage::SetOfObjects::ConstPointer rs = GetDataStorage()->GetSubset(pred);
            // calculate bounding geometry of these nodes
            mitk::TimeGeometry::Pointer bounds = GetDataStorage()->ComputeBoundingGeometry3D(rs);
            // initialize the views to the bounding geometry
            mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
        } else {

            GetDataStorage()->Modified();
            m_MultiWidget->RequestUpdate(); //necessary??
        }
        //__GEOMETRY FOR THREADMONITOR GENERATED

        /* ====== initialize thread for managing fiberThread information ========= */
        m_monitorThread = new QThread;
        // the package needs datastorage, MonitorDatanode, standardmultiwidget,
        struct Package4WorkingThread ItemPackageForThreadMonitor;
        ItemPackageForThreadMonitor.st_DataStorage = GetDataStorage();
        ItemPackageForThreadMonitor.st_ThreadMonitorDataNode = m_MonitorNode;
        ItemPackageForThreadMonitor.st_MultiWidget = m_MultiWidget;
        ItemPackageForThreadMonitor.st_FBX_Monitor = FBXThreadMonitor;

        m_fiberThreadMonitorWorker = new QmitkFiberThreadMonitorWorker(m_monitorThread, ItemPackageForThreadMonitor);

        m_fiberThreadMonitorWorker->moveToThread(m_monitorThread);
        connect ( m_monitorThread, SIGNAL( started() ), m_fiberThreadMonitorWorker, SLOT( run() ) );
        m_monitorThread->start(QThread::LowestPriority);
        m_fiberThreadMonitorWorker->initializeMonitor();//do some init animation ;-)


    } else {
        m_fiberMonitorIsOn = false;

        m_monitorThread->quit();
        //think about outsourcing following lines to quit / terminate slot of thread
        GetDataStorage()->Remove(m_MonitorNode);
        GetDataStorage()->Modified();
        m_MultiWidget->RequestUpdate(); //necessary??
    }



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

    if (nodes.empty())
        return;
    /* ==== reset everyhing related to FiberBundleX ======
   * - variable m_FiberBundleX
   * - visualization of analysed fiberbundle
   */
    m_FiberBundleNode = NULL;
    m_FiberBundleX = NULL; //reset pointer, so that member does not point to depricated locations
    m_PlanarFigure = NULL;
    ResetFiberInfoWidget();


    //timer reset only when no thread is in progress
    if (!m_threadInProgress) {
        m_Controls->infoTimerGenerateFiberIds->setText("-"); //set GUI representation of timer to -
        m_Controls->infoTimerGenerateFiberBundle->setText( "-" );
        m_Controls->infoTimerColorCoding->setText( "-" );
    }
    //====================================================




    for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
    {
        mitk::DataNode::Pointer node = *it;

        /* CHECKPOINT: FIBERBUNDLE*/
        if( node.IsNotNull() && dynamic_cast<mitk::FiberBundleX*>(node->GetData()) )
        {
            m_FiberBundleNode = node;
            m_FiberBundleX = dynamic_cast<mitk::FiberBundleX*>(node->GetData());
            if (m_FiberBundleX.IsNull()){
                MITK_INFO << "========ATTENTION=========\n unable to load selected FiberBundleX to FiberBundleDeveloper-plugin \n";
                m_FiberBundleNode = NULL;
            }
            // ==== FIBERBUNDLE_INFO ELEMENTS ====
            if ( m_Controls->page_FiberInfo->isVisible() )
                FeedFiberInfoWidget();

            // enable FiberBundleX related Gui Elements, such as buttons etc.
            this->FBXDependendGUIElementsConfigurator();
            this->DoGatherColorCodings();

        }
        /* CHECKPOINT: PLANARFIGURE */
        else if ( node.IsNotNull() && dynamic_cast<mitk::PlanarFigure*>(node->GetData()) )
        {
            m_PlanarFigure = dynamic_cast<mitk::PlanarFigure*>(node->GetData());
            MITK_INFO << "PF selected";

            if (m_PlanarFigure.IsNull())
                MITK_INFO << "========ATTENTION=========\n unable to load selected Planarfigure to FiberBundleDeveloper-plugin \n";

        }

    }
    //update gui elements depending on given nodes

    FBXDependendGUIElementsConfigurator(); //every gui element which needs a FBX for processing is disabled
}


void QmitkFiberBundleDeveloperView::ActionDrawEllipseTriggered()
{
    //    bool checked = m_Controls->m_CircleButton->isChecked();
    mitk::PlanarCircle::Pointer figure = mitk::PlanarCircle::New();
    this->PutFigureToDataStorage(figure, QString("Circle%1").arg(++m_CircleCounter));

    MITK_INFO << "PlanarCircle created ...";

    mitk::DataStorage::SetOfObjects::ConstPointer _NodeSet = this->GetDefaultDataStorage()->GetAll();
    mitk::DataNode* node = 0;
    mitk::PlanarFigureInteractor::Pointer figureInteractor = 0;
    mitk::PlanarFigure* figureP = 0;

    for(mitk::DataStorage::SetOfObjects::ConstIterator it=_NodeSet->Begin(); it!=_NodeSet->End()
        ; it++)
    {
        node = const_cast<mitk::DataNode*>(it->Value().GetPointer());
        figureP = dynamic_cast<mitk::PlanarFigure*>(node->GetData());

        if(figureP)
        {
          figureInteractor = dynamic_cast<mitk::PlanarFigureInteractor*>(node->GetDataInteractor().GetPointer());

          if(figureInteractor.IsNull())
          {
            figureInteractor = mitk::PlanarFigureInteractor::New();
            mitk::Module* planarFigureModule = mitk::ModuleRegistry::GetModule( "PlanarFigure" );
            figureInteractor->LoadStateMachine("PlanarFigureInteraction.xml", planarFigureModule );
            figureInteractor->SetEventConfig( "PlanarFigureConfig.xml", planarFigureModule );
            figureInteractor->SetDataNode( node );
          }
        }
    }

}

void QmitkFiberBundleDeveloperView::Activated()
{

    MITK_INFO << "FB DevelopersV ACTIVATED()";


}



