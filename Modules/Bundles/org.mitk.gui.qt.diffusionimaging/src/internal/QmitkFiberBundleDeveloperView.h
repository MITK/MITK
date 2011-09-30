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

#ifndef QmitkFiberBundleDeveloperView_h
#define QmitkFiberBundleDeveloperView_h

#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkFunctionality.h>
#include "ui_QmitkFiberBundleDeveloperViewControls.h"

#include <mitkDataStorage.h>
#include <mitkDataStorageSelection.h>
#include <mitkWeakPointer.h>
#include <mitkFiberBundleXThreadMonitor.h>

// Qt
#include <QVector>
#include <QRadioButton>
#include <QString>

// VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <mitkFiberBundleX.h>

#include <mitkFiberBundleX.h>

#include <QTimer>
#include <QThread>

class QmitkFiberThreadMonitorWorker; //include needed for struct element
class QmitkFiberBundleDeveloperView; //this include is needed for the struct element, especially for functors to QmitkFiberBundleDeveloperView

/* ==== THIS STRUCT CONTAINS ALL NECESSARY VARIABLES 
 * TO EXECUTE AND UPDATE GUI ELEMENTS DURING PROCESSING OF A THREAD 
 * why? either you add tons of friendclasses (e.g. FiberWorker objects), or you create a package containing all items needed. Otherwise you have to set all members etc. to public!
 */
struct Package4WorkingThread
{
  mitk::FiberBundleX* st_FBX;
  QTimer* st_FancyGUITimer1;
  Ui::QmitkFiberBundleDeveloperViewControls* st_Controls;
  
  
  
  //functors to outdoor methods
  QmitkFiberBundleDeveloperView* st_host;
  void (QmitkFiberBundleDeveloperView::*st_pntr_to_Method_PutFibersToDataStorage) (vtkPolyData*);
  
  
  //==DO NOT TOUCH THIS SECTION===
  //host MITK I/O elements, especially needed for thread monitoring
  QmitkFiberThreadMonitorWorker *st_fiberThreadMonitorWorker;
  mitk::FiberBundleXThreadMonitor::Pointer st_FBX_Monitor; //needed for direct access do animation/fancy methods
  mitk::DataNode::Pointer st_ThreadMonitorDataNode; //needed for renderer to recognize node modifications
  mitk::DataStorage::Pointer st_DataStorage; //well that is discussable if needed ;-) probably not
  QmitkStdMultiWidget* st_MultiWidget; //needed for rendering update
  
};


// ====================================================================
// ============= WORKER WHICH IS PASSED TO THREAD =====================
// ====================================================================
//## Documentation
//## This class does the actual work for generating fiber ids.
class QmitkFiberIDWorker : public QObject
{
  Q_OBJECT
  
public:
  QmitkFiberIDWorker( QThread*, Package4WorkingThread );
  
  public slots:
  void run();
  
private:
  Package4WorkingThread m_itemPackage;
  QThread* m_hostingThread;
  
  
};

// ====================================================================
// ============= WORKER WHICH IS PASSED TO THREAD =====================
// ====================================================================
//## Documentation
//## This class does the actual work for colorcoding fibers.
class QmitkFiberColoringWorker : public QObject
{
  Q_OBJECT
  
public:
  QmitkFiberColoringWorker( QThread*, Package4WorkingThread );

  public slots:
  void run();
  
private:
  Package4WorkingThread m_itemPackage;
  QThread* m_hostingThread;
  
};


// ====================================================================
// ============= WORKER WHICH IS PASSED TO THREAD =====================
// ====================================================================
class QmitkFiberGenerateRandomWorker : public QObject
{
  Q_OBJECT
  
public:
  QmitkFiberGenerateRandomWorker( QThread*, Package4WorkingThread );
  
  public slots:
  void run();
  
private:
  Package4WorkingThread m_itemPackage;
  QThread* m_hostingThread;
  
  
};

class QmitkFiberThreadMonitorWorker : public QObject
{
  Q_OBJECT
  
public:

  
  QmitkFiberThreadMonitorWorker( QThread*, Package4WorkingThread );
  
  void initializeMonitor();
  void threadForFiberProcessingStarted();
  void threadForFiberProcessingFinished();
  void threadForFiberProcessingTerminated();
  void setThreadStatus(QString);
  
  
  public slots:
  void run();
  void fancyMonitorInitialization();
  void fancyMonitorInitializationFinalPos();
  void fancyMonitorInitializationMask();
  void fancyTextFading_threadStarted();
  void fancyTextFading_threadFinished();
  void fancyTextFading_threadTerminated();
  
private:
  Package4WorkingThread m_itemPackage;
  QThread* m_hostingThread;
  QTimer* m_thtimer_initMonitor;
  QTimer* m_thtimer_initMonitorSetFinalPosition;
  QTimer* m_thtimer_initMonitorSetMasks;
  QTimer* m_thtimer_threadStarted;
  QTimer* m_thtimer_threadFinished;
  QTimer* m_thtimer_threadTerminated;
  
  // flags for fancy fading
  bool m_decreaseOpacity_threadStarted;
  bool m_decreaseOpacity_threadFinished;
  bool m_decreaseOpacity_threadTerminated;
  
  // members for fancy animation
  int m_pixelstepper;
  int m_steppingDistance;
  
  
};

// strings to display fiber_thread monitor
const QString FBX_STATUS_IDLE = "idle";
const QString FBX_STATUS_STARTED = "starting";
const QString FBX_STATUS_RUNNING = "running";



// ========= HERE STARTS THE ACTUAL FIBERBUNDLE DEVELOPER VIEW =======

const QString FIB_RADIOBUTTON_DIRECTION_RANDOM = "radioButton_directionRandom";
const QString FIB_RADIOBUTTON_DIRECTION_X      = "radioButton_directionX";
const QString FIB_RADIOBUTTON_DIRECTION_Y      = "radioButton_directionY";
const QString FIB_RADIOBUTTON_DIRECTION_Z      = "radioButton_directionZ";


/*!
 \brief QmitkFiberBundleView
 
 \warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.
 
 \sa QmitkFunctionality
 \ingroup Functionalities
 */
class QmitkFiberBundleDeveloperView : public QmitkFunctionality
{
  
  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
public:
  
  static const std::string VIEW_ID;
  
  QmitkFiberBundleDeveloperView();
  virtual ~QmitkFiberBundleDeveloperView();
  
  virtual void CreateQtPartControl(QWidget *parent);
  
  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();
  virtual void Activated();
  
  protected slots:
  void DoGenerateFibers();
  void DoGenerateFiberIDs();
  void DoUpdateGenerateFibersWidget();
  void SelectionChangedToolBox(int);
  void DoMonitorFiberThreads(int);
  void DoColorFibers();
  
  //SLOTS FOR THREADS
  void BeforeThread_IdGenerate();
  void AfterThread_IdGenerate();
  void BeforeThread_GenerateFibersRandom();
  void AfterThread_GenerateFibersRandom();
  void BeforeThread_FiberColorCoding();
  void AfterThread_FiberColorCoding();
  
  //SLOTS FOR TIMERS
  void UpdateFiberIDTimer();
  void UpdateGenerateRandomFibersTimer();
  void UpdateColorFibersTimer();
  
  
  
protected:
  
  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );
  
  Ui::QmitkFiberBundleDeveloperViewControls* m_Controls;
  
  QmitkStdMultiWidget* m_MultiWidget;
  
  
  
private:
  
  /* METHODS GENERATING FIBERSTRUCTURES */
  void GenerateVtkFibersRandom();
  vtkSmartPointer<vtkPolyData> GenerateVtkFibersDirectionX();
  vtkSmartPointer<vtkPolyData> GenerateVtkFibersDirectionY();
  vtkSmartPointer<vtkPolyData> GenerateVtkFibersDirectionZ();
  
  void PutFibersToDataStorage( vtkPolyData* );
  
  /* METHODS FOR FIBER PROCESSING OR PREPROCESSING  */
  
  
  /* HELPERMETHODS */
  mitk::Geometry3D::Pointer GenerateStandardGeometryForMITK();
  void ResetFiberInfoWidget();
  void FeedFiberInfoWidget();
  void FBXDependendGUIElementsConfigurator();
  
  void SetGeneratedFBX();
  
  
  //contains the selected FiberBundle
  //mitk::FiberBundleX* m_FiberBundleX;
  mitk::WeakPointer<mitk::FiberBundleX> m_FiberBundleX;
  
  //  radiobutton groups
  QVector< QRadioButton* > m_DirectionRadios;
  QVector< QRadioButton* > m_FARadios;
  QVector< QRadioButton* > m_GARadios;
  
  
  // Thread based Workers which do some processing of fibers
  QmitkFiberIDWorker* m_FiberIDGenerator;
  QmitkFiberGenerateRandomWorker* m_GeneratorFibersRandom;
  QmitkFiberColoringWorker* m_FiberColoringSlave;
  
  QThread* m_hostThread;
  QThread* m_monitorThread; 
  bool m_threadInProgress;
  mitk::DataNode::Pointer m_MonitorNode;
  QmitkFiberThreadMonitorWorker *m_fiberThreadMonitorWorker;
  bool m_fiberMonitorIsOn;
  
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

