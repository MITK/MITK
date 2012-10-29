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

#if !defined(QmitkPartialVolumeAnalysisView_H__INCLUDED)
#define QmitkPartialVolumeAnalysisView_H__INCLUDED

//#include "QmitkFunctionality.h"
#include "ui_QmitkPartialVolumeAnalysisViewControls.h"

#include <QmitkAbstractView.h>
#include <berryIWorkbenchPartReference.h>
#include <mitkIZombieViewPart.h>

// berry
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

// itk
#include <itkTimeStamp.h>
#include <itkDiffusionTensorPrincipalDirectionImageFilter.h>
#include <itkExtractChannelFromRgbaImageFilter.h>

// qmitk
#include "QmitkStepperAdapter.h"
#include "QmitkRenderWindow.h"

// mitk
#include "mitkPartialVolumeAnalysisHistogramCalculator.h"
#include "mitkPlanarLine.h"
#include <mitkWeakPointer.h>
#include "mitkDataStorageSelection.h"
#include <mitkVtkLayerController.h>

// vtk
#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vtkCornerAnnotation.h>
//#include "itkProcessObject.h"

/*!
\brief QmitkPartialVolumeAnalysis

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkPartialVolumeAnalysisView : public QmitkAbstractView, public mitk::IZombieViewPart//, public itk::ProcessObject
{
  Q_OBJECT

public:

  /*!
  \  Convenient typedefs
  */
  typedef mitk::DataStorage::SetOfObjects                ConstVector;
  typedef ConstVector::ConstPointer                      ConstVectorPointer;
  typedef ConstVector::ConstIterator                     ConstVectorIterator;
  typedef mitk::PartialVolumeAnalysisHistogramCalculator HistogramCalculatorType;
  typedef HistogramCalculatorType::HistogramType         HistogramType;
  typedef mitk::PartialVolumeAnalysisClusteringCalculator ClusteringType;
  typedef itk::DiffusionTensorPrincipalDirectionImageFilter<float,float> DirectionsFilterType;

  /*!
  \brief default constructor
  */
  QmitkPartialVolumeAnalysisView(QObject *parent=0, const char *name=0);

  /*!
  \brief default destructor
  */
  virtual ~QmitkPartialVolumeAnalysisView();

  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.
  */
  virtual void CreateQtPartControl(QWidget *parent);

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  bool IsExclusiveFunctionality() const;

  virtual bool event( QEvent *event );

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer> &nodes);

  virtual void Activated();

  virtual void Deactivated();

  virtual void ActivatedZombieView(berry::IWorkbenchPartReference::Pointer reference);

  virtual void Hidden();

  virtual void Visible();

  virtual void SetFocus();

  bool AssertDrawingIsPossible(bool checked);

  virtual void NodeChanged(const mitk::DataNode* node);
  virtual void PropertyChanged(const mitk::DataNode* node, const mitk::BaseProperty* prop);
  virtual void NodeRemoved(const mitk::DataNode* node);
  virtual void NodeAddedInDataStorage(const mitk::DataNode* node);

  virtual void AddFigureToDataStorage(mitk::PlanarFigure* figure, const QString& name,
    const char *propertyKey = NULL, mitk::BaseProperty *property = NULL );

  void PlanarFigureInitialized();

  void PlanarFigureFocus(mitk::DataNode* node);

  void ShowClusteringResults();

  static const std::string VIEW_ID;

protected slots:

  void EstimateCircle();
  void SetHistogramVisibility();
  void SetAdvancedVisibility();

  void NumberBinsChangedSlider(int v );
  void UpsamplingChangedSlider( int v );
  void GaussianSigmaChangedSlider( int v );
  void SimilarAnglesChangedSlider(int v );

  void OpacityChangedSlider(int v );

  void NumberBinsReleasedSlider( );
  void UpsamplingReleasedSlider(  );
  void GaussianSigmaReleasedSlider(  );
  void SimilarAnglesReleasedSlider( );

  void ActionDrawEllipseTriggered();
  void ActionDrawRectangleTriggered();
  void ActionDrawPolygonTriggered();

  void ToClipBoard();

  void GreenRadio(bool checked);
  void PartialVolumeRadio(bool checked);
  void BlueRadio(bool checked);
  void AllRadio(bool checked);

  void OnRenderWindowDelete(QObject * obj);

  void TextIntON();
  void ExportClusteringResults();

protected:

  //void StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget );

  /** \brief Issues a request to update statistics by sending an event to the
  * Qt event processing queue.
  *
  * Statistics update should only be executed after program execution returns
  * to the Qt main loop. This mechanism also prevents multiple execution of
  * updates where only one is required.*/
  void RequestStatisticsUpdate();

  /** \brief Recalculate statistics for currently selected image and mask and
   * update the GUI. */
  void UpdateStatistics();

  /** \brief Listener for progress events to update progress bar. */
  void UpdateProgressBar();

  /** \brief Removes any cached images which are no longer referenced elsewhere. */
  void RemoveOrphanImages();

  void Select( mitk::DataNode::Pointer node, bool clearMaskOnFirstArgNULL=false, bool clearImageOnFirstArgNULL=false );

  void SetMeasurementInfoToRenderWindow(const QString& text);

  void FindRenderWindow(mitk::DataNode* node);

  void ExtractTensorImages(
      mitk::Image::Pointer tensorimage);

  typedef std::map< mitk::Image *, mitk::PartialVolumeAnalysisHistogramCalculator::Pointer >
    PartialVolumeAnalysisMapType;

//  void OnSelectionChanged(const QList<mitk::DataNode::Pointer> &nodes);

  /*!
  * controls containing sliders for scrolling through the slices
  */
  Ui::QmitkPartialVolumeAnalysisViewControls *m_Controls;

  QmitkStepperAdapter*      m_TimeStepperAdapter;
  unsigned int              m_CurrentTime;

  QString                     m_Clipboard;

  // result text rendering
  vtkRenderer * m_MeasurementInfoRenderer;
  vtkCornerAnnotation *m_MeasurementInfoAnnotation;

  // Image and mask data
  mitk::DataStorageSelection::Pointer m_SelectedImageNodes;
  mitk::Image::Pointer m_SelectedImage;

  mitk::DataNode::Pointer m_SelectedMaskNode;
  mitk::Image::Pointer m_SelectedImageMask;

  mitk::DataStorageSelection::Pointer m_SelectedPlanarFigureNodes;
  mitk::PlanarFigure::Pointer m_SelectedPlanarFigure;

  bool m_IsTensorImage;
  mitk::Image::Pointer m_FAImage;
  mitk::Image::Pointer m_CAImage;
  mitk::Image::Pointer m_RDImage;
  mitk::Image::Pointer m_ADImage;
  mitk::Image::Pointer m_MDImage;
//  mitk::Image::Pointer m_DirectionImage;
  mitk::Image::Pointer m_DirectionComp1Image;
  mitk::Image::Pointer m_DirectionComp2Image;
  mitk::Image::Pointer m_AngularErrorImage;

  QmitkRenderWindow* m_SelectedRenderWindow;
  QmitkRenderWindow* m_LastRenderWindow;

  long m_ImageObserverTag;
  long m_ImageMaskObserverTag;
  long m_PlanarFigureObserverTag;

  // Hash map for associating one image statistics calculator with each iamge
  // (so that previously calculated histograms / statistics can be recovered
  // if a recalculation is not required)
  PartialVolumeAnalysisMapType m_PartialVolumeAnalysisMap;

  HistogramCalculatorType::Pointer m_CurrentStatisticsCalculator;

  bool m_CurrentStatisticsValid;

  bool m_StatisticsUpdatePending;

  bool m_GaussianSigmaChangedSliding;
  bool m_NumberBinsSliding;
  bool m_UpsamplingChangedSliding;

  bool m_Visible;

  mitk::DataNode::Pointer m_ClusteringResult;

  int m_EllipseCounter;
  int m_RectangleCounter;
  int m_PolygonCounter;
  unsigned int m_InitializedObserverTag;
  bool m_CurrentFigureNodeInitialized;

  int m_QuantifyClass;

  ClusteringType::HelperStructPerformRGBClusteringRetval* m_CurrentRGBClusteringResults;
  ClusteringType::HelperStructPerformClusteringRetval *m_CurrentPerformClusteringResults;

//  mitk::DataNode::Pointer m_newnode;
//  mitk::DataNode::Pointer m_newnode2;
  QIcon* m_IconTexOFF;
  QIcon* m_IconTexON;
  bool m_TexIsOn;
};


#endif // !defined(QmitkPartialVolumeAnalysis_H__INCLUDED)
