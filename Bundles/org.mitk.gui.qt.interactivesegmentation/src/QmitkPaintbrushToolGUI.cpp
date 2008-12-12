/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPaintbrushToolGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

QmitkPaintbrushToolGUI::QmitkPaintbrushToolGUI()
:QmitkToolGUI(),
 m_Slider(NULL)
{
  // create the visible widgets
  QBoxLayout* layout = new QHBoxLayout( this );

  QLabel* label = new QLabel( "Size ", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  layout->addWidget(label);

  m_SizeLabel = new QLabel( " 10", this );
  f = m_SizeLabel->font();
  f.setBold(false);
  m_SizeLabel->setFont( f );
  layout->addWidget(m_SizeLabel);

  m_Slider = new QSlider( 1, 50, 1, 10, Qt::Horizontal, this );
  connect( m_Slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
  layout->addWidget( m_Slider );

  /*
  m_Frame = new QFrame( this );
  m_Frame->setMinimumSize( QSize(50, 50) );
  m_Frame->setFrameStyle( QFrame::Box || QFrame::Plain );
  m_Frame->show();
  layout->addWidget( m_Frame );
  */

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkPaintbrushToolGUI::~QmitkPaintbrushToolGUI()
{
  // !!!
  if (m_PaintbrushTool.IsNotNull())
  {
    m_PaintbrushTool->SizeChanged -= mitk::MessageDelegate1<QmitkPaintbrushToolGUI, int>( this, &QmitkPaintbrushToolGUI::OnSizeChanged );
  }

}

void QmitkPaintbrushToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_PaintbrushTool.IsNotNull())
  {
    m_PaintbrushTool->SizeChanged -= mitk::MessageDelegate1<QmitkPaintbrushToolGUI, int>( this, &QmitkPaintbrushToolGUI::OnSizeChanged );
  }

  m_PaintbrushTool = dynamic_cast<mitk::PaintbrushTool*>( tool );

  if (m_PaintbrushTool.IsNotNull())
  {
    m_PaintbrushTool->SizeChanged += mitk::MessageDelegate1<QmitkPaintbrushToolGUI, int>( this, &QmitkPaintbrushToolGUI::OnSizeChanged );
  }
}

void QmitkPaintbrushToolGUI::OnSliderValueChanged(int value)
{
  if (m_PaintbrushTool.IsNotNull())
  {
    m_PaintbrushTool->SetSize( value );
  }

  VisualizePaintbrushSize(value);
}

void QmitkPaintbrushToolGUI::VisualizePaintbrushSize(int size)
{
  m_SizeLabel->setText(QString("%1 ").arg(size));

  /* nice, but useless. scale does not correspond to the image in a render window, so this is more or less useless */
  /*
  QPainter p( m_Frame );

  p.eraseRect( m_Frame->rect() );

  int width = size;
  int height = size;
  int x = m_Frame->width() / 2 - width / 2;
  int y = m_Frame->height() / 2 - height / 2;
  p.drawEllipse( x, y, width, height );
  */
}

void QmitkPaintbrushToolGUI::OnSizeChanged(int current)
{
  m_Slider->setValue(current);
}

