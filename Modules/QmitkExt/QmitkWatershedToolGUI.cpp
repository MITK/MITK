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

#include "QmitkWatershedToolGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>


MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkWatershedToolGUI, "")

QmitkWatershedToolGUI::QmitkWatershedToolGUI()
:QmitkToolGUI(),
 m_SliderThreshold(NULL),
 m_SliderLevel(NULL)
{
  // create the visible widgets
  QBoxLayout* layout = new QHBoxLayout( this );
  this->setContentsMargins( 0, 0, 0, 0 );

  QLabel* label = new QLabel( "Threshold ", this );
  QFont f = label->font();
  f.setBold(false);
  label->setFont( f );
  layout->addWidget(label);

  m_ThresholdLabel = new QLabel( " 10", this );
  f = m_ThresholdLabel->font();
  f.setBold(false);
  m_ThresholdLabel->setFont( f );
  layout->addWidget(m_ThresholdLabel);

  //m_Slider = new QSlider( 1, 50, 1, 10, Qt::Horizontal, this );
  m_SliderThreshold = new QSlider( Qt::Horizontal, this );
  m_SliderThreshold->setMinimum(1);
  m_SliderThreshold->setMaximum(50);
  m_SliderThreshold->setPageStep(1);
  m_SliderThreshold->setValue(10);
  connect( m_SliderThreshold, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueThresholdChanged(int)));
  layout->addWidget( m_SliderThreshold );

  //m_Slider = new QSlider( 1, 50, 1, 10, Qt::Horizontal, this );
  m_SliderLevel = new QSlider( Qt::Horizontal, this );
  m_SliderLevel->setMinimum(1);
  m_SliderLevel->setMaximum(50);
  m_SliderLevel->setPageStep(1);
  m_SliderLevel->setValue(10);
  connect( m_SliderLevel, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueThresholdChanged(int)));
  layout->addWidget( m_SliderLevel );

  QPushButton* okButton = new QPushButton("Create Segmentation", this);
  connect( okButton, SIGNAL(clicked()), this, SLOT(OnCreateSegmentation()));
  okButton->setFont( f );
  layout->addWidget( okButton );


  /*
  m_Frame = new QFrame( this );
  m_Frame->setMinimumSize( QSize(50, 50) );
  m_Frame->setFrameStyle( QFrame::Box || QFrame::Plain );
  m_Frame->show();
  layout->addWidget( m_Frame );
  */

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkWatershedToolGUI::~QmitkWatershedToolGUI()
{
  // !!!
  if (m_WatershedTool.IsNotNull())
  {
    //m_WatershedTool->SizeChanged -= mitk::MessageDelegate1<QmitkWatershedToolGUI, int>( this, &QmitkWatershedToolGUI::OnSizeChanged );
  }

}

void QmitkWatershedToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_WatershedTool.IsNotNull())
  {
    //m_WatershedTool->SizeChanged -= mitk::MessageDelegate1<QmitkWatershedToolGUI, int>( this, &QmitkWatershedToolGUI::OnSizeChanged );
  }

  m_WatershedTool = dynamic_cast<mitk::WatershedTool*>( tool );

  if (m_WatershedTool.IsNotNull())
  {
//    m_WatershedTool->SizeChanged += mitk::MessageDelegate1<QmitkWatershedToolGUI, int>( this, &QmitkWatershedToolGUI::OnSizeChanged );
  }
}

void QmitkWatershedToolGUI::OnSliderValueThresholdChanged(int value)
{
  if (m_WatershedTool.IsNotNull())
  {
//    m_WatershedTool->SetSize( value );
  }
}

void QmitkWatershedToolGUI::OnSliderValueLevelChanged(int value)
{
  if (m_WatershedTool.IsNotNull())
  {
//    m_WatershedTool->SetSize( value );
  }
}

void QmitkWatershedToolGUI::OnCreateSegmentation()
{}
