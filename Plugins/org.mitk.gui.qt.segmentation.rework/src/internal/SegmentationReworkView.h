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

#ifndef SegmentationReworkView_h
#define SegmentationReworkView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_SegmentationReworkViewControls.h"

#include "SegmentationReworkRest.h"
#include <mitkRESTClient.h>

/**
  \brief SegmentationReworkView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class SegmentationReworkView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  void RESTPutCallback(SegmentationReworkREST::DicomDTO& dto);

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();

  void UploadNewSegmentation();

  Ui::SegmentationReworkViewControls m_Controls;

private:

  void SetSimilarityGraph(std::vector<double> simScoreArray, int sliceMinStart);

  std::unique_ptr<SegmentationReworkREST> m_HttpHandler;
  mitk::RESTClient* m_RestClient;
};

#endif // SegmentationReworkView_h
