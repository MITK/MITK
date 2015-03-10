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

#ifndef QmitkCalculateGrayValueStatisticsToolGUI_h_Included
#define QmitkCalculateGrayValueStatisticsToolGUI_h_Included

#include "QmitkToolGUI.h"
#include <MitkSegmentationUIExports.h>

#include "mitkCalculateGrayValueStatisticsTool.h"

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::CalculateGrayValueStatisticsTool.

  Shows nothing. Only when the corresponding tool send a message that statistics are ready, this class pops up a window showing the results.

  Last contributor: $Author$
*/
class MITKSEGMENTATIONUI_EXPORT QmitkCalculateGrayValueStatisticsToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:

    mitkClassMacro(QmitkCalculateGrayValueStatisticsToolGUI, QmitkToolGUI);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /// Reacts to signals from mitk::CalculateGrayValueStatisticsTool
    void OnCalculationsDone();

  signals:

  public slots:

  protected slots:

    /// Connected to signal from QmitkToolGUI. We remember the current tool here
    void OnNewToolAssociated(mitk::Tool*);

  protected:

    QmitkCalculateGrayValueStatisticsToolGUI();
    virtual ~QmitkCalculateGrayValueStatisticsToolGUI();

    mitk::CalculateGrayValueStatisticsTool::Pointer m_CalculateGrayValueStatisticsTool;
};

#endif

