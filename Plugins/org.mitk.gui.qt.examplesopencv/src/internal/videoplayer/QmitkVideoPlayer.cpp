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

#include "QmitkVideoPlayer.h"
#include <QVBoxLayout>
#include <QmitkOpenCVVideoControls.h>
#include <QmitkStdMultiWidget.h>
#include <QmitkVideoBackground.h>

QmitkVideoPlayer::QmitkVideoPlayer()
: m_VideoSource(0)
, m_VideoBackground( new QmitkVideoBackground(m_VideoSource) )
{

}

QmitkVideoPlayer::~QmitkVideoPlayer()
{
  // save video preferences
}

bool QmitkVideoPlayer::IsExclusiveFunctionality() const
{
  return false;
}

void QmitkVideoPlayer::CreateQtPartControl( QWidget* parent )
{
  // retrieve old preferences
  m_VideoSource = mitk::OpenCVVideoSource::New();
  m_VideoBackground = new QmitkVideoBackground(m_VideoSource);
  m_VideoBackground->setParent(parent);

  QVBoxLayout* layout = new QVBoxLayout;
  m_OpenCVVideoControls = new QmitkOpenCVVideoControls(m_VideoBackground,
    this->GetActiveStdMultiWidget());
  layout->addWidget( m_OpenCVVideoControls );

  parent->setLayout( layout );
}

void QmitkVideoPlayer::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  m_OpenCVVideoControls->SetStdMultiWidget( &stdMultiWidget );
}
