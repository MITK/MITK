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

#include "QmitkCalculateGrayValueStatisticsToolGUI.h"

#include "QmitkHistogramWidget.h"

#include "mitkCalculateGrayValueStatisticsTool.h"

QmitkCalculateGrayValueStatisticsToolGUI::QmitkCalculateGrayValueStatisticsToolGUI() :
  QmitkToolGUI()
{
  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkCalculateGrayValueStatisticsToolGUI::~QmitkCalculateGrayValueStatisticsToolGUI()
{
  if (m_CalculateGrayValueStatisticsTool.IsNotNull())
  {
    m_CalculateGrayValueStatisticsTool->StatisticsCompleted -= mitk::MessageDelegate<QmitkCalculateGrayValueStatisticsToolGUI>( this, &QmitkCalculateGrayValueStatisticsToolGUI::OnCalculationsDone );
  }
}

void QmitkCalculateGrayValueStatisticsToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_CalculateGrayValueStatisticsTool.IsNotNull())
  {
    m_CalculateGrayValueStatisticsTool->StatisticsCompleted -= mitk::MessageDelegate<QmitkCalculateGrayValueStatisticsToolGUI>( this, &QmitkCalculateGrayValueStatisticsToolGUI::OnCalculationsDone );
  }

  m_CalculateGrayValueStatisticsTool = dynamic_cast<mitk::CalculateGrayValueStatisticsTool*>( tool );

  if (m_CalculateGrayValueStatisticsTool.IsNotNull())
  {
    m_CalculateGrayValueStatisticsTool->StatisticsCompleted += mitk::MessageDelegate<QmitkCalculateGrayValueStatisticsToolGUI>( this, &QmitkCalculateGrayValueStatisticsToolGUI::OnCalculationsDone );
  }
}

void QmitkCalculateGrayValueStatisticsToolGUI::OnCalculationsDone()
{
  if (m_CalculateGrayValueStatisticsTool.IsNotNull())
  {
    bool showreport = false;

    //uses the parameter "true" because the report should be shown in addition to the histogram
    QmitkHistogramWidget* hvw = new QmitkHistogramWidget();
    typedef itk::Statistics::Histogram<double, 1> HistogramType;

    HistogramType::ConstPointer histogram = m_CalculateGrayValueStatisticsTool->GetHistogram();
    hvw->SetHistogram(histogram);

    if(showreport)
    {
      std::string report = m_CalculateGrayValueStatisticsTool->GetReport();
      // one for linux users
      std::cout << report << std::endl;
      hvw->SetReport(report);
    }
    hvw->show();
  }
}

