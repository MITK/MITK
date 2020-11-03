/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __Q_MITK_MATCHPOINT_MAPPER_H
#define __Q_MITK_MATCHPOINT_MAPPER_H

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include "mitkNodePredicateBase.h"

#include "ui_QmitkSegmentationFlowControlView.h"

/*!
  \brief QmitkSegmentationFlowControlView

  Class that "controls" the segmentation view. It offers the possibility to accept a segmentation.
  Accepting the segmentation stores the segmentation to the given working directory.

  The working directory is specified by command line arguments. If no commandline flag is set the current working directory will be used.
*/
class QmitkSegmentationFlowControlView : public QmitkAbstractView
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

    void CreateQtPartControl(QWidget *parent) override;

protected slots:

    void OnAcceptButtonPushed();

protected:
    void SetFocus() override;

    void NodeAdded(const mitk::DataNode* node) override;
    void NodeChanged(const mitk::DataNode* node) override;
    void NodeRemoved(const mitk::DataNode* node) override;

    void UpdateControls();

    Ui::SegmentationFlowControlView m_Controls;

private:
    QWidget *m_Parent;
    mitk::NodePredicateBase::Pointer m_SegmentationPredicate;
    QString m_OutputDir;
    QString m_FileExtension;
};

#endif // MatchPoint_h

