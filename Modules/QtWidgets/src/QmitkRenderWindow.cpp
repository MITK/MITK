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

#include "QmitkRenderWindow.h"

#include <QCursor>
#include <QDesktopServices>
#include <QDir>
#include <QLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QTextBrowser>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QScrollArea>
#include <QSurfaceFormat>
#include <QTemporaryFile>
#include <QWindow>
#include "mitkMousePressEvent.h"
#include "mitkMouseMoveEvent.h"
#include "mitkMouseDoubleClickEvent.h"
#include "mitkMouseReleaseEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkMouseWheelEvent.h"
#include "mitkInternalEvent.h"
#include "mitkStructuredReport.h"
#include "mitkStructuredReportMapper.h"

#include "QmitkRenderWindowMenu.h"
#include "QmitkMimeTypes.h"

Q_DECLARE_METATYPE(mitk::StructuredReport*);
Q_DECLARE_METATYPE(mitk::DataStorage*);

QmitkRenderWindow::QmitkRenderWindow(QWidget *parent,
  QString name,
  mitk::VtkPropRenderer* /*renderer*/,
  mitk::RenderingManager* renderingManager, mitk::BaseRenderer::RenderingMode::Type renderingMode, bool useFXAA) :
  QVTKWidget(parent), m_ResendQtEvents(true), m_MenuWidget(NULL), m_MenuWidgetActivated(false), m_LayoutIndex(0), 
  m_FullScreenMode(false),
  m_WindowPlaneIsSelected(false),
  m_DropEnabled(true),
  m_IsTwoDimensionalView(false)
{
  // Needed if QVTKWidget2 is used instead of QVTKWidget
  //this will be fixed in VTK source if change 18864 is accepted
  /*QGLFormat newform = this->format();
  newform.setSamples(8);
  this->setFormat(newform);*/

  // init crosshair position by 0/0
  m_CrosshairPosition[0] = 0;
  m_CrosshairPosition[1] = 0;

  QSurfaceFormat surfaceFormat = windowHandle()->format();
  surfaceFormat.setStencilBufferSize(8);
  windowHandle()->setFormat(surfaceFormat);

  if (renderingMode == mitk::BaseRenderer::RenderingMode::DepthPeeling)
  {
    GetRenderWindow()->SetMultiSamples(0);
    GetRenderWindow()->SetAlphaBitPlanes(1);
  }
  else if (renderingMode == mitk::BaseRenderer::RenderingMode::MultiSampling)
  {
    GetRenderWindow()->SetMultiSamples(8);
  }
  else if (renderingMode == mitk::BaseRenderer::RenderingMode::Standard)
  {
    GetRenderWindow()->SetMultiSamples(0);
  }

  Initialize(renderingManager, name.toStdString().c_str(), renderingMode); // Initialize mitkRenderWindowBase

  setFocusPolicy(Qt::StrongFocus);
  setMouseTracking(true);

  initStructuredReportWidget();
}

QmitkRenderWindow::~QmitkRenderWindow()
{
  freeStructuredReportWidget();
  Destroy(); // Destroy mitkRenderWindowBase
}

void QmitkRenderWindow::initStructuredReportWidget()
{
  m_StructuredReportWidget = new QWidget(this);
  m_StructuredReportWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

  class SRPropagateFilter : public QObject {
  public:
    QmitkRenderWindow* window;
    bool eventFilter(QObject* object, QEvent* event) override{
      if (event->type() == QEvent::MouseMove) {
        window->event(event);
      } else if (event->type() == QEvent::MouseButtonPress) {
        window->event(event);
      }
      return false;
    }
  };

  SRPropagateFilter* pf = new SRPropagateFilter();
  pf->window = this;

  QVBoxLayout* srLayout = new QVBoxLayout();
  m_StructuredReportWidget->setLayout(srLayout);

  QVBoxLayout* mainLayout = new QVBoxLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(mainLayout);
  mainLayout->addWidget(m_StructuredReportWidget);

  m_StructuredReportWidget->setVisible(false);
  
  QPushButton* browserButton = new QPushButton(m_StructuredReportWidget);
  browserButton->setText(tr("View in Browser"));
  srLayout->addWidget(browserButton);

  m_StructuredReportText = new QTextBrowser(m_StructuredReportWidget);
  m_StructuredReportText->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
  srLayout->addWidget(m_StructuredReportText);

  connect(browserButton, &QAbstractButton::pressed, [this]() {
    mitk::StructuredReport* rep = m_StructuredReportWidget->property("StructuredReportPointer").value<mitk::StructuredReport*>();

    QDir tempDir(QDir::tempPath() + "/autoplan_sr");
    if (!tempDir.exists()) {
      tempDir.mkpath(".");
    }

    QString postfix = "";
    int counter = 0;
    while (tempDir.exists("sr_report" + postfix + ".html") && counter < 255) {
      counter++;
      postfix = QString::number(counter);
    }

    if (counter >= 255) {
      return;
    }

    QFile f(tempDir.filePath("sr_report" + postfix + ".html"));
    f.open(QFile::OpenModeFlag::WriteOnly);
    f.write(rep->GetReportText().c_str());
    f.close();

    QDesktopServices::openUrl(QUrl("file:///" + f.fileName()));
  });

  m_StructuredReportText->viewport()->installEventFilter(pf);
}

void QmitkRenderWindow::freeStructuredReportWidget()
{
  mitk::DataStorage* ds = m_StructuredReportWidget->property("connectedDataStorage").value<mitk::DataStorage*>();
  if (ds != nullptr) {
    auto s = ds->GetAll();
    for (const auto& node : *s) {
      RemoveNodeCallback(node);
    }
  }

  delete m_StructuredReportWidget;
}

void QmitkRenderWindow::fillStructuredReportWidget(mitk::StructuredReport* report)
{
  m_StructuredReportText->setText(QString::fromStdString(report->GetReportText()));
  m_StructuredReportWidget->setProperty("StructuredReportPointer", QVariant::fromValue(report));
}

void QmitkRenderWindow::UpdateStructuredReportVisibility(mitk::StructuredReport* report, bool visible)
{
  if (visible) {
    m_VisibleReports.insert(report);
  } else {
    m_VisibleReports.erase(report);
  }

  bool hasVisibleReport = m_VisibleReports.size() > 0;
  m_StructuredReportWidget->setVisible(hasVisibleReport);
  if (hasVisibleReport) {
    fillStructuredReportWidget(*m_VisibleReports.begin());
  }
}

void QmitkRenderWindow::SetResendQtEvents(bool resend)
{
  m_ResendQtEvents = resend;
}

void QmitkRenderWindow::SetLayoutIndex(unsigned int layoutIndex)
{
  m_LayoutIndex = layoutIndex;
  if (m_MenuWidget)
    m_MenuWidget->SetLayoutIndex(layoutIndex);
}

unsigned int QmitkRenderWindow::GetLayoutIndex()
{
  if (m_MenuWidget)
    return m_MenuWidget->GetLayoutIndex();
  else
    return 0;
}

void QmitkRenderWindow::LayoutDesignListChanged(int layoutDesignIndex)
{
  if (m_MenuWidget)
    m_MenuWidget->UpdateLayoutDesignList(layoutDesignIndex);
}

void QmitkRenderWindow::mousePressEvent(QMouseEvent *me)
{
  //Get mouse position in vtk display coordinate system. me contains qt display infos...
  mitk::Point2D displayPos = GetMousePosition(me);

  mitk::MousePressEvent::Pointer mPressEvent = mitk::MousePressEvent::New(m_Renderer,
                                                              displayPos,
                                                              GetButtonState(me),
                                                              GetModifiers(me), GetEventButton(me));

  if (!this->HandleEvent(mPressEvent.GetPointer()))
  {
    QVTKWidget::mousePressEvent(me);
  }

  m_CrosshairPosition = displayPos;

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseDoubleClickEvent( QMouseEvent *me )
{
  mitk::Point2D displayPos = GetMousePosition(me);
  mitk::MouseDoubleClickEvent::Pointer mPressEvent = mitk::MouseDoubleClickEvent::New(m_Renderer,displayPos, GetButtonState(me),
    GetModifiers(me), GetEventButton(me));

  if (!this->HandleEvent(mPressEvent.GetPointer()))
  {
    QVTKWidget::mousePressEvent(me);
  }

  m_CrosshairPosition = displayPos;

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseReleaseEvent(QMouseEvent *me)
{
  mitk::Point2D displayPos = GetMousePosition(me);
  mitk::MouseReleaseEvent::Pointer mReleaseEvent = mitk::MouseReleaseEvent::New(m_Renderer, displayPos, GetButtonState(me),
      GetModifiers(me), GetEventButton(me));

  if (!this->HandleEvent(mReleaseEvent.GetPointer()))
  {
    QVTKWidget::mouseReleaseEvent(me);
  }

  m_CrosshairPosition = displayPos;

  if (m_ResendQtEvents)
    me->ignore();
}

void QmitkRenderWindow::mouseMoveEvent(QMouseEvent *me)
{
  mitk::Point2D displayPos = GetMousePosition(me);

  this->AdjustRenderWindowMenuVisibility(me->pos());

  mitk::MouseMoveEvent::Pointer mMoveEvent = mitk::MouseMoveEvent::New(m_Renderer, displayPos, GetButtonState(me),
      GetModifiers(me));

  if (!this->HandleEvent(mMoveEvent.GetPointer()))
  {
    QVTKWidget::mouseMoveEvent(me);
  }
}

void QmitkRenderWindow::AddNodeCallback(const mitk::DataNode* node)
{
  mitk::StructuredReportMapper* srMapper = dynamic_cast<mitk::StructuredReportMapper*>(node->GetMapper(mitk::BaseRenderer::Standard3D));

  if (srMapper != nullptr) {
    srMapper->updateQtActorContext = this;
    srMapper->updateQtActor = [](mitk::StructuredReport* report, bool visible, void* context) {
      QmitkRenderWindow* window = (QmitkRenderWindow*)context;
      window->UpdateStructuredReportVisibility(report, visible);
    };
  }
}

void QmitkRenderWindow::RemoveNodeCallback(const mitk::DataNode* node)
{
  mitk::StructuredReportMapper* srMapper = dynamic_cast<mitk::StructuredReportMapper*>(node->GetMapper(mitk::BaseRenderer::Standard3D));
  if (srMapper != nullptr) {
    srMapper->freeConnection();
  }
}

void QmitkRenderWindow::ConnectStructuredReportWidgetToDataStorage(mitk::DataStorage* ds)
{
  ds->AddNodeEvent.AddListener(mitk::MessageDelegate1<QmitkRenderWindow, const mitk::DataNode*>(this, &QmitkRenderWindow::AddNodeCallback));
  ds->RemoveNodeEvent.AddListener(mitk::MessageDelegate1<QmitkRenderWindow, const mitk::DataNode*>(this, &QmitkRenderWindow::RemoveNodeCallback));

  m_StructuredReportWidget->setProperty("connectedDataStorage", QVariant::fromValue(ds));

  auto s = ds->GetAll();
  for (const auto& node : *s) {
    AddNodeCallback(node);
  }
}

void QmitkRenderWindow::wheelEvent(QWheelEvent *we)
{
  // Note: wheel action should not change x/y camera position, get existing one
  mitk::Point2D displayPos;
  if( m_CrosshairPosition[0] == 0 && m_CrosshairPosition[1] == 0 )
  {
    // We should take center of render window (we have no access to crosshair)
    // Mouse cursor position could be anywhere
    // Note: it's still being not fully correct because crosshair could be moved from center
    // by scrolling on another projection but editor cursor (crosshair) is not available here
    // possible solution could be writing cursor position from editors to renderwindows on change
    // though it's not fully safe, test this solution first
    displayPos[0] = m_Renderer->GetVtkRenderer()->GetCenter()[0];
    displayPos[1] = m_Renderer->GetVtkRenderer()->GetCenter()[1];
    // We need to convert the y component, as the display and vtk have other definitions for the y direction
    displayPos[1] = m_Renderer->GetSizeY() - displayPos[1];
  }
  else
  {
    displayPos = m_CrosshairPosition;
  }

  mitk::MouseWheelEvent::Pointer mWheelEvent = mitk::MouseWheelEvent::New(m_Renderer, displayPos, GetButtonState(we),
      GetModifiers(we), GetDelta(we));

  if (!this->HandleEvent(mWheelEvent.GetPointer()) && !(we->modifiers() & Qt::CTRL))
  {
    QVTKWidget::wheelEvent(we);
  }

  if (m_ResendQtEvents)
    we->ignore();
}

void QmitkRenderWindow::keyPressEvent(QKeyEvent *ke)
{
  mitk::InteractionEvent::ModifierKeys modifiers = GetModifiers(ke);
  std::string key = GetKeyLetter(ke);

  mitk::InteractionKeyEvent::Pointer keyEvent = mitk::InteractionKeyEvent::New(m_Renderer, key, modifiers);
  if (!this->HandleEvent(keyEvent.GetPointer()))
  {
    QVTKWidget::keyPressEvent(ke);
  }

  if (keyEvent->accepted) {
    ke->accept();
  } else if (m_ResendQtEvents) {
    ke->ignore();
  }
}

void QmitkRenderWindow::enterEvent(QEvent *e)
{
  // TODO implement new event
  QVTKWidget::enterEvent(e);
}

void QmitkRenderWindow::DeferredHideMenu()
{
  MITK_DEBUG << "QmitkRenderWindow::DeferredHideMenu";

  if (m_MenuWidget)
    m_MenuWidget->HideMenu();
}

void QmitkRenderWindow::leaveEvent(QEvent *e)
{
  mitk::InternalEvent::Pointer internalEvent = mitk::InternalEvent::New(this->m_Renderer, NULL, "LeaveRenderWindow");

  this->HandleEvent(internalEvent.GetPointer());

  if (m_MenuWidget)
    m_MenuWidget->smoothHide();

  QVTKWidget::leaveEvent(e);
}

void QmitkRenderWindow::paintEvent(QPaintEvent* /*event*/)
{
  //We are using our own interaction and thus have to call the rendering manually.
  this->GetRenderer()->GetRenderingManager()->RequestUpdate(GetRenderWindow());
}

void QmitkRenderWindow::moveEvent(QMoveEvent* event)
{
  QVTKWidget::moveEvent(event);

  // after a move the overlays need to be positioned
  emit moved();
}

void QmitkRenderWindow::showEvent(QShowEvent* event)
{
  QVTKWidget::showEvent(event);

  // this singleshot is necessary to have the overlays positioned correctly after initial show
  // simple call of moved() is no use here!!
  QTimer::singleShot(0, this, SIGNAL( moved() ));
}

void QmitkRenderWindow::ActivateMenuWidget(bool state, QmitkStdMultiWidget* stdMultiWidget)
{
  m_MenuWidgetActivated = state;

  if (!m_MenuWidgetActivated && m_MenuWidget)
  {
    //disconnect Signal/Slot Connection
    disconnect(m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)));
    disconnect(m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)));
    disconnect(m_MenuWidget, &QmitkRenderWindowMenu::fullScreenModeChanged, this, &QmitkRenderWindow::OnFullScreenModeChanged);

    delete m_MenuWidget;
    m_MenuWidget = 0;
  }
  else if (m_MenuWidgetActivated && !m_MenuWidget)
  {
    //create render window MenuBar for split, close Window or set new setting.
    m_MenuWidget = new QmitkRenderWindowMenu(this, 0, m_Renderer, stdMultiWidget, m_FullScreenMode);
    m_MenuWidget->SetLayoutIndex(m_LayoutIndex);

    //create Signal/Slot Connection
    connect(m_MenuWidget, SIGNAL( SignalChangeLayoutDesign(int) ), this, SLOT(OnChangeLayoutDesign(int)));
    connect(m_MenuWidget, SIGNAL( ChangeCrosshairRotationMode(int) ), this, SIGNAL( ChangeCrosshairRotationMode(int)));
    connect(m_MenuWidget, &QmitkRenderWindowMenu::fullScreenModeChanged, this, &QmitkRenderWindow::OnFullScreenModeChanged);
  }
}

void QmitkRenderWindow::OnFullScreenModeChanged(bool fullscreen)
{
  m_FullScreenMode = fullscreen;
}

void QmitkRenderWindow::resizeEvent(QResizeEvent * e)
{
  QVTKWidget::resizeEvent(e);
  emit resized();
}

void QmitkRenderWindow::AdjustRenderWindowMenuVisibility(const QPoint& /*pos*/)
{
  if (m_MenuWidget)
  {
    m_MenuWidget->MoveWidgetToCorrectPos(1.0f);
    m_MenuWidget->ShowMenu();
  }
}

void QmitkRenderWindow::HideRenderWindowMenu()
{
  if (m_MenuWidget) {
    m_MenuWidget->hide();
  }
}

void QmitkRenderWindow::OnChangeLayoutDesign(int layoutDesignIndex)
{
  emit SignalLayoutDesignChanged(layoutDesignIndex);
}

void QmitkRenderWindow::OnWidgetPlaneModeChanged(int mode)
{
  mitk::InternalEvent::Pointer internalEvent = mitk::InternalEvent::New(m_Renderer, nullptr, "RotationModeChanged");
  this->HandleEvent(internalEvent.GetPointer());

  if (m_MenuWidget)
    m_MenuWidget->NotifyNewWidgetPlanesMode(mode);
}

void QmitkRenderWindow::FullScreenMode(bool state)
{
  m_FullScreenMode = state;
  if (m_MenuWidget)
    m_MenuWidget->SetFullScreenMode(state);
}

void QmitkRenderWindow::dragEnterEvent(QDragEnterEvent *event)
{
  if (event->mimeData()->hasFormat("application/x-mitk-datanodes"))
  {
    event->accept();
  }
}

void QmitkRenderWindow::dropEvent(QDropEvent * event)
{
  QList<mitk::DataNode*> dataNodeList = QmitkMimeTypes::ToDataNodePtrList(event->mimeData());
  if (!dataNodeList.empty() && m_DropEnabled)
  {
    emit NodesDropped(this, dataNodeList.toVector().toStdVector());
  }
}

mitk::Point2D QmitkRenderWindow::GetMousePosition(QMouseEvent* me) const
{
  mitk::Point2D point;
  point[0] = me->x();
  // We need to convert the y component, as the display and vtk have other definitions for the y direction
  point[1] = m_Renderer->GetSizeY() - me->y();

  return point;
}

mitk::Point2D QmitkRenderWindow::GetMousePosition(QWheelEvent* we) const
{
  mitk::Point2D point;
  point[0] = we->x();
  //We need to convert the y component, as the display and vtk have other definitions for the y direction
  point[1] = m_Renderer->GetSizeY() - we->y();

  return point;
}

mitk::InteractionEvent::MouseButtons QmitkRenderWindow::GetEventButton(QMouseEvent* me) const
{
  mitk::InteractionEvent::MouseButtons eventButton;
  switch (me->button())
  {
  case Qt::LeftButton:
    eventButton = mitk::InteractionEvent::LeftMouseButton;
    break;
  case Qt::RightButton:
    eventButton = mitk::InteractionEvent::RightMouseButton;
    break;
  case Qt::MidButton:
    eventButton = mitk::InteractionEvent::MiddleMouseButton;
    break;
  default:
    eventButton = mitk::InteractionEvent::NoButton;
    break;
  }
  return eventButton;
}

mitk::InteractionEvent::MouseButtons QmitkRenderWindow::GetButtonState(QMouseEvent* me) const
{
  mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;

  if (me->buttons() & Qt::LeftButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::LeftMouseButton;
  }
  if (me->buttons() & Qt::RightButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::RightMouseButton;
  }
  if (me->buttons() & Qt::MidButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::MiddleMouseButton;
  }
  return buttonState;
}

mitk::InteractionEvent::ModifierKeys QmitkRenderWindow::GetModifiers(QInputEvent* me) const
{
  mitk::InteractionEvent::ModifierKeys modifiers = mitk::InteractionEvent::NoKey;

  if (me->modifiers() & Qt::ALT)
  {
    modifiers = modifiers | mitk::InteractionEvent::AltKey;
  }
  if (me->modifiers() & Qt::CTRL)
  {
    modifiers = modifiers | mitk::InteractionEvent::ControlKey;
  }
  if (me->modifiers() & Qt::SHIFT)
  {
    modifiers = modifiers | mitk::InteractionEvent::ShiftKey;
  }
  return modifiers;
}

mitk::InteractionEvent::MouseButtons QmitkRenderWindow::GetButtonState(QWheelEvent* we) const
{
  mitk::InteractionEvent::MouseButtons buttonState = mitk::InteractionEvent::NoButton;

  if (we->buttons() & Qt::LeftButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::LeftMouseButton;
  }
  if (we->buttons() & Qt::RightButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::RightMouseButton;
  }
  if (we->buttons() & Qt::MidButton)
  {
    buttonState = buttonState | mitk::InteractionEvent::MiddleMouseButton;
  }
  return buttonState;
}

std::string QmitkRenderWindow::GetKeyLetter(QKeyEvent *ke) const
{
  // Converting Qt Key Event to string element.
  std::string key = "";
  int tkey = ke->key();
  if (tkey < 128)
  { //standard ascii letter
    key = (char)toupper(tkey);
  }
  else
  { // special keys
    switch (tkey)
    {
    case Qt::Key_Return:
      key = mitk::InteractionEvent::KeyReturn;
      break;
    case Qt::Key_Enter:
      key = mitk::InteractionEvent::KeyEnter;
      break;
    case Qt::Key_Escape:
      key = mitk::InteractionEvent::KeyEsc;
      break;
    case Qt::Key_Delete:
      key = mitk::InteractionEvent::KeyDelete;
      break;
    case Qt::Key_Up:
      key = mitk::InteractionEvent::KeyArrowUp;
      break;
    case Qt::Key_Down:
      key = mitk::InteractionEvent::KeyArrowDown;
      break;
    case Qt::Key_Left:
      key = mitk::InteractionEvent::KeyArrowLeft;
      break;
    case Qt::Key_Right:
      key = mitk::InteractionEvent::KeyArrowRight;
      break;

    case Qt::Key_F1:
      key = mitk::InteractionEvent::KeyF1;
      break;
    case Qt::Key_F2:
      key = mitk::InteractionEvent::KeyF2;
      break;
    case Qt::Key_F3:
      key = mitk::InteractionEvent::KeyF3;
      break;
    case Qt::Key_F4:
      key = mitk::InteractionEvent::KeyF4;
      break;
    case Qt::Key_F5:
      key = mitk::InteractionEvent::KeyF5;
      break;
    case Qt::Key_F6:
      key = mitk::InteractionEvent::KeyF6;
      break;
    case Qt::Key_F7:
      key = mitk::InteractionEvent::KeyF7;
      break;
    case Qt::Key_F8:
      key = mitk::InteractionEvent::KeyF8;
      break;
    case Qt::Key_F9:
      key = mitk::InteractionEvent::KeyF9;
      break;
    case Qt::Key_F10:
      key = mitk::InteractionEvent::KeyF10;
      break;
    case Qt::Key_F11:
      key = mitk::InteractionEvent::KeyF11;
      break;
    case Qt::Key_F12:
      key = mitk::InteractionEvent::KeyF12;
      break;

    case Qt::Key_End:
      key = mitk::InteractionEvent::KeyEnd;
      break;
    case Qt::Key_Home:
      key = mitk::InteractionEvent::KeyPos1;
      break;
    case Qt::Key_Insert:
      key = mitk::InteractionEvent::KeyInsert;
      break;
    case Qt::Key_PageDown:
      key = mitk::InteractionEvent::KeyPageDown;
      break;
    case Qt::Key_PageUp:
      key = mitk::InteractionEvent::KeyPageUp;
      break;
    case Qt::Key_Space:
      key = mitk::InteractionEvent::KeySpace;
      break;
    }
  }
  return key;
}

int QmitkRenderWindow::GetDelta(QWheelEvent* we) const
{
  return we->delta();
}

bool QmitkRenderWindow::GetFullSreenMode()
{
  return m_FullScreenMode;
}

bool QmitkRenderWindow::isWindow3d()
{
  return mitk::BaseRenderer::GetInstance(GetRenderWindow())->GetMapperID() == mitk::BaseRenderer::Standard3D;
}

bool QmitkRenderWindow::windowPlaneIsSelected()
{
  return m_WindowPlaneIsSelected;
}

void QmitkRenderWindow::setWindowPlaneIsSelected(bool selected)
{
  if (m_WindowPlaneIsSelected == selected) {
    return;
  }

  m_WindowPlaneIsSelected = selected;
  emit windowPlaneSelectionChanged();
}

void QmitkRenderWindow::setDropMode(bool state)
{
  m_DropEnabled = state;
}

bool QmitkRenderWindow::getDropMode()
{
  return m_DropEnabled;
}

void QmitkRenderWindow::setTwoDimensionalViewState(bool state)
{
  m_IsTwoDimensionalView = state;
}

bool QmitkRenderWindow::isTwoDimensionalViewState()
{
  return m_IsTwoDimensionalView;
}

void QmitkRenderWindow::updateAllWindows()
{
  if (m_MenuWidget) {
    m_MenuWidget->updateWindows();
  }
}
