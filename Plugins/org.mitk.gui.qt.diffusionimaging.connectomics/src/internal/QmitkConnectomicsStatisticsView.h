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

#ifndef QmitkConnectomicsStatisticsView_h
#define QmitkConnectomicsStatisticsView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkConnectomicsStatisticsViewControls.h"

#include "mitkConnectomicsHistogramCache.h"

// ####### ITK includes #######
#include <itkImage.h>

/*!
\brief QmitkConnectomicsStatisticsView

This view provides the statistics GUI.
*/
class QmitkConnectomicsStatisticsView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkConnectomicsStatisticsView();
  virtual ~QmitkConnectomicsStatisticsView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void OnNetworkBalloonsNodeLabelsComboBoxCurrentIndexChanged(  );
  void UpdateGui();

protected:

  // ####### Functions #######
  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  /// \brief Wipe display and empty statistics
  void WipeDisplay();


  // ####### Variables #######
  Ui::QmitkConnectomicsStatisticsViewControls* m_Controls;

  /// Cache for histograms
  mitk::ConnectomicsHistogramCache histogramCache;
};



#endif // _QMITKBRAINNETWORKANALYSISVIEW_H_INCLUDED

