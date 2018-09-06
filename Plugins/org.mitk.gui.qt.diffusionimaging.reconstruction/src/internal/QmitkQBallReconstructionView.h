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

#ifndef _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED
#define _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

#include <string>

#include "ui_QmitkQBallReconstructionViewControls.h"

#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <itkVectorImage.h>
#include <mitkShImage.h>

typedef short DiffusionPixelType;

struct QbrSelListener;

struct QbrShellSelection;

/*!
 * \ingroup org_mitk_gui_qt_qballreconstruction_internal
 *
 * \brief QmitkQBallReconstructionView
 *
 * Document your class here.
 */
class QmitkQBallReconstructionView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  friend struct QbrSelListener;

  friend struct QbrShellSelection;

  typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
  typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;
  typedef mitk::DiffusionPropertyHelper::BValueMapType                    BValueMapType;
  typedef itk::VectorImage< DiffusionPixelType, 3 >                       ITKDiffusionImageType;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkQBallReconstructionView();
  virtual ~QmitkQBallReconstructionView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  /// \brief Called when the view gets activated
  virtual void Activated() override;

  /// \brief Called when the view gets deactivated
  virtual void Deactivated() override;

  /// \brief Called when the view becomes visible
  virtual void Visible() override;

  /// \brief Called when the view becomes hidden
  virtual void Hidden() override;

  static const int nrconvkernels;

protected slots:

  void UpdateGui();
  void ReconstructStandard();
  void ConvertShImage();
  void MethodChoosen(int method);
  void Reconstruct(int method, int normalization);

  void NumericalQBallReconstruction(mitk::DataNode::Pointer node, int normalization);
  void AnalyticalQBallReconstruction(mitk::DataNode::Pointer node, int normalization);
  void MultiQBallReconstruction(mitk::DataNode::Pointer node);


  /**
   * @brief PreviewThreshold Generates a preview of the values that are cut off by the thresholds
   * @param threshold
   */
  void PreviewThreshold(short threshold);

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkQBallReconstructionViewControls* m_Controls;

  template<int L>
  void TemplatedAnalyticalQBallReconstruction(mitk::DataNode* dataNodePointer, double lambda, int normalization);

  template<int L>
  void TemplatedMultiQBallReconstruction(double lambda, mitk::DataNode*);

private:

  std::map< const mitk::DataNode *, QbrShellSelection * > m_ShellSelectorMap;
  void GenerateShellSelectionUI(mitk::DataNode::Pointer node);
};




#endif // _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED

