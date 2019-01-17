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

#include "QmitkRenderWindowMenu.h"

#include "mitkResliceMethodProperty.h"
#include "mitkProperties.h"

#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSize>
#include <QPainter>
#include <QSlider>
#include <QComboBox>

#include<QGroupBox>
#include<QRadioButton>
#include<QAction>
#include<QLine>
#include<QLabel>
#include<QWidgetAction>

#include <QTimer>

#include "QmitkStdMultiWidget.h"

//#include"iconClose.xpm"
#include"iconFullScreen.xpm"
#include"iconCrosshairMode.xpm"
//#include"iconHoriSplit.xpm"
#include"iconSettings.xpm"
//#include"iconVertiSplit.xpm"
#include"iconLeaveFullScreen.xpm"

#include <math.h>

unsigned int QmitkRenderWindowMenu::m_DefaultThickMode(1);

#ifdef QMITK_USE_EXTERNAL_RENDERWINDOW_MENU
QmitkRenderWindowMenu::QmitkRenderWindowMenu(QWidget *parent, Qt::WindowFlags f, mitk::BaseRenderer *b, QmitkStdMultiWidget* mw, bool fullScreenMode)
:QWidget(NULL, Qt::Tool | Qt::FramelessWindowHint ),

#else
QmitkRenderWindowMenu::QmitkRenderWindowMenu(QWidget *parent, Qt::WindowFlags f, mitk::BaseRenderer *b, QmitkStdMultiWidget* mw, bool fullScreenMode)
:QWidget(parent,f),
#endif

m_Settings(NULL),
m_CrosshairMenu(NULL),
m_Layout(0),
m_LayoutDesign(0),
m_OldLayoutDesign(0),
m_FullScreenMode(fullScreenMode),
m_Entered(false),
m_Renderer(b),
m_MultiWidget(mw),
m_Parent(parent)
{

  MITK_DEBUG << "creating renderwindow menu on baserenderer " << b;

  //Create Menu Widget
  this->CreateMenuWidget();
  this->setMinimumWidth(62); //DIRTY.. If you add or remove a button, you need to change the size.
  this->setMaximumWidth(62);
  this->setAutoFillBackground( true );

  //Else part fixes the render window menu issue on Linux bug but caused bugs on Mac OS and Windows
  //for Mac OS see bug 3192
  //for Windows see bug 12130
  //... so Mac OS and Windows must be treated differently:
#if defined(Q_OS_MAC)
  this->show();
  this->setWindowOpacity(0.0f);
#else
  this->setVisible(false);
#endif

  //this->setAttribute( Qt::WA_NoSystemBackground  );
  //this->setBackgroundRole( QPalette::Dark );
  //this->update();

  //SetOpacity  --  its just posible if the widget is a window.
  //Windows indicates that the widget is a window, usually with a window system frame and a title bar,
  //irrespective of whether the widget has a parent or not.
  /*
  this->setWindowFlags( Qt::Window | Qt::FramelessWindowHint);
  */
  //this->setAttribute(Qt::WA_TranslucentBackground);
  //this->setWindowOpacity(0.75);

  currentCrosshairRotationMode = 0;

  // for autorotating
  m_AutoRotationTimer.setInterval( 75 );
  connect( &m_AutoRotationTimer, SIGNAL(timeout()), this, SLOT(AutoRotateNextStep()) );
  m_HideTimer.setSingleShot(true);
  connect( &m_HideTimer, SIGNAL(timeout()), this, SLOT(DeferredHideMenu()));
  connect(m_Parent, SIGNAL(destroyed()), this, SLOT(deleteLater()));
}

QmitkRenderWindowMenu::~QmitkRenderWindowMenu()
{
  if( m_AutoRotationTimer.isActive() )
    m_AutoRotationTimer.stop();
}



void QmitkRenderWindowMenu::CreateMenuWidget()
{
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setAlignment( Qt::AlignHCenter );
  layout->setContentsMargins(1,1,1,1);

  QSize size( 13, 13 );

  m_CrosshairMenu = new QMenu(this);
  connect( m_CrosshairMenu, SIGNAL( aboutToShow() ), this, SLOT(OnCrossHairMenuAboutToShow()) );

  // button for changing rotation mode
  m_CrosshairModeButton = new QToolButton(this);
  m_CrosshairModeButton->setMaximumSize(16, 16);
  m_CrosshairModeButton->setIconSize(size);
  m_CrosshairModeButton->setMenu( m_CrosshairMenu );
  m_CrosshairModeButton->setIcon(QIcon(QPixmap(iconCrosshairMode_xpm)));
  m_CrosshairModeButton->setPopupMode(QToolButton::InstantPopup);
  m_CrosshairModeButton->setStyleSheet("QToolButton::menu-indicator { image: none; }");
  m_CrosshairModeButton->setAutoRaise(true);
  layout->addWidget( m_CrosshairModeButton );

  //fullScreenButton
  m_FullScreenButton = new QToolButton(this);
  m_FullScreenButton->setMaximumSize(16, 16);
  m_FullScreenButton->setIconSize(size);
  m_FullScreenButton->setIcon(QIcon(QPixmap(iconFullScreen_xpm)));
  m_FullScreenButton->setAutoRaise(true);
  layout->addWidget( m_FullScreenButton );
  this->ChangeFullScreenIcon();

  //settingsButton
  m_SettingsButton = new QToolButton(this);
  m_SettingsButton->setMaximumSize(16, 16);
  m_SettingsButton->setIconSize(size);
  m_SettingsButton->setIcon(QIcon(QPixmap(iconSettings_xpm)));
  m_SettingsButton->setAutoRaise(true);
  layout->addWidget( m_SettingsButton );

  //Create Connections -- coming soon?
  connect( m_FullScreenButton, SIGNAL( clicked(bool) ), this, SLOT(OnFullScreenButton(bool)) );
  connect( m_SettingsButton, SIGNAL( clicked(bool) ), this, SLOT(OnSettingsButton(bool)) );
}

void QmitkRenderWindowMenu::CreateSettingsWidget()
{
  m_Settings = new QMenu(this);

  m_DefaultLayoutAction = new QAction( QAction::tr("standard layout"), m_Settings );
  m_DefaultLayoutAction->setDisabled( true );

  m_2DImagesUpLayoutAction = new QAction( QAction::tr("2D images top, 3D bottom"), m_Settings );
  m_2DImagesUpLayoutAction->setDisabled( false );

  m_2DImagesLeftLayoutAction = new QAction( QAction::tr("2D images left, 3D right"), m_Settings );
  m_2DImagesLeftLayoutAction->setDisabled( false );

  m_Big3DLayoutAction = new QAction( QAction::tr("Big 3D"), m_Settings );
  m_Big3DLayoutAction->setDisabled( false );

  m_Widget1LayoutAction = new QAction( QAction::tr("Axial plane"), m_Settings );
  m_Widget1LayoutAction->setDisabled( false );

  m_Widget2LayoutAction = new QAction( QAction::tr("Sagittal plane"), m_Settings );
  m_Widget2LayoutAction->setDisabled( false );

  m_Widget3LayoutAction = new QAction( QAction::tr("Coronal plane"), m_Settings );
  m_Widget3LayoutAction->setDisabled( false );

  m_RowWidget3And4LayoutAction = new QAction( QAction::tr("Coronal top, 3D bottom"), m_Settings );
  m_RowWidget3And4LayoutAction->setDisabled( false );

  m_ColumnWidget3And4LayoutAction = new QAction( QAction::tr("Coronal left, 3D right"), m_Settings );
  m_ColumnWidget3And4LayoutAction->setDisabled( false );

  m_SmallUpperWidget2Big3and4LayoutAction = new QAction( QAction::tr("Sagittal top, Coronal n 3D bottom"), m_Settings );
  m_SmallUpperWidget2Big3and4LayoutAction->setDisabled( false );

  m_2x2Dand3DWidgetLayoutAction = new QAction( QAction::tr("Axial n Sagittal left, 3D right"), m_Settings );
  m_2x2Dand3DWidgetLayoutAction->setDisabled( false );

  m_Left2Dand3DRight2DLayoutAction = new QAction( QAction::tr("Axial n 3D left, Sagittal right"), m_Settings );
  m_Left2Dand3DRight2DLayoutAction->setDisabled( false );

  m_LeftAxialRight2DLayoutAction = new QAction( QAction::tr("Axial left, 2D right"), m_Settings );
  m_LeftAxialRight2DLayoutAction->setDisabled( false );

  m_Settings->addAction(m_DefaultLayoutAction);
  m_Settings->addAction(m_2DImagesUpLayoutAction);
  m_Settings->addAction(m_2DImagesLeftLayoutAction);
  m_Settings->addAction(m_Big3DLayoutAction);
  m_Settings->addAction(m_Widget1LayoutAction);
  m_Settings->addAction(m_Widget2LayoutAction);
  m_Settings->addAction(m_Widget3LayoutAction);
  m_Settings->addAction(m_RowWidget3And4LayoutAction);
  m_Settings->addAction(m_ColumnWidget3And4LayoutAction);
  m_Settings->addAction(m_SmallUpperWidget2Big3and4LayoutAction);
  m_Settings->addAction(m_2x2Dand3DWidgetLayoutAction);
  m_Settings->addAction(m_Left2Dand3DRight2DLayoutAction);
  m_Settings->addAction(m_LeftAxialRight2DLayoutAction);

  m_Settings->setVisible( false );

  connect( m_DefaultLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToDefault(bool)) );
  connect( m_2DImagesUpLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutTo2DImagesUp(bool)) );
  connect( m_2DImagesLeftLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutTo2DImagesLeft(bool)) );
  connect( m_Big3DLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToBig3D(bool)) );
  connect( m_Widget1LayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToWidget1(bool)) );
  connect( m_Widget2LayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToWidget2(bool)) );
  connect( m_Widget3LayoutAction  , SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToWidget3(bool)) );
  connect( m_RowWidget3And4LayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToRowWidget3And4(bool)) );
  connect( m_ColumnWidget3And4LayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToColumnWidget3And4(bool)) );
  connect( m_SmallUpperWidget2Big3and4LayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToSmallUpperWidget2Big3and4(bool)) );
  connect( m_2x2Dand3DWidgetLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutTo2x2Dand3DWidget(bool)) );
  connect( m_Left2Dand3DRight2DLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToLeft2Dand3DRight2D(bool)) );
  connect( m_LeftAxialRight2DLayoutAction, SIGNAL( triggered(bool) ), this, SLOT(OnChangeLayoutToAxialLeft2DRight(bool)) );

}

void QmitkRenderWindowMenu::paintEvent( QPaintEvent*  /*e*/ )
{
  QPainter painter(this);
  QColor semiTransparentColor = Qt::black;
  semiTransparentColor.setAlpha(255);
  painter.fillRect(rect(), semiTransparentColor);
}

void QmitkRenderWindowMenu::SetLayoutIndex( unsigned int layoutIndex )
{
  m_Layout = layoutIndex;
}

void QmitkRenderWindowMenu::HideMenu( )
{
  MITK_DEBUG << "menu hideEvent";
  DeferredHideMenu();
}

void QmitkRenderWindowMenu::ShowMenu( )
{
  MITK_DEBUG << "menu showMenu";
  DeferredShowMenu();
}


void QmitkRenderWindowMenu::enterEvent( QEvent * /*e*/ )
{
  MITK_DEBUG << "menu enterEvent";
  DeferredShowMenu();

  m_Entered=true;
}

void QmitkRenderWindowMenu::DeferredHideMenu( )
{
  MITK_DEBUG << "menu deferredhidemenu";
  //Else part fixes the render window menu issue on Linux bug but caused bugs on Mac OS and Windows
  //for Mac OS see bug 3192
  //for Windows see bug 12130
  //... so Mac OS and Windows must be treated differently:
#if defined(Q_OS_MAC)
  this->setWindowOpacity(0.0f);
#else
  this->setVisible(false);
#endif

//    setVisible(false);
//    setWindowOpacity(0.0f);
  ///hide();
}

void QmitkRenderWindowMenu::leaveEvent( QEvent * /*e*/ )
{
  MITK_DEBUG << "menu leaveEvent";

  m_Entered = false;
  smoothHide();
}

/* This method is responsible for non fluttering of
 the renderWindowMenu when mouse cursor moves along the renderWindowMenu*/
void QmitkRenderWindowMenu::smoothHide()
{

  MITK_DEBUG<< "menu leaveEvent";
  m_HideTimer.start(10);
}


void QmitkRenderWindowMenu::ChangeFullScreenMode( bool state )
{
  this->OnFullScreenButton( state );
}
/// \brief
void QmitkRenderWindowMenu::OnFullScreenButton( bool  /*checked*/ )
{
  if( !m_FullScreenMode )
  {
    m_FullScreenMode = true;
    m_OldLayoutDesign = m_LayoutDesign;

    switch( m_Layout )
    {
    case AXIAL:
      {
        emit SignalChangeLayoutDesign( LAYOUT_AXIAL );
        break;
      }

    case SAGITTAL:
      {
        emit SignalChangeLayoutDesign( LAYOUT_SAGITTAL );
        break;
      }
    case CORONAL:
      {
        emit SignalChangeLayoutDesign( LAYOUT_CORONAL );
        break;
      }
    case THREE_D:
      {
        emit SignalChangeLayoutDesign( LAYOUT_BIG3D );
        break;
      }
    }
  }
  else
  {
    m_FullScreenMode = false;
    emit SignalChangeLayoutDesign( m_OldLayoutDesign );
  }

  emit fullScreenModeChanged(m_FullScreenMode);

  //Move Widget and show again
  this->MoveWidgetToCorrectPos(1.0f);

  //change icon
  this->ChangeFullScreenIcon();
  DeferredShowMenu( );
}


/// \brief
void QmitkRenderWindowMenu::OnSettingsButton( bool  /*checked*/ )
{
  if( m_Settings == NULL )
    this->CreateSettingsWidget();

  QPoint point = this->mapToGlobal( m_SettingsButton->geometry().topLeft() );
  m_Settings->setVisible( true );
  m_Settings->exec( point );
}

void QmitkRenderWindowMenu::OnChangeLayoutTo2DImagesUp(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_2DIMAGEUP;
  emit SignalChangeLayoutDesign( LAYOUT_2DIMAGEUP );

  DeferredShowMenu( );

}
void QmitkRenderWindowMenu::OnChangeLayoutTo2DImagesLeft(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_2DIMAGELEFT;
  emit SignalChangeLayoutDesign( LAYOUT_2DIMAGELEFT );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToDefault(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_DEFAULT;
  emit SignalChangeLayoutDesign( LAYOUT_DEFAULT );

  DeferredShowMenu( );

}

void QmitkRenderWindowMenu::DeferredShowMenu()
{

  MITK_DEBUG << "deferred show menu";
  m_HideTimer.stop();

  //Else part fixes the render window menu issue on Linux bug but caused bugs on Mac OS and Windows
  //for Mac OS see bug 3192
  //for Windows see bug 12130
  //... so Mac OS and Windows must be treated differently:
#if defined(Q_OS_MAC)
  this->setWindowOpacity(1.0f);
#else
  this->setVisible(true);
  this->raise();
#endif
}

void QmitkRenderWindowMenu::OnChangeLayoutToBig3D(bool)
{
  MITK_DEBUG << "OnChangeLayoutToBig3D";

  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_BIG3D;
  emit SignalChangeLayoutDesign( LAYOUT_BIG3D );

  DeferredShowMenu( );

}

void QmitkRenderWindowMenu::OnChangeLayoutToWidget1(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_AXIAL;
  emit SignalChangeLayoutDesign( LAYOUT_AXIAL );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToWidget2(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_SAGITTAL;
  emit SignalChangeLayoutDesign( LAYOUT_SAGITTAL );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToWidget3(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_CORONAL;
  emit SignalChangeLayoutDesign( LAYOUT_CORONAL );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToRowWidget3And4(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_ROWWIDGET3AND4;
  emit SignalChangeLayoutDesign( LAYOUT_ROWWIDGET3AND4 );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToColumnWidget3And4(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_COLUMNWIDGET3AND4;
  emit SignalChangeLayoutDesign( LAYOUT_COLUMNWIDGET3AND4 );

  DeferredShowMenu( );
}

void QmitkRenderWindowMenu::OnChangeLayoutToSmallUpperWidget2Big3and4(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_SMALLUPPERWIDGET2BIGAND4;
  emit SignalChangeLayoutDesign( LAYOUT_SMALLUPPERWIDGET2BIGAND4 );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutTo2x2Dand3DWidget(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_2X2DAND3DWIDGET;
  emit SignalChangeLayoutDesign( LAYOUT_2X2DAND3DWIDGET );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToLeft2Dand3DRight2D(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_LEFT2DAND3DRIGHT2D;
  emit SignalChangeLayoutDesign( LAYOUT_LEFT2DAND3DRIGHT2D );

  DeferredShowMenu( );
}
void QmitkRenderWindowMenu::OnChangeLayoutToAxialLeft2DRight(bool)
{
  //set Full Screen Mode to false, if Layout Design was changed by the LayoutDesign_List
  m_FullScreenMode = false;
  this->ChangeFullScreenIcon();

  m_LayoutDesign = LAYOUT_LEFTAXIAL2DRIGHT;
  emit SignalChangeLayoutDesign( LAYOUT_LEFTAXIAL2DRIGHT );

  DeferredShowMenu( );
}

void QmitkRenderWindowMenu::UpdateLayoutDesignList( int layoutDesignIndex )
{
  m_LayoutDesign = layoutDesignIndex;

  if( m_Settings == NULL )
    this->CreateSettingsWidget();

  switch( m_LayoutDesign )
  {
  case LAYOUT_DEFAULT:
    {
      m_DefaultLayoutAction->setEnabled(false);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }

  case LAYOUT_2DIMAGEUP:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(false);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_2DIMAGELEFT:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(false);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_BIG3D:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(false);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_AXIAL:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(false);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_SAGITTAL:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(false);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_CORONAL:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(false);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_2X2DAND3DWIDGET:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(false);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_ROWWIDGET3AND4:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(false);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_COLUMNWIDGET3AND4:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(false);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_SMALLUPPERWIDGET2BIGAND4:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(false);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_LEFT2DAND3DRIGHT2D:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(false);
      m_LeftAxialRight2DLayoutAction->setEnabled(true);
      break;
    }
  case LAYOUT_LEFTAXIAL2DRIGHT:
    {
      m_DefaultLayoutAction->setEnabled(true);
      m_2DImagesUpLayoutAction->setEnabled(true);
      m_2DImagesLeftLayoutAction->setEnabled(true);
      m_Big3DLayoutAction->setEnabled(true);
      m_Widget1LayoutAction->setEnabled(true);
      m_Widget2LayoutAction->setEnabled(true);
      m_Widget3LayoutAction->setEnabled(true);
      m_RowWidget3And4LayoutAction->setEnabled(true);
      m_ColumnWidget3And4LayoutAction->setEnabled(true);
      m_SmallUpperWidget2Big3and4LayoutAction->setEnabled(true);
      m_2x2Dand3DWidgetLayoutAction->setEnabled(true);
      m_Left2Dand3DRight2DLayoutAction->setEnabled(true);
      m_LeftAxialRight2DLayoutAction->setEnabled(false);
      break;
    }
  }
}

#ifdef QMITK_USE_EXTERNAL_RENDERWINDOW_MENU
void QmitkRenderWindowMenu::MoveWidgetToCorrectPos(float opacity)
#else
void QmitkRenderWindowMenu::MoveWidgetToCorrectPos(float /*opacity*/)
#endif
{
#ifdef QMITK_USE_EXTERNAL_RENDERWINDOW_MENU
  int X=floor( double(this->m_Parent->width()/2 - this->width()/2) );
  int Y=7;

  QPoint pos = this->m_Parent->mapToGlobal( QPoint(0,0) );

  this->move( X+pos.x(), Y+pos.y() );

  if(opacity<0) opacity=0;
  else if(opacity>1) opacity=1;

  this->setWindowOpacity(opacity);
#else
  int moveX= floor( double(this->m_Parent->width()/2 - this->width()/2) );
  this->move( moveX, 3 );
  this->show();
#endif
}

void QmitkRenderWindowMenu::ChangeFullScreenIcon()
{
  m_FullScreenButton->setIcon(m_FullScreenMode
    ? QPixmap(iconLeaveFullScreen_xpm)
    : QPixmap(iconFullScreen_xpm));
}

void QmitkRenderWindowMenu::OnCrosshairRotationModeSelected(QAction* action)
{
  MITK_DEBUG << "selected crosshair mode " << action->data().toInt() ;
  emit ChangeCrosshairRotationMode( action->data().toInt() );
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
    m_Renderer->GetRenderingManager()->RequestUpdateAll();
  }
}



void QmitkRenderWindowMenu::OnCrossHairMenuAboutToShow()
{
  QMenu *crosshairModesMenu = m_CrosshairMenu;

  crosshairModesMenu->clear();

  // Show/hide crosshair
  {
    QAction* showHideCrosshairAction = crosshairModesMenu->addAction(tr("Show Crosshair"));
    showHideCrosshairAction->setCheckable(true);
    showHideCrosshairAction->setChecked(m_MultiWidget->crosshairManager->getShowPlanesIn3D());
    connect(showHideCrosshairAction, &QAction::triggered, this, [this] (bool state) {
      m_MultiWidget->crosshairManager->setShowPlanesIn3d(state);
      updateWindows();
    });
  }

  // Rotation mode
  {
    QAction* rotationGroupSeparator = new QAction(crosshairModesMenu);
    rotationGroupSeparator->setSeparator(true);
    rotationGroupSeparator->setText(tr("Rotation mode"));
    crosshairModesMenu->addAction( rotationGroupSeparator );

    QActionGroup* rotationModeActionGroup = new QActionGroup(crosshairModesMenu);
    rotationModeActionGroup->setExclusive(true);

    QAction* noCrosshairRotation = new QAction(crosshairModesMenu);
    noCrosshairRotation->setActionGroup(rotationModeActionGroup);
    noCrosshairRotation->setText(tr("Crosshair rotation"));
    noCrosshairRotation->setCheckable(true);
    noCrosshairRotation->setChecked(currentCrosshairRotationMode==0);
    noCrosshairRotation->setData( 0 );
    crosshairModesMenu->addAction( noCrosshairRotation );

    QAction* coupledCrosshairRotation = new QAction(crosshairModesMenu);
    coupledCrosshairRotation->setActionGroup(rotationModeActionGroup);
    coupledCrosshairRotation->setText(tr("Coupled crosshair rotation"));
    coupledCrosshairRotation->setCheckable(true);
    coupledCrosshairRotation->setChecked(currentCrosshairRotationMode==2);
    coupledCrosshairRotation->setData( 2 );
    crosshairModesMenu->addAction( coupledCrosshairRotation );

    QAction* swivelMode = new QAction(crosshairModesMenu);
    swivelMode->setActionGroup(rotationModeActionGroup);
    swivelMode->setText(tr("Swivel mode"));
    swivelMode->setCheckable(true);
    swivelMode->setChecked(currentCrosshairRotationMode==3);
    swivelMode->setData( 3 );
    crosshairModesMenu->addAction( swivelMode );

    connect( rotationModeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(OnCrosshairRotationModeSelected(QAction*)) );
  }

  // auto rotation support
  if( m_Renderer.IsNotNull() && m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard3D )
  {
    QAction* autoRotationGroupSeparator = new QAction(crosshairModesMenu);
    autoRotationGroupSeparator->setSeparator(true);
    crosshairModesMenu->addAction( autoRotationGroupSeparator );

    QAction* autoRotationAction = crosshairModesMenu->addAction(tr("Auto Rotation"));
    autoRotationAction->setCheckable(true);
    autoRotationAction->setChecked( m_AutoRotationTimer.isActive() );
    connect( autoRotationAction, SIGNAL(triggered()), this, SLOT(OnAutoRotationActionTriggered()) );
  }

  // Thickslices support
  if (m_Renderer.IsNotNull() && m_Renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
  {
    QAction *thickSlicesGroupSeparator = new QAction(crosshairModesMenu);
    thickSlicesGroupSeparator->setSeparator(true);
    crosshairModesMenu->addAction(thickSlicesGroupSeparator);

    QLabel* tSlabel = new QLabel(tr("Thick slice mode:"), crosshairModesMenu);
    tSlabel->setAlignment(Qt::AlignCenter);

    QWidgetAction* tsla = new QWidgetAction(crosshairModesMenu);
    tsla->setDefaultWidget(tSlabel);

    crosshairModesMenu->addAction(tsla);

    int currentMode = 0;
    {
      mitk::ResliceMethodProperty::Pointer m = dynamic_cast<mitk::ResliceMethodProperty *>(
        m_Renderer->GetCurrentWorldPlaneGeometryNode()->GetProperty("reslice.thickslices"));
      if (m.IsNotNull())
        currentMode = m->GetValueAsId();
    }

    QComboBox* thickSliceComboBox = new QComboBox(crosshairModesMenu);
    thickSliceComboBox->setToolTip(tr("Set up thick slice mode"));

    QString modeNames[] = {
      tr("Maximum"),
      tr("Sum"),
      tr("Weighted"),
      tr("Minimum"),
      tr("Mean")
    };
    for (auto mode : modeNames) {
      thickSliceComboBox->addItem(mode);
    }

    QWidgetAction *m_TSComboBoxAction = new QWidgetAction(crosshairModesMenu);
    m_TSComboBoxAction->setDefaultWidget(thickSliceComboBox);

    crosshairModesMenu->addAction(m_TSComboBoxAction);

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
    {
      thickSliceComboBox->setCurrentIndex(0);
      currentNum = 0;
    }
    else
    {
      thickSliceComboBox->setCurrentIndex(currentMode - 1);
    }

    QSlider *m_TSSlider = new QSlider(crosshairModesMenu);
    m_TSSlider->setMinimum(0);
    m_TSSlider->setMaximum(50);
    m_TSSlider->setValue(currentNum);

    m_TSSlider->setOrientation(Qt::Horizontal);

    connect(m_TSSlider, SIGNAL(valueChanged(int)), this, SLOT(OnTSNumChanged(int)));

    QHBoxLayout *_TSLayout = new QHBoxLayout;
    _TSLayout->setContentsMargins(4, 4, 4, 4);
    _TSLayout->addWidget(new QLabel("TS: "));
    _TSLayout->addWidget(m_TSSlider);
    _TSLayout->addWidget(m_TSLabel = new QLabel(QString::number(currentNum * 2 + 1), this));

    QWidget *_TSWidget = new QWidget;
    _TSWidget->setLayout(_TSLayout);

    QWidgetAction *m_TSSliderAction = new QWidgetAction(crosshairModesMenu);
    m_TSSliderAction->setDefaultWidget(_TSWidget);
    crosshairModesMenu->addAction(m_TSSliderAction);

    connect(thickSliceComboBox
      , static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged)
      , this
      , [this, currentNum] (int index) {
        int num = currentNum;
        mitk::IntProperty::Pointer m = dynamic_cast<mitk::IntProperty *>(
          m_Renderer->GetCurrentWorldPlaneGeometryNode()->GetProperty("reslice.thickslices.num"));
        if (m.IsNotNull())
        {
          num = m->GetValue();
        }
        m_Renderer->GetCurrentWorldPlaneGeometryNode()->SetProperty("reslice.thickslices",
                                                                    mitk::ResliceMethodProperty::New(index + 1));
        OnTSNumChanged(num);
    });
  }
}

void QmitkRenderWindowMenu::NotifyNewWidgetPlanesMode( int mode )
{
  currentCrosshairRotationMode = mode;
}

void QmitkRenderWindowMenu::OnAutoRotationActionTriggered()
{
  if(m_AutoRotationTimer.isActive())
  {
    m_AutoRotationTimer.stop();
    m_Renderer->GetCameraRotationController()->GetSlice()->PingPongOff();
  }
  else
  {
    m_Renderer->GetCameraRotationController()->GetSlice()->PingPongOn();
    m_AutoRotationTimer.start();
  }
}

void QmitkRenderWindowMenu::AutoRotateNextStep()
{
  if(m_Renderer->GetCameraRotationController())
    m_Renderer->GetCameraRotationController()->GetSlice()->Next();
}

void QmitkRenderWindowMenu::SetFullScreenMode(bool state)
{
  m_FullScreenMode = state;

  //Move Widget and show again
  this->MoveWidgetToCorrectPos(1.0f);

  //change icon
  this->ChangeFullScreenIcon();
  DeferredShowMenu();
}

void QmitkRenderWindowMenu::updateWindows()
{
  if (m_MultiWidget && m_MultiWidget->crosshairManager) {
    m_MultiWidget->crosshairManager->updateAllWindows();
  }
}
