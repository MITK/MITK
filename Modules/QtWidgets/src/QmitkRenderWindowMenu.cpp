/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkRenderWindowMenu.h>
#include "QmitkRenderWindowMenuBar.h"

// mitk core
#include <mitkExceptionMacro.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkResliceMethodProperty.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkCoreServices.h>

// mitk qt
#include <QmitkIconTheme.h>

// qt
#include <QActionGroup>
#include <QCursor>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSlider>
#include <QWidgetAction>

namespace
{
  const std::string MENU_SIZE_PREFERENCE = "render window menu size";
  const std::string SUBDUE_MENUS_PREFERENCE = "subdue render window menus";

  /** Relative to the full appearance of the bars. */
  constexpr double RESTING_SCALE = 0.75;
  constexpr double RESTING_OPACITY = 0.6;

  enum class MenuSize
  {
    Smaller,
    Default,
    Larger
  };

  MenuSize ReadMenuSize(const mitk::IPreferences *preferences)
  {
    const auto value = preferences->Get(MENU_SIZE_PREFERENCE, "default");

    if ("smaller" == value)
      return MenuSize::Smaller;

    if ("larger" == value)
      return MenuSize::Larger;

    if ("default" != value)
      MITK_WARN << "Unknown render window menu size \"" << value << "\" in the preferences. Using the default size.";

    return MenuSize::Default;
  }

  double GetScale(MenuSize size)
  {
    switch (size)
    {
      case MenuSize::Smaller:
        return 0.75;
      case MenuSize::Larger:
        return 1.25;
      case MenuSize::Default:
        break;
    }

    return 1.0;
  }

  /* One turn per 27 seconds is the speed the auto rotation had when it
   * advanced the 360 position camera stepper every 75 ms.
   */
  constexpr double AUTO_ROTATION_SECONDS_PER_TURN = 27.0;

  /** The menu stores its renderer as the base type, but the lighting rig is
   * declared on mitk::VtkPropRenderer. nullptr if this renderer is not one.
   */
  mitk::VtkPropRenderer *GetPropRenderer(mitk::BaseRenderer *renderer)
  {
    return dynamic_cast<mitk::VtkPropRenderer *>(renderer);
  }

  mitk::IPreferences* GetPreferences()
  {
    auto preferencesService = mitk::CoreServices::GetPreferencesService();
    if (preferencesService->GetSystemPreferences() == nullptr)
      return nullptr;
    return preferencesService->GetSystemPreferences()->Node("org.mitk.editors");
  }
}

unsigned int QmitkRenderWindowMenu::m_DefaultThickMode(1);

QmitkRenderWindowMenu::QmitkRenderWindowMenu(QWidget* parent, mitk::BaseRenderer* baseRenderer)
  : QObject(parent)
  , m_LightingModeButton(nullptr)
  , m_LayoutActionsMenu(nullptr)
  , m_CrosshairMenu(nullptr)
  , m_LightingMenu(nullptr)
  , m_FullScreenMode(false)
  , m_TopLeftBar(nullptr)
  , m_TopRightBar(nullptr)
  , m_PopupOpen(false)
  , m_Renderer(baseRenderer)
  , m_AutoRotationObserverTag(0)
  , m_Parent(parent)
  , m_CrosshairRotationMode(QmitkCrosshairRotationMode::None)
  , m_CrosshairVisibility(true)
  , m_Crosshair3DVisibility(true)
  , m_PreferredLightingMode(mitk::VtkPropRenderer::LightingMode::Studio)
  , m_Layout(LayoutIndex::Axial)
  , m_LayoutDesign(LayoutDesign::DEFAULT)
  , m_OldLayoutDesign(LayoutDesign::DEFAULT)
{
  if (nullptr == m_Parent)
    mitkThrow() << "The render window menu needs the render window it belongs to as its parent.";

  CreateMenuWidget();
  this->UpdateLightingModeButton();
  this->ApplyPreferences();

  if (auto* preferences = GetPreferences(); nullptr != preferences)
    preferences->OnChanged.AddListener(mitk::MessageDelegate1<QmitkRenderWindowMenu, const mitk::IPreferences*>(this, &QmitkRenderWindowMenu::OnPreferencesChanged));

  m_Parent->installEventFilter(this);
}

QmitkRenderWindowMenu::~QmitkRenderWindowMenu()
{
  if (auto* preferences = GetPreferences(); nullptr != preferences)
    preferences->OnChanged.RemoveListener(mitk::MessageDelegate1<QmitkRenderWindowMenu, const mitk::IPreferences*>(this, &QmitkRenderWindowMenu::OnPreferencesChanged));

  this->SetAutoRotation(false);

  // Children of the window rather than of the menu, since they are widgets on it.
  delete m_TopLeftBar;
  delete m_TopRightBar;
}

void QmitkRenderWindowMenu::SetLayoutIndex(LayoutIndex layoutIndex)
{
  m_Layout = layoutIndex;
}

void QmitkRenderWindowMenu::UpdateLayoutDesignList(LayoutDesign layoutDesign)
{
  m_LayoutDesign = layoutDesign;

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

void QmitkRenderWindowMenu::UpdateCrosshair3DVisibility(bool visible)
{
  m_Crosshair3DVisibility = visible;
}

void QmitkRenderWindowMenu::UpdateCrosshairRotationMode(QmitkCrosshairRotationMode mode)
{
  m_CrosshairRotationMode = mode;
}

mitk::VtkPropRenderer::LightingMode QmitkRenderWindowMenu::GetPreferredLightingMode() const
{
  return m_PreferredLightingMode;
}

void QmitkRenderWindowMenu::MoveWidgetToCorrectPos()
{
  m_TopLeftBar->Dock();
  m_TopRightBar->Dock();

  // Layout changes resize the windows under a cursor that does not move, so
  // the cursor position decides rather than the enter and leave events.
  if (m_Parent->rect().contains(m_Parent->mapFromGlobal(QCursor::pos())))
  {
    this->ShowMenu();
  }
  else
  {
    this->HideMenu();
  }
}

void QmitkRenderWindowMenu::ShowMenu()
{
  MITK_DEBUG << "menu showMenu";

  // The window can be switched between 2D and 3D from elsewhere while the menu
  // is hidden.
  this->UpdateLightingModeButton();
  this->UpdateBarVisibility();
  this->UpdateProximity(m_Parent->mapFromGlobal(QCursor::pos()));
}

void QmitkRenderWindowMenu::HideMenu()
{
  MITK_DEBUG << "menu hideEvent";

  // The window reports the cursor as gone once it moves into one of the popup
  // menus. Closing the popup decides anew.
  if (m_PopupOpen)
    return;

  m_TopLeftBar->Conceal();
  m_TopRightBar->Conceal();
}

void QmitkRenderWindowMenu::UpdateLightingModeButton()
{
  m_LightingModeButton->setVisible(m_Renderer.IsNotNull() && m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard3D);
}

void QmitkRenderWindowMenu::UpdateBarVisibility()
{
  if (m_TopRightBar->HasShownButtons())
  {
    m_TopRightBar->Reveal();
  }
  else
  {
    m_TopRightBar->Conceal();
  }

  // Judged by the full sizes, so that a bar growing towards the cursor never
  // pushes the other one out. The upper right bar gives way last since it
  // holds what every window has.
  const bool bothFit = m_TopLeftBar->GetFullSize().width() + m_TopRightBar->GetFullSize().width() <= m_Parent->width();

  if (m_TopLeftBar->HasShownButtons() && bothFit)
  {
    m_TopLeftBar->Reveal();
  }
  else
  {
    m_TopLeftBar->Conceal();
  }
}

void QmitkRenderWindowMenu::UpdateProximity(const QPoint& cursor)
{
  m_TopLeftBar->UpdateProximity(cursor);
  m_TopRightBar->UpdateProximity(cursor);
}

void QmitkRenderWindowMenu::ApplyPreferences()
{
  const auto* preferences = GetPreferences();

  const auto size = nullptr != preferences
    ? ReadMenuSize(preferences)
    : MenuSize::Default;

  const bool subdue = nullptr != preferences
    ? preferences->GetBool(SUBDUE_MENUS_PREFERENCE, true)
    : true;

  // The smaller bars are small enough already, so they only fade at rest.
  const double restingScale = subdue && MenuSize::Smaller != size ? RESTING_SCALE : 1.0;
  const double restingOpacity = subdue ? RESTING_OPACITY : 1.0;

  for (auto* bar : { m_TopLeftBar, m_TopRightBar })
  {
    bar->SetScale(GetScale(size));
    bar->SetRestingAppearance(restingScale, restingOpacity);
  }

  // A different size can make both bars fit next to each other, or no longer.
  if (!m_TopRightBar->isHidden())
    this->ShowMenu();
}

void QmitkRenderWindowMenu::OnPreferencesChanged(const mitk::IPreferences* /*preferences*/)
{
  this->ApplyPreferences();
}

bool QmitkRenderWindowMenu::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == m_Parent && QEvent::MouseMove == event->type())
  {
    const auto* mouseEvent = static_cast<const QMouseEvent*>(event);

    // A held button means the mouse interacts with the scene. Bars growing
    // along the way of a drag would only distract.
    if (Qt::NoButton == mouseEvent->buttons())
      this->UpdateProximity(mouseEvent->position().toPoint());
  }

  return QObject::eventFilter(watched, event);
}

void QmitkRenderWindowMenu::CreateMenuWidget()
{
  m_TopLeftBar = new QmitkRenderWindowMenuBar(QmitkRenderWindowMenuBar::Corner::TopLeft, m_Parent);
  m_TopRightBar = new QmitkRenderWindowMenuBar(QmitkRenderWindowMenuBar::Corner::TopRight, m_Parent);

  m_LightingMenu = new QMenu(m_TopLeftBar);
  connect(m_LightingMenu, &QMenu::aboutToShow, this, &QmitkRenderWindowMenu::OnLightingMenuAboutToShow);
  m_LightingModeButton = m_TopLeftBar->AddMenuButton(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/lighting.svg")), m_LightingMenu);

  m_CrosshairMenu = new QMenu(m_TopRightBar);
  connect(m_CrosshairMenu, &QMenu::aboutToShow, this, &QmitkRenderWindowMenu::OnCrosshairMenuAboutToShow);
  m_CrosshairModeButton = m_TopRightBar->AddMenuButton(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/crosshair.svg")), m_CrosshairMenu);

  m_FullScreenButton = m_TopRightBar->AddButton(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/fullscreen.svg")));
  connect(m_FullScreenButton, &QToolButton::clicked, this, &QmitkRenderWindowMenu::OnFullScreenButton);

  this->CreateSettingsWidget();
  m_LayoutDesignButton = m_TopRightBar->AddMenuButton(QmitkIconTheme::GetIcon(QStringLiteral(":/Qmitk/layout.svg")), m_LayoutActionsMenu);

  for (auto* menu : { m_LightingMenu, m_CrosshairMenu, m_LayoutActionsMenu })
  {
    connect(menu, &QMenu::aboutToShow, this, [this]() {
      m_PopupOpen = true;
    });

    connect(menu, &QMenu::aboutToHide, this, [this]() {
      m_PopupOpen = false;
      this->MoveWidgetToCorrectPos();
    });
  }
}

void QmitkRenderWindowMenu::CreateSettingsWidget()
{
  m_LayoutActionsMenu = new QMenu(m_TopRightBar);

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
  m_FullScreenButton->setIcon(QmitkIconTheme::GetIcon(m_FullScreenMode
    ? QStringLiteral(":/Qmitk/fullscreen_exit.svg")
    : QStringLiteral(":/Qmitk/fullscreen.svg")));
}

void QmitkRenderWindowMenu::AutoRotate(double seconds)
{
  auto* cameraRotationController = m_Renderer->GetCameraRotationController();
  if (nullptr == cameraRotationController)
  {
    return;
  }

  // Deriving the angle from the elapsed time keeps the rotation at the same
  // angular speed when a frame takes longer than the frame interval. The scene
  // then simply gets fewer frames instead of rotating more slowly.
  cameraRotationController->RotateCameraBy(-360.0 * seconds / AUTO_ROTATION_SECONDS_PER_TURN);
}

void QmitkRenderWindowMenu::SetAutoRotation(bool enabled)
{
  if (enabled == (0 != m_AutoRotationObserverTag))
    return;

  auto* renderingManager = mitk::RenderingManager::GetInstance();

  if (enabled)
  {
    m_AutoRotationObserverTag = renderingManager->AddAnimationFrameObserver([this](double seconds) { this->AutoRotate(seconds); });
  }
  else
  {
    renderingManager->RemoveAnimationFrameObserver(m_AutoRotationObserverTag);
    m_AutoRotationObserverTag = 0;
  }
}

void QmitkRenderWindowMenu::OnAutoRotationActionTriggered()
{
  this->SetAutoRotation(0 == m_AutoRotationObserverTag);
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
  resetViewAction->setText("Fit views to all data");
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

  // Show hide crosshair in the 3D render window only
  if (m_Renderer.IsNotNull() && m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard3D)
  {
    QAction *show3DCrosshairVisibilityAction = new QAction(crosshairModesMenu);
    show3DCrosshairVisibilityAction->setText("Show crosshair in 3D window");
    show3DCrosshairVisibilityAction->setCheckable(true);
    show3DCrosshairVisibilityAction->setChecked(m_Crosshair3DVisibility);
    crosshairModesMenu->addAction(show3DCrosshairVisibilityAction);
    connect(show3DCrosshairVisibilityAction, &QAction::toggled, this, &QmitkRenderWindowMenu::OnCrosshair3DVisibilityChanged);
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
    noCrosshairRotation->setChecked(m_CrosshairRotationMode == QmitkCrosshairRotationMode::None);
    noCrosshairRotation->setData(QVariant::fromValue(QmitkCrosshairRotationMode::None));
    crosshairModesMenu->addAction(noCrosshairRotation);

    QAction *singleCrosshairRotation = new QAction(crosshairModesMenu);
    singleCrosshairRotation->setActionGroup(rotationModeActionGroup);
    singleCrosshairRotation->setText("Crosshair rotation");
    singleCrosshairRotation->setCheckable(true);
    singleCrosshairRotation->setChecked(m_CrosshairRotationMode == QmitkCrosshairRotationMode::Single);
    singleCrosshairRotation->setData(QVariant::fromValue(QmitkCrosshairRotationMode::Single));
    crosshairModesMenu->addAction(singleCrosshairRotation);

    QAction *coupledCrosshairRotation = new QAction(crosshairModesMenu);
    coupledCrosshairRotation->setActionGroup(rotationModeActionGroup);
    coupledCrosshairRotation->setText("Coupled crosshair rotation");
    coupledCrosshairRotation->setCheckable(true);
    coupledCrosshairRotation->setChecked(m_CrosshairRotationMode == QmitkCrosshairRotationMode::Coupled);
    coupledCrosshairRotation->setData(QVariant::fromValue(QmitkCrosshairRotationMode::Coupled));
    crosshairModesMenu->addAction(coupledCrosshairRotation);

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
    autoRotationAction->setChecked(0 != m_AutoRotationObserverTag);
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

    int maxTS = 50;
    auto preferences = GetPreferences();
    if (preferences != nullptr)
      maxTS = preferences->GetInt("max TS", 50);

    QSlider *m_TSSlider = new QSlider(crosshairModesMenu);
    m_TSSlider->setMinimum(0);
    m_TSSlider->setMaximum(maxTS);
    m_TSSlider->setValue(currentNum);
    m_TSSlider->setToolTip("Sets the virtual slice thickness.\nIt is the number of slices that are integrated in a maximum intensity projection (MIP).");

    m_TSSlider->setOrientation(Qt::Horizontal);

    connect(m_TSSlider, &QSlider::valueChanged, this, &QmitkRenderWindowMenu::OnTSNumChanged);

    QHBoxLayout *tsLayout = new QHBoxLayout;
    tsLayout->setContentsMargins(4, 4, 4, 4);
    tsLayout->addWidget(new QLabel("TS: "));
    tsLayout->addWidget(m_TSSlider);
    tsLayout->addWidget(m_TSLabel = new QLabel(QString::number(currentNum * 2 + 1)));

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

void QmitkRenderWindowMenu::OnCrosshair3DVisibilityChanged(bool visible)
{
  UpdateCrosshair3DVisibility(visible);
  emit Crosshair3DVisibilityChanged(m_Crosshair3DVisibility);
}

void QmitkRenderWindowMenu::OnCrosshairRotationModeSelected(QAction *action)
{
  UpdateCrosshairRotationMode(action->data().value<QmitkCrosshairRotationMode>());
  emit CrosshairRotationModeChanged(m_CrosshairRotationMode);
}

void QmitkRenderWindowMenu::OnLightingMenuAboutToShow()
{
  m_LightingMenu->clear();

  auto *renderer = GetPropRenderer(m_Renderer);

  if (nullptr == renderer)
    return;

  // Read from the renderer rather than from a remembered value: a volume being
  // rendered installs its own rig without coming through this menu, so anything
  // cached here would tick a rig the window is not on.
  const auto currentMode = renderer->GetLightingMode();

  struct Entry
  {
    mitk::VtkPropRenderer::LightingMode mode;
    const char *label;
  };

  // Labeled as the volume lighting models are, so that the two controls
  // offering them read as one choice rather than two similar ones.
  constexpr Entry entries[] = {
    { mitk::VtkPropRenderer::LightingMode::Studio,    "Default lighting" },
    { mitk::VtkPropRenderer::LightingMode::Headlight, "Headlight"        },
    { mitk::VtkPropRenderer::LightingMode::KeyLight,  "Key light"        }
  };

  QActionGroup *lightingModeActionGroup = new QActionGroup(m_LightingMenu);
  lightingModeActionGroup->setExclusive(true);

  for (const auto &[mode, label] : entries)
  {
    QAction *action = new QAction(label, m_LightingMenu);
    action->setActionGroup(lightingModeActionGroup);
    action->setCheckable(true);
    action->setChecked(mode == currentMode);
    action->setData(static_cast<int>(mode));
    m_LightingMenu->addAction(action);
  }

  connect(lightingModeActionGroup, &QActionGroup::triggered, this, &QmitkRenderWindowMenu::OnLightingModeSelected);

  emit LightingMenuAboutToShow(m_LightingMenu);
}

void QmitkRenderWindowMenu::OnLightingModeSelected(QAction *action)
{
  auto *renderer = GetPropRenderer(m_Renderer);

  if (nullptr == renderer)
    return;

  const auto mode = static_cast<mitk::VtkPropRenderer::LightingMode>(action->data().toInt());

  // Remembered as well as installed: a volume switched on afterwards puts its
  // own rig over this one, and this is what the window returns to once nothing
  // overrides it any more.
  m_PreferredLightingMode = mode;

  renderer->SetLightingMode(mode);

  // Swapping the lights marks the renderer modified but schedules nothing.
  mitk::RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());

  emit LightingModeChanged(mode);
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

void QmitkRenderWindowMenu::OnSetLayout(LayoutDesign layoutDesign)
{
  m_FullScreenMode = false;
  ChangeFullScreenIcon();

  m_LayoutDesign = layoutDesign;
  emit LayoutDesignChanged(m_LayoutDesign);

  ShowMenu();
}
