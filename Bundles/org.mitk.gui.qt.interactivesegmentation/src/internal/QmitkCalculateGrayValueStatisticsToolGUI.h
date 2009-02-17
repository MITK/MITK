/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.0 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkCalculateGrayValueStatisticsToolGUI_h_Included
#define QmitkCalculateGrayValueStatisticsToolGUI_h_Included

#include "../QmitkToolGUI.h"

#include "mitkCalculateGrayValueStatisticsTool.h"
//!mm,added
#include "mitkQtInteractiveSegmentationDll.h"
//!

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::CalculateGrayValueStatisticsTool.

  Shows nothing. Only when the corresponding tool send a message that statistics are ready, this class pops up a window showing the results.

  Last contributor: $Author$
*/
//!mm
//class QMITK_EXPORT QmitkCalculateGrayValueStatisticsToolGUI : public QmitkToolGUI
//#changed
class MITK_QT_INTERACTIVESEGMENTATION QmitkCalculateGrayValueStatisticsToolGUI : public QmitkToolGUI
//!
{
  Q_OBJECT

  public:
    //!mm
    //mitkClassMacro(QmitkCalculateGrayValueStatisticsToolGUI, QmitkToolGUI);
    //itkNewMacro(QmitkCalculateGrayValueStatisticsToolGUI);
    //#changed
    cherryObjectMacro(QmitkCalculateGrayValueStatisticsToolGUI);
    //!

    //!mm
    //#add
    QmitkCalculateGrayValueStatisticsToolGUI();
    //!

    virtual ~QmitkCalculateGrayValueStatisticsToolGUI();

    /// Reacts to signals from mitk::CalculateGrayValueStatisticsTool
    void OnCalculationsDone();

  signals:

  public slots:

  protected slots:

    /// Connected to signal from QmitkToolGUI. We remember the current tool here
    void OnNewToolAssociated(mitk::Tool*);

  protected:

    //!mm
    //#del
    //QmitkCalculateGrayValueStatisticsToolGUI();
    //!

    mitk::CalculateGrayValueStatisticsTool::Pointer m_CalculateGrayValueStatisticsTool;
};

#endif

