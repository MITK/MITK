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

#ifndef QmitkImageStatisticsView_H__INCLUDED
#define QmitkImageStatisticsView_H__INCLUDED

#include "ui_QmitkImageStatisticsViewControls.h"

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

#include "QmitkStepperAdapter.h"

#include "mitkImageStatisticsCalculator.h"

#include <itkTimeStamp.h>
#include "mitkPlanarLine.h"
#include "QmitkImageStatisticsCalculationThread.h"

#include <QMutex>

/*!
\brief QmitkImageStatisticsView

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkImageStatisticsView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  Q_OBJECT

public:

  /*!
  \  Convenient typedefs
  */
  typedef mitk::DataStorage::SetOfObjects  ConstVector;
  typedef ConstVector::ConstPointer        ConstVectorPointer;
  typedef ConstVector::ConstIterator       ConstVectorIterator;
  typedef std::map< mitk::Image *, mitk::ImageStatisticsCalculator::Pointer > ImageStatisticsMapType;
  typedef std::vector<mitk::DataNode*> SelectedDataNodeVectorType;
  typedef itk::SimpleMemberCommand< QmitkImageStatisticsView > ITKCommandType;

  /*!
  \brief default constructor
  */
  QmitkImageStatisticsView(QObject *parent=0, const char *name=0);

  /*!
  \brief default destructor
  */
  virtual ~QmitkImageStatisticsView();

  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.
  */
  virtual void CreateQtPartControl(QWidget *parent);

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  bool IsExclusiveFunctionality() const;
  /*!
  \brief */
  //void SetStatisticsUpdatePendingFlag( bool flag);
  //virtual bool event( QEvent *event );

  void OnSelectionChanged( berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes );

  static const std::string VIEW_ID;

public slots: 
    void OnThreadedStatisticsCalculationEnds(bool , bool );

protected slots:
  void OnClipboardHistogramButtonClicked();

  void OnClipboardStatisticsButtonClicked();

  void OnIgnoreZerosCheckboxClicked(  );

  void RequestStatisticsUpdate();

signals:
  void StatisticsUpdate();

protected:

  void FillStatisticsTableView( const mitk::ImageStatisticsCalculator::Statistics &s,
    const mitk::Image *image );

  void InvalidateStatisticsTableView();

  /** \brief Issues a request to update statistics by sending an event to the
  * Qt event processing queue.
  *
  * Statistics update should only be executed after program execution returns
  * to the Qt main loop. This mechanism also prevents multiple execution of
  * updates where only one is required.*/
  //void RequestStatisticsUpdate();

  /** \brief Recalculate statistics for currently selected image and mask and
   * update the GUI. */
  void UpdateStatistics();

  /** \brief Listener for progress events to update progress bar. */
  void UpdateProgressBar();

  /** \brief Removes any cached images which are no longer referenced elsewhere. */
  void RemoveOrphanImages();

  /** \brief Computes an Intensity Profile along line and updates the histogram widget with it. */
  void ComputeIntensityProfile( mitk::PlanarLine* line );

  void Activated();
  void Deactivated();

  void Visible();
  void Hidden();

  void SetFocus();
  /** \brief Method called when itkModifiedEvent is called by selected data. */
  void SelectedDataModified();
  /** \brief  */
  void SelectionChanged(const QList<mitk::DataNode::Pointer> &selectedNodes);
  /** \brief  */
  void ReinitData();
  /** \brief  */
  void WriteStatisticsToGUI();
  /** \brief Classifies the selected data node for image, segmentation and planar figure. */
  //void ClassifySelectedNodes();
  /*!
  * controls containing sliders for scrolling through the slices
  */
  Ui::QmitkImageStatisticsViewControls *m_Controls;

  QmitkImageStatisticsCalculationThread* m_CalculationThread;

  QmitkStepperAdapter*      m_TimeStepperAdapter;
  unsigned int              m_CurrentTime;

  QString                   m_Clipboard;

  // Image and mask data
  //mitk::DataNode::Pointer m_SelectedImageNode;
  mitk::Image* m_SelectedImage;

  //mitk::DataNode::Pointer m_SelectedMaskNode;
  mitk::Image* m_SelectedImageMask;
  mitk::PlanarFigure* m_SelectedPlanarFigure;

  long m_ImageObserverTag;
  long m_ImageMaskObserverTag;
  long m_PlanarFigureObserverTag;

  // Hash map for associating one image statistics calculator with each image
  // (so that previously calculated histograms / statistics can be recovered
  // if a recalculation is not required)
  ImageStatisticsMapType m_ImageStatisticsMap;
  SelectedDataNodeVectorType m_SelectedDataNodes;
  mitk::ImageStatisticsCalculator::Pointer m_CurrentStatisticsCalculator;
  
  bool m_CurrentStatisticsValid;

  bool m_StatisticsUpdatePending;
  bool m_StatisticsIntegrationPending;
  bool m_DataNodeSelectionChanged;
  bool m_Visible;
  QMutex* m_QThreadMutex;
};

#endif // QmitkImageStatisticsView_H__INCLUDED
