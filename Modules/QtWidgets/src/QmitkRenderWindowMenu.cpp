/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkRenderWindowMenu.h"

// mitk core
#include "mitkProperties.h"
#include "mitkResliceMethodProperty.h"

// qt
#include <QHBoxLayout>
#include <QPainter>
#include <QSize>
#include <QSpacerItem>
#include <QSlider>
#include <QGroupBox>
#include <QLine>
#include <QRadioButton>
#include <QWidgetAction>

//#include"iconClose.xpm"
#include "iconCrosshairMode.xpm"
#include "iconFullScreen.xpm"
//#include"iconHoriSplit.xpm"
#include "iconSettings.xpm"
//#include"iconVertiSplit.xpm"
#include "iconLeaveFullScreen.xpm"

// c++
#include <cmath>

unsigned int QmitkRenderWindowMenu::m_DefaultThickMode(1);

QmitkRenderWindowMenu::QmitkRenderWindowMenu(QWidget* parent,
                                             Qt::WindowFlags flags,
                                             mitk::BaseRenderer* baseRenderer)
  : QWidget(parent, flags)
  , m_LayoutActionsMenu(nullptr)
  , m_CrosshairMenu(nullptr)
  , m_FullScreenMode(false)
  , m_Renderer(baseRenderer)
  , m_Parent(parent)
  , m_CrosshairRotationMode(0)
  , m_CrosshairVisibility(true)
  , m_Layout(LayoutIndex::AXIAL)
  , m_LayoutDesign(LayoutDesign::DEFAULT)
  , m_OldLayoutDesign(LayoutDesign::DEFAULT)
{
  CreateMenuWidget();
  setMinimumWidth(61); // DIRTY.. If you add or remove a button, you need to change the size.
  setMaximumWidth(61);
  setAutoFillBackground(true);

  this->hide();

  m_AutoRotationTimer = new QTimer(this);
  m_AutoRotationTimer->setInterval(75);

  connect(m_AutoRotationTimer, &QTimer::timeout, this, &QmitkRenderWindowMenu::AutoRotateNextStep);
  connect(m_Parent, &QObject::destroyed, this, &QmitkRenderWindowMenu::deleteLater);
}

QmitkRenderWindowMenu::~QmitkRenderWindowMenu()
{
  if (m_AutoRotationTimer->isActive())
  {
    m_AutoRotationTimer->stop();
  }
}

void QmitkRenderWindowMenu::SetLayoutIndex(LayoutIndex layoutIndex)
{
  m_Layout = layoutIndex;
}

void QmitkRenderWindowMenu::UpdateLayoutDesignList(LayoutDesign layoutDesign)
{
  m_LayoutDesign = layoutDesign;

  if (nullptr == m_LayoutActionsMenu)
  {
    CreateSettingsWidget();
  }

  m_DefaultLayoutAction->setEnabled(true);
  m_All2DTop3DBottomLayoutAction->setEnabled(true);
  m_All2DLeft3DRightLayoutAction->setEnabled(true);
  m_OneBigLayoutAction->setEnabled(true);
  m_Only2DHorizontalLayoutAction->setEnabled(true);
  m_Only2DVerticalLayoutAction->setEnabled(true);
  m_OneTop3DBottomLayoutAction->setEnabled(true);
  m_OneLeft3DRightLayoutAction->setEnabled(true);
  m_AllHorizontalLayoutAction->setEnabled(true);
  m_AllVerticalLayoutAction->setEnabled(true);
  m_RemoveOneLayoutAction->setEnabled(true);

  switch (m_LayoutDesign)
  {
  case LayoutDesign::DEFAULT:
  {
    m_DefaultLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ALL_2D_TOP_3D_BOTTOM:
  {
    m_All2DTop3DBottomLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ALL_2D_LEFT_3D_RIGHT:
  {
    m_All2DLeft3DRightLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ONE_BIG:
  {
    m_OneBigLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ONLY_2D_HORIZONTAL:
  {
    m_Only2DHorizontalLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ONLY_2D_VERTICAL:
  {
    m_Only2DVerticalLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ONE_TOP_3D_BOTTOM:
  {
    m_OneTop3DBottomLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ONE_LEFT_3D_RIGHT:
  {
    m_OneLeft3DRightLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ALL_HORIZONTAL:
  {
    m_AllHorizontalLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::ALL_VERTICAL:
  {
    m_AllVerticalLayoutAction->setEnabled(false);
    break;
  }
  case LayoutDesign::REMOVE_ONE:
  {
    m_RemoveOneLayoutAction->setEnabled(false);
    break;
  }
    case LayoutDesign::NONE:
  {
    break;
  }
  }
}

void QmitkRenderWindowMenu::UpdateCrosshairVisibility(bool visible)
{
  m_CrosshairVisibility = visible;
}

void QmitkRenderWindowMenu::UpdateCrosshairRotationMode(int mode)
{
  m_CrosshairRotationMode = mode;
}

void QmitkRenderWindowMenu::MoveWidgetToCorrectPos()
{
  int moveX = floor(static_cast<double>(this->m_Parent->width()) - static_cast<double>(this->width()) - 4.0);
  this->move(moveX, 3);

  auto cursorPos = this->mapFromGlobal(QCursor::pos());

  if (cursorPos.x() < 0 || cursorPos.x() >= this->width() ||
      cursorPos.y() < 0 || cursorPos.y() >= this->height())
  {
    this->HideMenu();
  }
  else
  {
    this->ShowMenu();
  }
}

void QmitkRenderWindowMenu::ShowMenu()
{
  MITK_DEBUG << "menu showMenu";
  this->show();
  this->raise();
}

void QmitkRenderWindowMenu::HideMenu()
{
  MITK_DEBUG << "menu hideEvent";
  this->hide();
}

void QmitkRenderWindowMenu::paintEvent(QPaintEvent * /*e*/)
{
  QPainter painter(this);
  QColor semiTransparentColor = Qt::black;
  semiTransparentColor.setAlpha(255);
  painter.fillRect(rect(), semiTransparentColor);
}

void QmitkRenderWindowMenu::CreateMenuWidget()
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setAlignment(Qt::AlignRight);
  layout->setContentsMargins(1, 1, 1, 1);

  QSize size(13, 13);

  m_CrosshairMenu = new QMenu(this);
  connect(m_CrosshairMenu, &QMenu::aboutToShow, this, &QmitkRenderWindowMenu::OnCrosshairMenuAboutToShow);

  m_CrosshairModeButton = new QToolButton(this);
  m_CrosshairModeButton->setMaximumSize(15, 15);
  m_CrosshairModeButton->setIconSize(size);
  m_CrosshairModeButton->setMenu(m_CrosshairMenu);
  m_CrosshairModeButton->setIcon(QIcon(QPixmap(iconCrosshairMode_xpm)));
  m_CrosshairModeButton->setPopupMode(QToolButton::InstantPopup);
  m_CrosshairModeButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
  m_CrosshairModeButton->setAutoRaise(true);
  layout->addWidget(m_CrosshairModeButton);

  m_FullScreenButton = new QToolButton(this);
  m_FullScreenButton->setMaximumSize(15, 15);
  m_FullScreenButton->setIconSize(size);
  m_FullScreenButton->setIcon(QIcon(QPixmap(iconFullScreen_xpm)));
  m_FullScreenButton->setAutoRaise(true);
  layout->addWidget(m_FullScreenButton);

  m_LayoutDesignButton = new QToolButton(this);
  m_LayoutDesignButton->setMaximumSize(15, 15);
  m_LayoutDesignButton->setIconSize(size);
  m_LayoutDesignButton->setIcon(QIcon(QPixmap(iconSettings_xpm)));
  m_LayoutDesignButton->setAutoRaise(true);
  layout->addWidget(m_LayoutDesignButton);

  connect(m_FullScreenButton, &QToolButton::clicked, this, &QmitkRenderWindowMenu::OnFullScreenButton);
  connect(m_LayoutDesignButton, &QToolButton::clicked, this, &QmitkRenderWindowMenu::OnLayoutDesignButton);
}

void QmitkRenderWindowMenu::CreateSettingsWidget()
{
  m_LayoutActionsMenu = new QMenu(this);

  m_DefaultLayoutAction = new QAction("Standard layout", m_LayoutActionsMenu);
  m_DefaultLayoutAction->setDisabled(true);

  m_All2DTop3DBottomLayoutAction = new QAction("All 2D top, 3D bottom", m_LayoutActionsMenu);
  m_All2DTop3DBottomLayoutAction->setDisabled(false);

  m_All2DLeft3DRightLayoutAction = new QAction("All 2D left, 3D right", m_LayoutActionsMenu);
  m_All2DLeft3DRightLayoutAction->setDisabled(false);

  m_OneBigLayoutAction = new QAction("This big", m_LayoutActionsMenu);
  m_OneBigLayoutAction->setDisabled(false);

  m_Only2DHorizontalLayoutAction = new QAction("Only 2D horizontal", m_LayoutActionsMenu);
  m_Only2DHorizontalLayoutAction->setDisabled(false);

  m_Only2DVerticalLayoutAction = new QAction("Only 2D vertical", m_LayoutActionsMenu);
  m_Only2DVerticalLayoutAction->setDisabled(false);

  m_OneTop3DBottomLayoutAction = new QAction("This top, 3D bottom", m_LayoutActionsMenu);
  m_OneTop3DBottomLayoutAction->setDisabled(false);

  m_OneLeft3DRightLayoutAction = new QAction("This left, 3D right", m_LayoutActionsMenu);
  m_OneLeft3DRightLayoutAction->setDisabled(false);

  m_AllHorizontalLayoutAction = new QAction("All horizontal", m_LayoutActionsMenu);
  m_AllHorizontalLayoutAction->setDisabled(false);

  m_AllVerticalLayoutAction = new QAction("All vertical", m_LayoutActionsMenu);
  m_AllVerticalLayoutAction->setDisabled(false);

  m_RemoveOneLayoutAction = new QAction("Remove this", m_LayoutActionsMenu);
  m_RemoveOneLayoutAction->setDisabled(false);

  m_LayoutActionsMenu->addAction(m_DefaultLayoutAction);
  m_LayoutActionsMenu->addAction(m_All2DTop3DBottomLayoutAction);
  m_LayoutActionsMenu->addAction(m_All2DLeft3DRightLayoutAction);
  m_LayoutActionsMenu->addAction(m_OneBigLayoutAction);
  m_LayoutActionsMenu->addAction(m_Only2DHorizontalLayoutAction);
  m_LayoutActionsMenu->addAction(m_Only2DVerticalLayoutAction);
  m_LayoutActionsMenu->addAction(m_OneTop3DBottomLayoutAction);
  m_LayoutActionsMenu->addAction(m_OneLeft3DRightLayoutAction);
  m_LayoutActionsMenu->addAction(m_AllHorizontalLayoutAction);
  m_LayoutActionsMenu->addAction(m_AllVerticalLayoutAction);
  m_LayoutActionsMenu->addAction(m_RemoveOneLayoutAction);

  m_LayoutActionsMenu->setVisible(false);

  connect(m_DefaultLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::DEFAULT); });
  connect(m_All2DTop3DBottomLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ALL_2D_TOP_3D_BOTTOM); });
  connect(m_All2DLeft3DRightLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ALL_2D_LEFT_3D_RIGHT); });
  connect(m_OneBigLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ONE_BIG); });
  connect(m_Only2DHorizontalLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ONLY_2D_HORIZONTAL); });
  connect(m_Only2DVerticalLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ONLY_2D_VERTICAL); });
  connect(m_OneTop3DBottomLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ONE_TOP_3D_BOTTOM); });
  connect(m_OneLeft3DRightLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ONE_LEFT_3D_RIGHT); });
  connect(m_AllHorizontalLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ALL_HORIZONTAL); });
  connect(m_AllVerticalLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::ALL_VERTICAL); });
  connect(m_RemoveOneLayoutAction, &QAction::triggered, [this]() { this->OnSetLayout(LayoutDesign::REMOVE_ONE); });
}

void QmitkRenderWindowMenu::ChangeFullScreenIcon()
{
  m_FullScreenButton->setIcon(m_FullScreenMode ? QPixmap(iconLeaveFullScreen_xpm) : QPixmap(iconFullScreen_xpm));
}

void QmitkRenderWindowMenu::AutoRotateNextStep()
{
  if (m_Renderer->GetCameraRotationController())
  {
    m_Renderer->GetCameraRotationController()->GetSlice()->Next();
  }
}

void QmitkRenderWindowMenu::OnAutoRotationActionTriggered()
{
  if (m_AutoRotationTimer->isActive())
  {
    m_AutoRotationTimer->stop();
    m_Renderer->GetCameraRotationController()->GetSlice()->PingPongOff();
  }
  else
  {
    m_Renderer->GetCameraRotationController()->GetSlice()->PingPongOn();
    m_AutoRotationTimer->start();
  }
}

void QmitkRenderWindowMenu::OnTSNumChanged(int num)
{
  MITK_DEBUG << "Thickslices num: " << num << " on renderer " << m_Renderer.GetPointer();

  if (m_Renderer.IsNotNull())
  {
    unsigned int thickSlicesMode = 0;
    // determine the state of the thick-slice mode
    mitk::ResliceMethodProperty *resliceMethodEnumProperty = nullptr;

    if(m_Renderer->GetCurrentWorldPlaneGeometryNode()->GetProperty(resliceMethodEnumProperty, "reslice.thickslices") && resliceMethodEnumProperty)
    {
      thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();
      if(thickSlicesMode!=0)
        m_DefaultThickMode = thickSlicesMode;
    }

    if(thickSlicesMode==0 && num>0) //default mode only for single slices
    {
      thickSlicesMode = m_DefaultThickMode; //mip default
      m_Renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices.showarea",
                                                                  mitk::BoolProperty::New(true));
    }
    if(num<1)
    {
      thickSlicesMode = 0;
      m_Renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices.showarea",
                                                                  mitk::BoolProperty::New(false));
    }

    m_Renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices",
                                                                mitk::ResliceMethodProperty::New(thickSlicesMode));
    m_Renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices.num",
                                                                mitk::IntProperty::New(num));

    m_TSLabel->setText(QString::number(num * 2 + 1));
    m_Renderer->SendUpdateSlice();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRenderWindowMenu::OnCrosshairMenuAboutToShow()
{
  QMenu *crosshairModesMenu = m_CrosshairMenu;

  crosshairModesMenu->clear();

  QAction *resetViewAction = new QAction(crosshairModesMenu);
  resetViewAction->setText("Reset view");
  crosshairModesMenu->addAction(resetViewAction);
  connect(resetViewAction, &QAction::triggered, this, &QmitkRenderWindowMenu::ResetView);

  // Show hide crosshairs
  {
    QAction *showHideCrosshairVisibilityAction = new QAction(crosshairModesMenu);
    showHideCrosshairVisibilityAction->setText("Show crosshair");
    showHideCrosshairVisibilityAction->setCheckable(true);
    showHideCrosshairVisibilityAction->setChecked(m_CrosshairVisibility);
    crosshairModesMenu->addAction(showHideCrosshairVisibilityAction);
    connect(showHideCrosshairVisibilityAction, &QAction::toggled, this, &QmitkRenderWindowMenu::OnCrosshairVisibilityChanged);
  }

  // Rotation mode
  {
    QAction *rotationGroupSeparator = new QAction(crosshairModesMenu);
    rotationGroupSeparator->setSeparator(true);
    rotationGroupSeparator->setText("Rotation mode");
    crosshairModesMenu->addAction(rotationGroupSeparator);

    QActionGroup *rotationModeActionGroup = new QActionGroup(crosshairModesMenu);
    rotationModeActionGroup->setExclusive(true);

    QAction *noCrosshairRotation = new QAction(crosshairModesMenu);
    noCrosshairRotation->setActionGroup(rotationModeActionGroup);
    noCrosshairRotation->setText("No crosshair rotation");
    noCrosshairRotation->setCheckable(true);
    noCrosshairRotation->setChecked(m_CrosshairRotationMode == 0);
    noCrosshairRotation->setData(0);
    crosshairModesMenu->addAction(noCrosshairRotation);

    QAction *singleCrosshairRotation = new QAction(crosshairModesMenu);
    singleCrosshairRotation->setActionGroup(rotationModeActionGroup);
    singleCrosshairRotation->setText("Crosshair rotation");
    singleCrosshairRotation->setCheckable(true);
    singleCrosshairRotation->setChecked(m_CrosshairRotationMode == 1);
    singleCrosshairRotation->setData(1);
    crosshairModesMenu->addAction(singleCrosshairRotation);

    QAction *coupledCrosshairRotation = new QAction(crosshairModesMenu);
    coupledCrosshairRotation->setActionGroup(rotationModeActionGroup);
    coupledCrosshairRotation->setText("Coupled crosshair rotation");
    coupledCrosshairRotation->setCheckable(true);
    coupledCrosshairRotation->setChecked(m_CrosshairRotationMode == 2);
    coupledCrosshairRotation->setData(2);
    crosshairModesMenu->addAction(coupledCrosshairRotation);

    QAction *swivelMode = new QAction(crosshairModesMenu);
    swivelMode->setActionGroup(rotationModeActionGroup);
    swivelMode->setText("Swivel mode");
    swivelMode->setCheckable(true);
    swivelMode->setChecked(m_CrosshairRotationMode == 3);
    swivelMode->setData(3);
    crosshairModesMenu->addAction(swivelMode);

    connect(rotationModeActionGroup, &QActionGroup::triggered, this, &QmitkRenderWindowMenu::OnCrosshairRotationModeSelected);
  }

  // auto rotation support
  if (m_Renderer.IsNotNull() && m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard3D)
  {
    QAction *autoRotationGroupSeparator = new QAction(crosshairModesMenu);
    autoRotationGroupSeparator->setSeparator(true);
    crosshairModesMenu->addAction(autoRotationGroupSeparator);

    QAction *autoRotationAction = crosshairModesMenu->addAction("Auto Rotation");
    autoRotationAction->setCheckable(true);
    autoRotationAction->setChecked(m_AutoRotationTimer->isActive());
    connect(autoRotationAction, &QAction::triggered, this, &QmitkRenderWindowMenu::OnAutoRotationActionTriggered);
  }

  // Thickslices support
  if (m_Renderer.IsNotNull() && m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
  {
    QAction *thickSlicesGroupSeparator = new QAction(crosshairModesMenu);
    thickSlicesGroupSeparator->setSeparator(true);
    thickSlicesGroupSeparator->setText("ThickSlices mode");
    crosshairModesMenu->addAction(thickSlicesGroupSeparator);

    QActionGroup *thickSlicesActionGroup = new QActionGroup(crosshairModesMenu);
    thickSlicesActionGroup->setExclusive(true);

    int currentMode = 0;
    {
      mitk::ResliceMethodProperty::Pointer m = dynamic_cast<mitk::ResliceMethodProperty *>(
        m_Renderer->GetCurrentWorldPlaneGeometryNode()->GetProperty("reslice.thickslices"));
      if (m.IsNotNull())
        currentMode = m->GetValueAsId();
    }

    int currentNum = 1;
    {
      mitk::IntProperty::Pointer m = dynamic_cast<mitk::IntProperty *>(
        m_Renderer->GetCurrentWorldPlaneGeometryNode()->GetProperty("reslice.thickslices.num"));
      if (m.IsNotNull())
      {
        currentNum = m->GetValue();
      }
    }

    if (currentMode == 0)
      currentNum = 0;

    QSlider *m_TSSlider = new QSlider(crosshairModesMenu);
    m_TSSlider->setMinimum(0);
    m_TSSlider->setMaximum(50);
    m_TSSlider->setValue(currentNum);

    m_TSSlider->setOrientation(Qt::Horizontal);

    connect(m_TSSlider, &QSlider::valueChanged, this, &QmitkRenderWindowMenu::OnTSNumChanged);

    QHBoxLayout *tsLayout = new QHBoxLayout;
    tsLayout->setContentsMargins(4, 4, 4, 4);
    tsLayout->addWidget(new QLabel("TS: "));
    tsLayout->addWidget(m_TSSlider);
    tsLayout->addWidget(m_TSLabel = new QLabel(QString::number(currentNum * 2 + 1), this));

    QWidget *tsWidget = new QWidget;
    tsWidget->setLayout(tsLayout);

    QWidgetAction *m_TSSliderAction = new QWidgetAction(crosshairModesMenu);
    m_TSSliderAction->setDefaultWidget(tsWidget);
    crosshairModesMenu->addAction(m_TSSliderAction);
  }
}

void QmitkRenderWindowMenu::OnCrosshairVisibilityChanged(bool visible)
{
  UpdateCrosshairVisibility(visible);
  emit CrosshairVisibilityChanged(m_CrosshairVisibility);
}

void QmitkRenderWindowMenu::OnCrosshairRotationModeSelected(QAction *action)
{
  UpdateCrosshairRotationMode(action->data().toInt());
  emit CrosshairRotationModeChanged(m_CrosshairRotationMode);
}

void QmitkRenderWindowMenu::OnFullScreenButton(bool /*checked*/)
{
  if (!m_FullScreenMode)
  {
    m_FullScreenMode = true;
    m_OldLayoutDesign = m_LayoutDesign;

    emit LayoutDesignChanged(LayoutDesign::ONE_BIG);
  }
  else
  {
    m_FullScreenMode = false;
    emit LayoutDesignChanged(m_OldLayoutDesign);
  }

  MoveWidgetToCorrectPos();
  ChangeFullScreenIcon();
  ShowMenu();
}

void QmitkRenderWindowMenu::OnLayoutDesignButton(bool /*checked*/)
{
  if (nullptr == m_LayoutActionsMenu)
  {
    CreateSettingsWidget();
  }

  QPoint point = mapToGlobal(m_LayoutDesignButton->geometry().topLeft());
  m_LayoutActionsMenu->setVisible(true);
  m_LayoutActionsMenu->exec(point);
}

void QmitkRenderWindowMenu::OnSetLayout(LayoutDesign layoutDesign)
{
  m_FullScreenMode = false;
  ChangeFullScreenIcon();

  m_LayoutDesign = layoutDesign;
  emit LayoutDesignChanged(m_LayoutDesign);

  ShowMenu();
}
