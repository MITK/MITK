/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef SpectroCamRecorder_h
#define SpectroCamRecorder_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_SpectroCamRecorderControls.h"

#include <mitkSpectroCamController.h>


/**
\brief SpectroCamRecorder

\warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\sa QmitkAbstractView
\ingroup ${plugin_target}_internal
*/
class SpectroCamRecorder : public QmitkAbstractView
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    virtual ~SpectroCamRecorder();

protected slots:

    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();

    void SetWhiteBalance();

    void SaveImage();

protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
        const QList<mitk::DataNode::Pointer>& nodes );

    Ui::SpectroCamRecorderControls m_Controls;

private:

    mitk::SpectroCamController m_Controller;



};

#endif // SpectroCamRecorder_h
