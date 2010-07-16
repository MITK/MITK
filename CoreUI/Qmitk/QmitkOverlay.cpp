/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-14 14:20:26 +0100 (Thu, 14 Jan 2010) $
Version:   $Revision: 21047 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkOverlay.h"

#include <QBoxLayout>


QmitkOverlay::QmitkOverlay( const char* id ): 
QObject(), m_Id(id), m_Position(top_Left), m_Layer(-1)
{
  m_Widget = new QWidget();
}

QmitkOverlay::~QmitkOverlay()
{

}

QmitkOverlay::DisplayPosition QmitkOverlay::GetPosition()
{
  return m_Position;
}

void QmitkOverlay::SetPosition( DisplayPosition pos )
{
  m_Position = pos;
}

unsigned int QmitkOverlay::GetLayer()
{
  return m_Layer;
}

void QmitkOverlay::SetLayer( unsigned int layer )
{
  m_Layer = layer;
}

QWidget* QmitkOverlay::GetWidget()
{
  return m_Widget;
}