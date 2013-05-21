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

#include "QmitkFastMarchingTool3DGUI.h"

#include "QmitkNewSegmentationDialog.h"

#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qpainter.h>

MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkFastMarchingTool3DGUI, "")

QmitkFastMarchingTool3DGUI::QmitkFastMarchingTool3DGUI()
:QmitkToolGUI(),
 m_ThresholdSlider(NULL)
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

  //m_ThresholdSlider = new QSlider( 1, 50, 1, 10, Qt::Horizontal, this );
  m_ThresholdSlider = new QSlider( Qt::Horizontal, this );
  m_ThresholdSlider->setMinimum(0);
  m_ThresholdSlider->setMaximum(10000);
  m_ThresholdSlider->setPageStep(1);
  m_ThresholdSlider->setValue(2000);
  connect( m_ThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderValueChanged(int)));
  layout->addWidget( m_ThresholdSlider );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkFastMarchingTool3DGUI::~QmitkFastMarchingTool3DGUI()
{
  // !!!
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->ThresholdChanged -= mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, int>( this, &QmitkFastMarchingTool3DGUI::OnThresholdChanged );
  }

}

void QmitkFastMarchingTool3DGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->ThresholdChanged -= mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, int>( this, &QmitkFastMarchingTool3DGUI::OnThresholdChanged );
  }

  m_FastMarchingTool = dynamic_cast<mitk::FastMarchingTool3D*>( tool );

  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->ThresholdChanged += mitk::MessageDelegate1<QmitkFastMarchingTool3DGUI, int>( this, &QmitkFastMarchingTool3DGUI::OnThresholdChanged );
  }
}

void QmitkFastMarchingTool3DGUI::OnSliderValueChanged(int value)
{
  if (m_FastMarchingTool.IsNotNull())
  {
    m_FastMarchingTool->SetThreshold( value );
  }

  VisualizePaintbrushSize(value);
}

void QmitkFastMarchingTool3DGUI::VisualizePaintbrushSize(int size)
{
  m_ThresholdLabel->setText(QString("%1 ").arg(float(size)/10.0));

}

void QmitkFastMarchingTool3DGUI::OnThresholdChanged(int current)
{
  m_ThresholdSlider->setValue(current);
}
