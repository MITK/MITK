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

#ifndef QmitkPaintbrushToolGUI_h_Included
#define QmitkPaintbrushToolGUI_h_Included

#include "QmitkToolGUI.h"
#include "mitkPaintbrushTool.h"

//!mm,added
#include "mitkQtInteractiveSegmentationDll.h"
//!

class QSlider;
class QLabel;
class QFrame;

/**
  \brief GUI for mitk::PaintbrushTool.
  \sa mitk::PaintbrushTool

  This GUI shows a slider to change the pen's size.

  Last contributor: $Author$
*/
//!mm
//class QMITK_EXPORT QmitkPaintbrushToolGUI : public QmitkToolGUI
//#changed
class MITK_QT_INTERACTIVESEGMENTATION QmitkPaintbrushToolGUI : public QmitkToolGUI
//!
{
  Q_OBJECT

  public:
    //!mm
    //mitkClassMacro(QmitkPaintbrushToolGUI, QmitkToolGUI);
    //#changed
    cherryObjectMacro(QmitkPaintbrushToolGUI);
    //!

    //!mm
    //#add
    QmitkPaintbrushToolGUI();
    //!

    virtual ~QmitkPaintbrushToolGUI();

    void OnSizeChanged(int current);

  signals:

  public slots:

  protected slots:

    void OnNewToolAssociated(mitk::Tool*);

    void OnSliderValueChanged(int value);

    void VisualizePaintbrushSize(int size);

  protected:

    //!mm
    //#del
    //QmitkPaintbrushToolGUI();
    //!

    QSlider* m_Slider;
    QLabel* m_SizeLabel;
    QFrame* m_Frame;

    mitk::PaintbrushTool::Pointer m_PaintbrushTool;
};

#endif

