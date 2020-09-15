/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKBASICIMAGEPROCESSINGVIEW_H
#define QMITKBASICIMAGEPROCESSINGVIEW_H

#include <ui_QmitkBasicImageProcessingViewControls.h>

#include <org_mitk_gui_qt_basicimageprocessing_Export.h>

#include <mitkNodePredicateAnd.h>
#include <QmitkStepperAdapter.h>
#include <QmitkAbstractView.h>

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

\class QmitkBasicImageProcessing
\author Tobias Schwarz
\version 1.0 (3M3)
\date 2009-05-10
\ingroup Bundles
*/

class BASICIMAGEPROCESSING_EXPORT QmitkBasicImageProcessing : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkBasicImageProcessing();
  ~QmitkBasicImageProcessing() override;

  void SetFocus() override;

private Q_SLOTS:

  void OnCurrentSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes);

    /*
   * When an action is selected in the "one image ops" list box
   */
  void SelectAction(int action);

  /*
   * When an action is selected in the "two image ops" list box
   */
  void SelectAction2(int operation);

  /*
   * The "Execute" button in the "one image ops" box was triggered
   */
  void StartButtonClicked();

  /*
   * The "Execute" button in the "two image ops" box was triggered
   */
  void StartButton2Clicked();

  /*
   *  Switch between the one and the two image operations GUI
   */
  void ChangeGUI();

  void SelectInterpolator(int interpolator);

private:

  void CreateQtPartControl(QWidget *parent) override;
  virtual void CreateConnections();

  /*
  * Helper method to reset the parameter set panel
  */
  void ResetParameterPanel();

  /** retrieve the tnc from renderwindow part */
  void InternalGetTimeNavigationController();

  Ui::QmitkBasicImageProcessingViewControls* m_Controls;

  QmitkStepperAdapter* m_TimeStepperAdapter;

  mitk::NodePredicateAnd::Pointer m_IsImagePredicate;

  enum ActionType
  {
    NOACTIONSELECTED,
    CATEGORY_DENOISING,
    GAUSSIAN,
    MEDIAN,
    TOTALVARIATION,
    CATEGORY_MORPHOLOGICAL,
    DILATION,
    EROSION,
    OPENING,
    CLOSING,
    CATEGORY_EDGE_DETECTION,
    GRADIENT,
    LAPLACIAN,
    SOBEL,
    CATEGORY_MISC,
    THRESHOLD,
    INVERSION,
    DOWNSAMPLING,
    FLIPPING,
    RESAMPLING,
    RESCALE,
    RESCALE2
  } m_SelectedAction;

  enum OperationType
  {
    TWOIMAGESNOACTIONSELECTED,
    CATEGORY_ARITHMETIC,
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    RESAMPLE_TO,
    CATEGORY_BOOLEAN,
    AND,
    OR,
    XOR
  } m_SelectedOperation;

  enum InterpolationType
  {
    LINEAR,
    NEAREST
  } m_SelectedInterpolation;
};

#endif // !defined(QmitkBasicImageProcessing_H__INCLUDED)QMITKBASICIMAGEPROCESSINGVIEW_H
