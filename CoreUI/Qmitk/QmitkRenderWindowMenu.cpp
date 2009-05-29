/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 20:04:59 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17180 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "QmitkRenderWindowMenu.h"

#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSize>
#include <QPainter>

#include<QGroupBox>
#include<QRadioButton>
#include<QAction>
#include<QLine>

#include"iconClose.xpm"
#include"iconFullScreen.xpm"
#include"iconHoriSplit.xpm"
#include"iconSettings.xpm"
#include"iconVertiSplit.xpm"


QmitkRenderWindowMenu::QmitkRenderWindowMenu(QWidget *parent, Qt::WindowFlags f )
  :QWidget(parent,f),
  m_Settings(NULL)
{
  //Create Menu Widget
  this->CreateMenuWidget();
  this->hide();
  this->setAutoFillBackground( true );
  //this->setAttribute( Qt::WA_NoSystemBackground  );
  //this->setBackgroundRole( QPalette::Dark );
  //this->update();

  //SetOpacity  --  its just posible if the widget is a window. 
  //Windows indicates that the widget is a window, usually with a window system frame and a title bar, 
  //irrespective of whether the widget has a parent or not.
  //this->setWindowFlags( Qt::Window | Qt::FramelessWindowHint);
  //this->setAttribute(Qt::WA_TranslucentBackground);
  //this->setWindowOpacity(0.75);
  //this->setFocusPolicy( Qt::NoFocus );
}

QmitkRenderWindowMenu::~QmitkRenderWindowMenu()
{}

void QmitkRenderWindowMenu::CreateMenuWidget()
{
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setAlignment( Qt::AlignRight );
  layout->setContentsMargins(1,1,1,1);

  QSize size( 13, 13 );

  //HoriSplitButton
  m_HoriSplitButton = new QPushButton();
  m_HoriSplitButton->setMaximumSize(15, 15);
  m_HoriSplitButton->setIconSize(size);
  m_HoriSplitButton->setFlat( true );
  const QIcon iconHoriSplit( iconHoriSplit_xpm );
  m_HoriSplitButton->setIcon(iconHoriSplit);
  layout->addWidget( m_HoriSplitButton );

  //VertiSplitButton
  m_VertiSplitButton = new QPushButton();
  m_VertiSplitButton->setMaximumSize(15, 15);
  m_VertiSplitButton->setIconSize(size);
  m_VertiSplitButton->setFlat( true );
  const QIcon iconVertiSplit( iconVertiSplit_xpm );
  m_VertiSplitButton->setIcon(iconVertiSplit);
  layout->addWidget( m_VertiSplitButton );

  //fullScreenButton
  m_FullScreenButton = new QPushButton();
  m_FullScreenButton->setMaximumSize(15, 15);
  m_FullScreenButton->setIconSize(size);
  m_FullScreenButton->setFlat( true );
  const QIcon iconFullScreen( iconFullScreen_xpm );
  m_FullScreenButton->setIcon(iconFullScreen);
  layout->addWidget( m_FullScreenButton );

  //settingsButton
  m_SettingsButton = new QPushButton();
  m_SettingsButton->setMaximumSize(15, 15);
  m_SettingsButton->setIconSize(size);
  m_SettingsButton->setFlat( true );
  const QIcon iconSettings( iconSettings_xpm );
  m_SettingsButton->setIcon(iconSettings);
  layout->addWidget( m_SettingsButton );

  //closeButton
  m_CloseButton = new QPushButton();
  m_CloseButton->setMaximumSize(15, 15);
  m_CloseButton->setIconSize(size);
  m_CloseButton->setFlat( true );
  const QIcon iconClose( iconClose_xpm );
  m_CloseButton->setIcon(iconClose);
  layout->addWidget( m_CloseButton );

  //Create Connections -- comming Soon!
  connect( m_HoriSplitButton, SIGNAL( clicked(bool) ), this, SLOT(OnHoriSplitButton(bool)) );
  connect( m_VertiSplitButton, SIGNAL( clicked(bool) ), this, SLOT(OnVertiSplitButton(bool)) );
  connect( m_FullScreenButton, SIGNAL( clicked(bool) ), this, SLOT(OnFullScreenButton(bool)) );
  connect( m_SettingsButton, SIGNAL( clicked(bool) ), this, SLOT(OnSettingsButton(bool)) );
  connect( m_CloseButton, SIGNAL( clicked(bool) ), this, SLOT(OnCloseButton(bool)) );

}

void QmitkRenderWindowMenu::CreateMenuBar()
{
  //create Render Window MenuBar.
  m_MenuBar = new QMenuBar(this);

  m_MenuBar->setContentsMargins(0,0,0,0);
  m_MenuBar->setMouseTracking( true );
 
  QAction* horiSplitAction = new QAction(QIcon(iconHoriSplit_xpm), "horiSplit", m_MenuBar);
  QAction* vertiSplitAction = new QAction(QIcon(iconVertiSplit_xpm), "vertiSplit", m_MenuBar);
  QAction* fullScreenAction = new QAction(QIcon(iconFullScreen_xpm), "FullScreen", m_MenuBar);
  QAction* settingsAction = new QAction(QIcon(iconSettings_xpm), "Settings", m_MenuBar);
  QAction* closeAction = new QAction(QIcon(iconClose_xpm), "close", m_MenuBar);
  m_MenuBar->addAction(horiSplitAction);
  m_MenuBar->addAction(vertiSplitAction);
  m_MenuBar->addAction(fullScreenAction);
  m_MenuBar->addAction(settingsAction);
  m_MenuBar->addAction(closeAction);  

  //Create Connections -- comming Soon.
}

void QmitkRenderWindowMenu::CreateSettingsWidget()
{
     m_Settings = new QMenu(this);
    QAction* transversalAction = new QAction( "Transversal", m_Settings );
    transversalAction->setDisabled( true );
    QAction* saggitalAction = new QAction("Saggital", m_Settings);
    saggitalAction->setDisabled( true );
    QAction* coronalAction = new QAction("Coronal", m_Settings);
    coronalAction->setDisabled( true );
    QAction* threeDAction = new QAction("3D", m_Settings);
    threeDAction->setDisabled( true );
    
    QAction* defaultLayoutAction = new QAction( "Default Layout", m_Settings );
    defaultLayoutAction->setDisabled( true );
       
    m_Settings->addAction(transversalAction);
    m_Settings->addAction(saggitalAction);
    m_Settings->addAction(coronalAction);
    m_Settings->addAction(threeDAction);
    m_Settings->addSeparator();
    m_Settings->addAction(defaultLayoutAction);

    m_Settings->setVisible( false );

    //Create Connections -- comming Soon.
}

void QmitkRenderWindowMenu::paintEvent( QPaintEvent* e )
{
  QPainter painter(this);
  QColor semiTransparentColor = Qt::black;
  semiTransparentColor.setAlpha(255);
  painter.fillRect(rect(), semiTransparentColor);
}

/// \brief
void QmitkRenderWindowMenu::OnHoriSplitButton( bool checked )
{
  std::cout << "Split widget horizontal. not implemented yet... " << std::endl;
}

/// \brief
void QmitkRenderWindowMenu::OnVertiSplitButton( bool checked )
{
  std::cout << "Split widget vertical. not implemented yet... " << std::endl;
}

/// \brief
void QmitkRenderWindowMenu::OnFullScreenButton( bool checked )
{
  std::cout << "Show widget in FullScreen. not implemented yet... " << std::endl;
}

/// \brief
void QmitkRenderWindowMenu::OnSettingsButton( bool checked )
{
 if( m_Settings == NULL )
    this->CreateSettingsWidget();

  QPoint point = this->mapToGlobal( m_SettingsButton->geometry().topLeft() );
  m_Settings->setVisible( true );
  m_Settings->exec( point );
}

/// \brief
void QmitkRenderWindowMenu::OnCloseButton( bool checked )
{
  std::cout << "Close widget. not implemented yet... " << std::endl;
}
