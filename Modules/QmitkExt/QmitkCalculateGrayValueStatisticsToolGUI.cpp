/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkCopyToClipBoardDialog.h"

MITK_TOOL_GUI_MACRO(QmitkExt_EXPORT, QmitkCalculateGrayValueStatisticsToolGUI, "")

QmitkCalculateGrayValueStatisticsToolGUI::QmitkCalculateGrayValueStatisticsToolGUI()
:QmitkToolGUI()
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
    std::string report = m_CalculateGrayValueStatisticsTool->GetReport();

    // one for linux users
    std::cout << report << std::endl;

    // one for window users
    QmitkCopyToClipBoardDialog* dialog = new QmitkCopyToClipBoardDialog( report.c_str(), NULL);
    dialog->show();
  }
}

