/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#if !defined(QmitkPreprocessingResamplingView_H__INCLUDED)
#define QmitkPreprocessingResamplingView_H__INCLUDED

#include <QmitkAbstractView.h>
#include <org_mitk_gui_qt_preprocessing_resampling_Export.h>
#include "ui_QmitkPreprocessingResamplingViewControls.h"

#include "QmitkStepperAdapter.h"

#include <mitkDataStorageSelection.h>

/*!
\brief This module allows to use some basic image processing filters for preprocessing, image enhancement and testing purposes

Several basic ITK image processing filters, like denoising, morphological and edge detection
are encapsulated in this module and can be selected via a list and an intuitive parameter input.
The selected filter will be applied on the image, and a new image showing the output is displayed
as result.
Also, some image arithmetic operations are available.

Images can be 3D or 4D.
In the 4D case, the filters work on the 3D image selected via the
time slider. The result is also a 3D image.

\sa QmitkFunctionality, QObject

\class QmitkBasicImageProcessing
\author Tobias Schwarz
\version 1.0 (3M3)
\date 2009-05-10
\ingroup Bundles
*/

class PREPROCESSING_RESAMPLING_EXPORT QmitkPreprocessingResampling : public QmitkAbstractView
{
  Q_OBJECT

public:

  /*!
  \brief default constructor
  */
  QmitkPreprocessingResampling();

  /*!
  \brief default destructor
  */
  ~QmitkPreprocessingResampling() override;

  /*!
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.
  */
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  /*!
  \brief Invoked when the DataManager selection changed
  */
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>& nodes) override;


  protected slots:

    /*
    * The "Execute" button in the "one image ops" box was triggered
    */
    void StartButtonClicked();

    void StartMultipleImagesButtonClicked();

    void SelectInterpolator(int interpolator);

private:

  /*
  * After a one image operation, reset the "one image ops" panel
  */
  void ResetOneImageOpPanel();

  /*
  * Helper method to reset the parameter set panel
  */
  void ResetParameterPanel();

  /*
  * After a two image operation, reset the "two image ops" panel
  */
  void ResetTwoImageOpPanel();

  /** retrieve the tnc from renderwindow part */
  void InternalGetTimeNavigationController();

  /*!
  * controls containing sliders for scrolling through the slices
  */
  Ui::QmitkPreprocessingResamplingViewControls *m_Controls;

  //mitk::DataNode*       m_SelectedImageNode;
  mitk::DataStorageSelection::Pointer m_SelectedImageNode;
  QmitkStepperAdapter*      m_TimeStepperAdapter;

  std::vector<mitk::DataNode::Pointer> m_SelectedNodes;

  enum InterpolationType{
    LINEAR,
    NEAREST,
    SPLINE
  } m_SelectedInterpolation;
};

#endif // !defined(QmitkBasicImageProcessing_H__INCLUDED)


