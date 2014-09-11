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

#ifndef _QMITKQmitkDenoisingView_H_INCLUDED
#define _QMITKQmitkDenoisingView_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkDenoisingViewControls.h"

#include <itkVectorImage.h>
#include <itkImage.h>
#include <mitkDiffusionImage.h>
#include <itkNonLocalMeansDenoisingFilter.h>
#include <itkMaskImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkVectorImageToImageFilter.h>
#include <itkComposeImageFilter.h>
#include <QThread>
#include <QTimer>

/**
 * \class QmitkDenoisingView
 * \brief View displaying details to denoise diffusionweighted images.
 *
 * \sa QmitkFunctionality
 * \ingroup Functionalities
 */
class QmitkDenoisingView;

class QmitkDenoisingWorker : public QObject
{
  Q_OBJECT

public:

  QmitkDenoisingWorker(QmitkDenoisingView* view);

public slots:

  void run();

private:

  QmitkDenoisingView* m_View;
};


class QmitkDenoisingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDenoisingView();
  virtual ~QmitkDenoisingView();

  /** Typedefs */
  typedef short                                                                                                         DiffusionPixelType;
  typedef mitk::DiffusionImage< DiffusionPixelType >                                                                    DiffusionImageType;
  typedef mitk::Image                                                                                                   MaskImageType;
  typedef itk::NonLocalMeansDenoisingFilter< DiffusionPixelType >                                                       NonLocalMeansDenoisingFilterType;
  typedef itk::DiscreteGaussianImageFilter < itk::Image< DiffusionPixelType, 3>, itk::Image< DiffusionPixelType, 3> >   GaussianFilterType;
  typedef itk::VectorImageToImageFilter < DiffusionPixelType >                                                          ExtractFilterType;
  typedef itk::ComposeImageFilter < itk::Image<DiffusionPixelType, 3> >                                                 ComposeFilterType;

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();
  /// \brief Creation of the connections of the FilterComboBox
  virtual void Activated();

private slots:

  void StartDenoising();                  //< prepares filter condition and starts thread for denoising
  void SelectFilter(int filter);          //< updates which filter is selected
  void BeforeThread();                    //< starts timer & disables all buttons while denoising
  void AfterThread();                     //< stops timer & creates a new datanode of the denoised image
  void UpdateProgress();                  //< updates the progressbar each timestep

private:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );
  void ResetParameterPanel();

  Ui::QmitkDenoisingViewControls*  m_Controls;
  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_BrainMaskNode;
  QmitkDenoisingWorker m_DenoisingWorker;
  QThread m_DenoisingThread;
  bool m_ThreadIsRunning;
  bool m_CompletedCalculation;
  NonLocalMeansDenoisingFilterType::Pointer m_NonLocalMeansFilter;
  GaussianFilterType::Pointer m_GaussianFilter;
  DiffusionImageType::Pointer m_InputImage;
  MaskImageType::Pointer m_ImageMask;
  QTimer* m_DenoisingTimer;
  unsigned int m_LastProgressCount;
  unsigned int m_MaxProgressCount;

  enum FilterType {
    NOFILTERSELECTED,
    NLM,
    GAUSS
  }m_SelectedFilter;

  friend class QmitkDenoisingWorker;
};



#endif // _QmitkDenoisingView_H_INCLUDED
