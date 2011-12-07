/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2005/06/28 12:37:25 $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "QmitkMouseModeSwitcher.h"

QmitkMouseModeSwitcher::QmitkMouseModeSwitcher( QWidget* parent )
:QToolBar(parent)
,m_ActionGroup(new QActionGroup(this))
,m_MouseModeSwitcher(NULL)
{
  QToolBar::setOrientation( Qt::Vertical );
  QToolBar::setIconSize( QSize(17, 17) );
  m_ActionGroup->setExclusive(true); // only one selectable

  /*
     TODO change int to enum of MouseModeSwitcher
  */
  addButton( 0, tr("Pointer"), QIcon(":/Qmitk/mm_pointer.png"), true ); // toggle ON
  addButton( 1, tr("Scroll"), QIcon(":/Qmitk/mm_scroll.png") );
  addButton( 2, tr("Level/Window"), QIcon(":/Qmitk/mm_contrast.png") );
  addButton( 3, tr("Zoom"), QIcon(":/Qmitk/mm_zoom.png") );
  addButton( 4, tr("Pan"), QIcon(":/Qmitk/mm_pan.png") );
}

void QmitkMouseModeSwitcher::addButton( int id, const QString& toolName, const QIcon& icon, bool on)
{
  QAction* action = new QAction( icon, toolName, this );
  action->setCheckable(true);
  action->setActionGroup(m_ActionGroup);
  action->setChecked(on);
  action->setData(id);
  connect( action, SIGNAL(triggered()), this, SLOT(modeSelectedByUser()) );
  QToolBar::addAction( action );
}


QmitkMouseModeSwitcher::~QmitkMouseModeSwitcher()
{
}

void QmitkMouseModeSwitcher::setMouseModeSwitcher( mitk::MouseModeSwitcher* mms )
{
  /* TODO put any goodbye / welcome ceremonies here, e.g. get current mode */
  m_MouseModeSwitcher = mms;
}

void QmitkMouseModeSwitcher::modeSelectedByUser()
{
  QAction* action = dynamic_cast<QAction*>(sender());

  if (action)
  {
    qDebug() << "Mouse mode '" << qPrintable(action->text()) << "' selected, trigger mode id " << action->data().toInt();
    /* TODO
    if (m_MouseModeSwitcher)
    {
      m_MouseModeSwitcher->ModeSelected( id );
    }
    emit ModeSelected( id );
    */
  }
}

