/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkStdMultiWidgetTest.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include <mitkGlobalInteraction.h>
#include <mitkIOUtil.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestingMacros.h>

#include <QmitkRenderingManagerFactory.h>
#include <QmitkApplicationCursor.h>

#include <QmitkStdMultiWidget.h>

#include <mitkItkSignalCollector.cpp>
#include <mitkQtSignalCollector.cpp>
#include <QmitkStdMultiWidgetState.h>


class QmitkStdMultiWidgetTestClassPrivate
{
public:
  QmitkStdMultiWidgetTestClassPrivate();

  std::string FileName;
  mitk::DataStorage::Pointer DataStorage;
  mitk::RenderingManager::Pointer RenderingManager;

  mitk::DataNode::Pointer ImageNode;
  mitk::Image* Image;

  QmitkStdMultiWidget* Viewer;

  QmitkStdMultiWidgetTestClass::ViewerStateTester::Pointer StateTester;

  bool InteractiveMode;

  mitk::Vector3D SpacingsInWorld;

  QmitkRenderWindow* AxialWindow;
  QmitkRenderWindow* SagittalWindow;
  QmitkRenderWindow* CoronalWindow;
  QmitkRenderWindow* _3DWindow;
  
  mitk::SliceNavigationController* AxialSnc;
  mitk::SliceNavigationController* SagittalSnc;
  mitk::SliceNavigationController* CoronalSnc;

  mitk::DisplayGeometry* AxialDisplayGeometry;
  mitk::DisplayGeometry* SagittalDisplayGeometry;
  mitk::DisplayGeometry* CoronalDisplayGeometry;

  mitk::FocusEvent FocusEvent;
  mitk::SliceNavigationController::GeometrySliceEvent GeometrySliceEvent;
  itk::ModifiedEvent ModifiedEvent;
};


// --------------------------------------------------------------------------
QmitkStdMultiWidgetTestClassPrivate::QmitkStdMultiWidgetTestClassPrivate()
: ImageNode(0)
, Image(0)
, Viewer(0)
, InteractiveMode(false)
, GeometrySliceEvent(NULL, 0)
{
  this->SpacingsInWorld.Fill(1.0);
}

// --------------------------------------------------------------------------
QmitkStdMultiWidgetTestClass::QmitkStdMultiWidgetTestClass()
: QObject()
, d_ptr(new QmitkStdMultiWidgetTestClassPrivate())
{
}


// --------------------------------------------------------------------------
QmitkStdMultiWidgetTestClass::~QmitkStdMultiWidgetTestClass()
{
}


// --------------------------------------------------------------------------
std::string QmitkStdMultiWidgetTestClass::GetFileName() const
{
  Q_D(const QmitkStdMultiWidgetTestClass);
  return d->FileName;
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::SetFileName(const std::string& fileName)
{
  Q_D(QmitkStdMultiWidgetTestClass);
  d->FileName = fileName;
}


// --------------------------------------------------------------------------
bool QmitkStdMultiWidgetTestClass::GetInteractiveMode() const
{
  Q_D(const QmitkStdMultiWidgetTestClass);
  return d->InteractiveMode;
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::SetInteractiveMode(bool interactiveMode)
{
  Q_D(QmitkStdMultiWidgetTestClass);
  d->InteractiveMode = interactiveMode;
}


// --------------------------------------------------------------------------
QPoint QmitkStdMultiWidgetTestClass::GetPointAtCursorPosition(QmitkRenderWindow *renderWindow, const mitk::Vector2D& cursorPosition)
{
  QRect rect = renderWindow->rect();
  double x = cursorPosition[0] * rect.width();
  double y = (1.0 - cursorPosition[1]) * rect.height();
  return QPoint(x, y);
}


// --------------------------------------------------------------------------
mitk::Vector2D QmitkStdMultiWidgetTestClass::GetCursorPositionAtPoint(QmitkRenderWindow *renderWindow, const QPoint& point)
{
  QRect rect = renderWindow->rect();
  mitk::Vector2D cursorPosition;
  cursorPosition[0] = double(point.x()) / rect.width();
  cursorPosition[1] = 1.0 - double(point.y()) / rect.height();
  return cursorPosition;
}


// --------------------------------------------------------------------------
bool QmitkStdMultiWidgetTestClass::Equals(const mitk::Point3D& selectedPosition1, const mitk::Point3D& selectedPosition2)
{
  Q_D(QmitkStdMultiWidgetTestClass);

  for (int i = 0; i < 3; ++i)
  {
    double tolerance = d->SpacingsInWorld[i] / 2.0;
    if (std::abs(selectedPosition1[i] - selectedPosition2[i]) > tolerance)
    {
      return false;
    }
  }

  return true;
}


// --------------------------------------------------------------------------
bool QmitkStdMultiWidgetTestClass::Equals(const mitk::Vector2D& cursorPosition1, const mitk::Vector2D& cursorPosition2, double tolerance)
{
  return std::abs(cursorPosition1[0] - cursorPosition2[0]) <= tolerance && std::abs(cursorPosition1[1] - cursorPosition2[1]) <= tolerance;
}


// --------------------------------------------------------------------------
bool QmitkStdMultiWidgetTestClass::Equals(const std::vector<mitk::Vector2D>& cursorPositions1, const std::vector<mitk::Vector2D>& cursorPositions2, double tolerance)
{
  return cursorPositions1.size() == std::size_t(3)
      && cursorPositions2.size() == std::size_t(3)
      && Self::Equals(cursorPositions1[0], cursorPositions2[0], tolerance)
      && Self::Equals(cursorPositions1[1], cursorPositions2[1], tolerance)
      && Self::Equals(cursorPositions1[2], cursorPositions2[2], tolerance);
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::initTestCase()
{
  Q_D(QmitkStdMultiWidgetTestClass);

  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetInstance();
  globalInteraction->Initialize("global");

  /// Create and register RenderingManagerFactory for this platform.
  static QmitkRenderingManagerFactory qmitkRenderingManagerFactory;
  Q_UNUSED(qmitkRenderingManagerFactory);

  /// Create one instance
  static QmitkApplicationCursor globalQmitkApplicationCursor;
  Q_UNUSED(globalQmitkApplicationCursor);

  d->DataStorage = mitk::StandaloneDataStorage::New();

  d->RenderingManager = mitk::RenderingManager::GetInstance();
  d->RenderingManager->SetDataStorage(d->DataStorage);

  /// Disable VTK warnings. For some reason, they appear using these tests, but
  /// not with the real application. We simply suppress them here.
  vtkObject::GlobalWarningDisplayOff();

  std::vector<std::string> files;
  files.push_back(d->FileName);

  mitk::IOUtil::LoadFiles(files, *(d->DataStorage.GetPointer()));
  mitk::DataStorage::SetOfObjects::ConstPointer allImages = d->DataStorage->GetAll();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(allImages->size(), 1), ".. Test image loaded.");

  d->ImageNode = (*allImages)[0];
  d->ImageNode->SetVisibility(true);

  d->Image = dynamic_cast<mitk::Image*>(d->ImageNode->GetData());
  
  // TODO
  // Fill d->SpacingsInWorld with correct values. Expected axis order is Sagittal, Coronal, Axial, i.e. in world coordinate order.  
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::cleanupTestCase()
{
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::init()
{
  Q_D(QmitkStdMultiWidgetTestClass);

  d->Viewer = new QmitkStdMultiWidget(0, 0, d->RenderingManager);
  d->Viewer->SetDataStorage(d->DataStorage);
  d->Viewer->setObjectName(tr("QmitkStdMultiWidget"));

  d->Viewer->resize(1024, 1024);
  d->Viewer->show();

  QTest::qWaitForWindowShown(d->Viewer);

  d->AxialWindow = d->Viewer->GetRenderWindow1();
  d->SagittalWindow = d->Viewer->GetRenderWindow2();
  d->CoronalWindow = d->Viewer->GetRenderWindow3();
  d->_3DWindow = d->Viewer->GetRenderWindow4();

  d->ImageNode->SetVisibility(true);
  d->ImageNode->SetVisibility(true, d->AxialWindow->GetRenderer());
  d->ImageNode->SetVisibility(true, d->SagittalWindow->GetRenderer());
  d->ImageNode->SetVisibility(true, d->CoronalWindow->GetRenderer());
  d->ImageNode->SetVisibility(true, d->_3DWindow->GetRenderer());
  
  d->RenderingManager->RequestUpdateAll();

  /// Create a state tester that works for all of the test functions.

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  d->AxialSnc = d->AxialWindow->GetSliceNavigationController();
  d->SagittalSnc = d->SagittalWindow->GetSliceNavigationController();
  d->CoronalSnc = d->CoronalWindow->GetSliceNavigationController();
  d->AxialDisplayGeometry = d->AxialWindow->GetRenderer()->GetDisplayGeometry();
  d->SagittalDisplayGeometry = d->SagittalWindow->GetRenderer()->GetDisplayGeometry();
  d->CoronalDisplayGeometry = d->CoronalWindow->GetRenderer()->GetDisplayGeometry();

  d->StateTester = ViewerStateTester::New(d->Viewer);

  d->StateTester->Connect(focusManager, d->FocusEvent);
  d->StateTester->Connect(d->AxialSnc, d->GeometrySliceEvent);
  d->StateTester->Connect(d->SagittalSnc, d->GeometrySliceEvent);
  d->StateTester->Connect(d->CoronalSnc, d->GeometrySliceEvent);
  d->StateTester->Connect(d->AxialDisplayGeometry, d->ModifiedEvent);
  d->StateTester->Connect(d->SagittalDisplayGeometry, d->ModifiedEvent);
  d->StateTester->Connect(d->CoronalDisplayGeometry, d->ModifiedEvent);
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::cleanup()
{
  Q_D(QmitkStdMultiWidgetTestClass);

  /// Release the pointer so that the desctructor is be called.
  d->StateTester = 0;

  if (d->InteractiveMode)
  {
    QEventLoop loop;
    loop.connect(d->Viewer, SIGNAL(destroyed()), SLOT(quit()));
    loop.exec();
  }

  delete d->Viewer;
  d->Viewer = 0;
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::testViewer()
{
  Q_D(QmitkStdMultiWidgetTestClass);

  /// Tests if the viewer has been successfully created.
  QVERIFY(d->Viewer);
}


// --------------------------------------------------------------------------
void QmitkStdMultiWidgetTestClass::testResize()
{
  Q_D(QmitkStdMultiWidgetTestClass);

  ViewerState::Pointer expectedState = ViewerState::New(d->Viewer);
  
  // TODO Set the expected state.
  
  // d->StateTester->SetExpectedState(expectedState);
  
  d->Viewer->resize(800, 800);

  QCOMPARE(d->StateTester->GetItkSignals(d->AxialSnc).size(), std::size_t(0));
  QCOMPARE(d->StateTester->GetItkSignals(d->GeometrySliceEvent).size(), std::size_t(0));
  QCOMPARE(d->StateTester->GetItkSignals(d->AxialDisplayGeometry, d->ModifiedEvent).size(), std::size_t(1));
  QCOMPARE(d->StateTester->GetItkSignals(d->SagittalDisplayGeometry, d->ModifiedEvent).size(), std::size_t(1));
  QCOMPARE(d->StateTester->GetItkSignals(d->CoronalDisplayGeometry, d->ModifiedEvent).size(), std::size_t(1));
  QCOMPARE(d->StateTester->GetItkSignals().size(), std::size_t(3));
  QCOMPARE(d->StateTester->GetQtSignals().size(), std::size_t(1));

  d->StateTester->Clear();
  
  // Test other public functions or interactions.

}


// --------------------------------------------------------------------------
static void ShiftArgs(int& argc, char* argv[], int steps = 1)
{
  /// We exploit that there must be a NULL pointer after the arguments.
  /// (Guaranteed by the standard.)
  int i = 1;
  do
  {
    argv[i] = argv[i + steps];
    ++i;
  }
  while (argv[i - 1]);
  argc -= steps;
}


// --------------------------------------------------------------------------
int QmitkStdMultiWidgetTest(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Q_UNUSED(app);

  QmitkStdMultiWidgetTestClass test;

  std::string interactiveModeOption("-i");
  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == interactiveModeOption)
    {
      test.SetInteractiveMode(true);
      ::ShiftArgs(argc, argv);
      break;
    }
  }

  if (argc < 2)
  {
    MITK_INFO << "Missing argument. No image file given.";
    return 1;
  }

  test.SetFileName(argv[1]);
  ::ShiftArgs(argc, argv);

  /// We used the arguments to initialise the test. No arguments is passed
  /// to the Qt test, so that all the test functions are executed.
//  argc = 1;
//  argv[1] = NULL;
  return QTest::qExec(&test, argc, argv);
}
