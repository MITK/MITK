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
  //mitk::FiberBundleX* m_FBX;

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
  //mitk::FiberBundleX* m_FBX;
  
  Package4WorkingThread m_itemPackage;
  QThread* m_hostingThread;
  
  
};





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
  
  //SLOTS FOR THREADS
  void BeforeThread_IdGenerate();
  void AfterThread_IdGenerate();
  void BeforeThread_GenerateFibersRandom();
  void AfterThread_GenerateFibersRandom();
  
  //SLOTS FOR TIMERS
  void UpdateFiberIDTimer();
  void UpdateGenerateRandomFibersTimer();
  
    
  
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
  void FBXDependendGUIElementsConfigurator(bool);
  
  void SetGeneratedFBX();
  
  
  //contains the selected FiberBundle
  //mitk::FiberBundleX* m_FiberBundleX;
  mitk::WeakPointer<mitk::FiberBundleX> m_FiberBundleX;
  
//  radiobutton groups
  QVector< QRadioButton* > m_DirectionRadios;
  QVector< QRadioButton* > m_FARadios;
  QVector< QRadioButton* > m_GARadios;
  
  
  // Thread based Workers which do some processing of fibers
  QmitkFiberIDWorker * m_FiberIDGenerator;
  QmitkFiberGenerateRandomWorker * m_GeneratorFibersRandom;
  
  QThread * m_hostThread;
  bool m_threadInProgress;
  

};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

