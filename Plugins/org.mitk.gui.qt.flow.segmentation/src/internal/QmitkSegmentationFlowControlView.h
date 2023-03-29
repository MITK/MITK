/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkSegmentationFlowControlView_h
#define QmitkSegmentationFlowControlView_h

#include <berryISelectionListener.h>
#include <berryIWorkbenchListener.h>
#include <QmitkAbstractView.h>

#include "mitkNodePredicateBase.h"

namespace Ui
{
  class SegmentationFlowControlView;
}

/*!
  \brief QmitkSegmentationFlowControlView

  Class that "controls" the segmentation view. It offers the possibility to accept a segmentation.
  Accepting the segmentation stores the segmentation to the given working directory.

  The working directory is specified by command line arguments. If no commandline flag is set the current working directory will be used.
*/
class QmitkSegmentationFlowControlView : public QmitkAbstractView, public berry::IWorkbenchListener
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    /**
     * Creates smartpointer typedefs
     */
    berryObjectMacro(QmitkSegmentationFlowControlView)

    QmitkSegmentationFlowControlView();
    ~QmitkSegmentationFlowControlView() override;

    void CreateQtPartControl(QWidget *parent) override;

protected slots:

    void OnAcceptButtonClicked();


protected:
    void SetFocus() override;

    void NodeAdded(const mitk::DataNode* node) override;
    void NodeChanged(const mitk::DataNode* node) override;
    void NodeRemoved(const mitk::DataNode* node) override;

    bool PreShutdown(berry::IWorkbench*, bool) override;

    void UpdateControls();

    Ui::SegmentationFlowControlView* m_Controls;

private:
    mitk::NodePredicateBase::Pointer m_SegmentationPredicate;
    mitk::NodePredicateBase::Pointer m_SegmentationTaskListPredicate;
    QString m_OutputDir;
    QString m_FileExtension;
};

#endif
