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

#include "SegmentationReworkREST.h"
#include <mitkDICOMWeb.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>

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

  void RESTPutCallback(const SegmentationReworkREST::DicomDTO& dto);
  void RESTGetCallback(const SegmentationReworkREST::DicomDTO& dto);
  void RESTGetCallbackGeneric(const SegmentationReworkREST::DicomDTO& dto);

  void UpdateChartWidget();
  void LoadData(std::vector<std::string> filePathList);

signals:
  void InvokeLoadData(std::vector<std::string> filePathList);

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief Called when the user clicks the GUI button
  void CreateNewSegmentationC();

  void CleanDicomFolder();
  void UploadNewSegmentation();
  void RestartConnection();
  void OnIndividualCheckChange(int state);

  Ui::SegmentationReworkViewControls m_Controls;

private:

  std::string getNextFolderName();
  std::vector<unsigned int> CreateSegmentation(mitk::Image::Pointer baseSegmentation, double threshold);
  void InterpolateSegmentation(mitk::Image::Pointer baseSegmentation, std::vector<unsigned int> sliceIndices);

  void SetSimilarityGraph(std::vector<double> simScoreArray, int sliceMinStart);
  std::unique_ptr<SegmentationReworkREST> m_HttpHandler;
  mitk::DICOMWeb* m_DICOMWeb;

  std::string m_CurrentStudyUID;

  std::string m_DownloadBaseDir;
  std::string m_tempSegDir;

  mitk::DataNode::Pointer m_Image;
  mitk::DataNode::Pointer m_SegA;
  mitk::DataNode::Pointer m_SegB;
  mitk::DataNode::Pointer m_SegC;
  std::map<double, double> m_ScoreMap;
  std::string m_GroundTruth;

  QWidget* m_Parent;
  int counter;
};

#endif // SegmentationReworkView_h
