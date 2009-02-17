/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitk.cpp,v $
Language:  C++
Date:      $Date: 2008-08-01 14:25:23 +0200 (Fr, 01 Aug 2008) $
Version:   $Revision: 1.0 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkBinaryThresholdToolGUI_h_Included
#define QmitkBinaryThresholdToolGUI_h_Included

#include "../QmitkToolGUI.h"
#include "mitkBinaryThresholdTool.h"
//! mmueller
#include "mitkQtInteractiveSegmentationDll.h"
//!

class QSlider;

/**
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::BinaryThresholdTool.

  This GUI shows a slider to change the tool's threshold and an OK button to accept a preview for actual thresholding.

  Last contributor: $Author$
*/
class MITK_QT_INTERACTIVESEGMENTATION QmitkBinaryThresholdToolGUI : public QmitkToolGUI
{
  Q_OBJECT

  public:
    //!mm
    //mitkClassMacro(QmitkBinaryThresholdToolGUI, QmitkToolGUI);
    //itkNewMacro(QmitkBinaryThresholdToolGUI);
    //#changed
    cherryObjectMacro(QmitkBinaryThresholdToolGUI);
    //!

    //!mm
    //#add
    QmitkBinaryThresholdToolGUI();
    //!
    virtual ~QmitkBinaryThresholdToolGUI();

    void OnThresholdingIntervalBordersChanged(int lower, int upper);
    void OnThresholdingValueChanged(int current);

  signals:

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnSliderValueChanged(int value);
    void OnAcceptThresholdPreview();

  protected:
    //!mm
    //#del
    //QmitkBinaryThresholdToolGUI();
    //!

    QSlider* m_Slider;

    mitk::BinaryThresholdTool::Pointer m_BinaryThresholdTool;
};

#endif

