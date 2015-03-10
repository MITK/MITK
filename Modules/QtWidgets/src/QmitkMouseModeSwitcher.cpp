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

#include "QmitkMouseModeSwitcher.h"
#include <QActionGroup>

QmitkMouseModeSwitcher::QmitkMouseModeSwitcher( QWidget* parent )
:QToolBar(parent)
,m_ActionGroup(new QActionGroup(this))
,m_MouseModeSwitcher(NULL)
,m_ObserverTag(0)
,m_InObservationReaction(false)
{
  QToolBar::setOrientation( Qt::Vertical );
  QToolBar::setIconSize( QSize(17, 17) );
  m_ActionGroup->setExclusive(true); // only one selectable

  addButton( mitk::MouseModeSwitcher::MousePointer, tr("Pointer"), QIcon(":/Qmitk/mm_pointer.png"), true ); // toggle ON
  addButton( mitk::MouseModeSwitcher::Scroll, tr("Scroll"), QIcon(":/Qmitk/mm_scroll.png") );
  addButton( mitk::MouseModeSwitcher::LevelWindow, tr("Level/Window"), QIcon(":/Qmitk/mm_contrast.png") );
  addButton( mitk::MouseModeSwitcher::Zoom, tr("Zoom"), QIcon(":/Qmitk/mm_zoom.png") );
  addButton( mitk::MouseModeSwitcher::Pan, tr("Pan"), QIcon(":/Qmitk/mm_pan.png") );
}

void QmitkMouseModeSwitcher::addButton( MouseMode id, const QString& toolName, const QIcon& icon, bool on)
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
  if (m_MouseModeSwitcher)
  {
    m_MouseModeSwitcher->RemoveObserver( m_ObserverTag );
  }
}

void QmitkMouseModeSwitcher::setMouseModeSwitcher( mitk::MouseModeSwitcher* mms )
{
  // goodbye / welcome ceremonies
  if (m_MouseModeSwitcher)
  {
    m_MouseModeSwitcher->RemoveObserver( m_ObserverTag );
  }

  m_MouseModeSwitcher = mms;

  if ( m_MouseModeSwitcher )
  {
    itk::ReceptorMemberCommand<QmitkMouseModeSwitcher>::Pointer command =
      itk::ReceptorMemberCommand<QmitkMouseModeSwitcher>::New();
    command->SetCallbackFunction(this, &QmitkMouseModeSwitcher::OnMouseModeChanged);
    m_ObserverTag = m_MouseModeSwitcher->AddObserver( mitk::MouseModeSwitcher::MouseModeChangedEvent(), command );
  }
}

void QmitkMouseModeSwitcher::modeSelectedByUser()
{
  if (m_InObservationReaction) return; // this was NOT actually by the user but by ourselves

  QAction* action = dynamic_cast<QAction*>(sender());

  if (action)
  {
    MouseMode id = static_cast<MouseMode>( action->data().toInt() );
    //qDebug() << "Mouse mode '" << qPrintable(action->text()) << "' selected, trigger mode id " << id;

    if (m_MouseModeSwitcher)
    {
      m_MouseModeSwitcher->SelectMouseMode( id );
    }
    emit MouseModeSelected( id );

  }
}

void QmitkMouseModeSwitcher::OnMouseModeChanged(const itk::EventObject&)
{
  m_InObservationReaction = true;

  // push button graphically
  assert( m_MouseModeSwitcher );

  MouseMode activeMode = m_MouseModeSwitcher->GetCurrentMouseMode();

  foreach(QAction* action, m_ActionGroup->actions())
  {
    if ( action->data().toInt() == activeMode )
    {
      action->setChecked( true );
    }
  }

  m_InObservationReaction = false;
}

