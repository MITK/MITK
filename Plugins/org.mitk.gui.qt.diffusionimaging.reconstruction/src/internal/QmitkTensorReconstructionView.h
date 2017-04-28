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

#ifndef _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED
#define _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>

#include <string>

#include "ui_QmitkTensorReconstructionViewControls.h"

#include <mitkTensorImage.h>
#include <mitkImage.h>
#include <mitkDiffusionPropertyHelper.h>
#include <itkVectorImage.h>

typedef short DiffusionPixelType;

struct TrSelListener;

/*!
 * \ingroup org_mitk_gui_qt_tensorreconstruction_internal
 *
 * \brief QmitkTensorReconstructionView
 *
 * Document your class here.
 */
class QmitkTensorReconstructionView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{

  friend struct TrSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    typedef mitk::DiffusionPropertyHelper::GradientDirectionType            GradientDirectionType;
    typedef mitk::DiffusionPropertyHelper::GradientDirectionsContainerType  GradientDirectionContainerType;
    typedef mitk::DiffusionPropertyHelper::BValueMapType                    BValueMapType;
    typedef itk::VectorImage< DiffusionPixelType, 3 >                       ITKDiffusionImageType;

  static const std::string VIEW_ID;

  QmitkTensorReconstructionView();
  virtual ~QmitkTensorReconstructionView();

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

  void TensorsToQbi();
  void TensorsToDWI();
  void DoTensorsToDWI(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void Advanced1CheckboxClicked();
  void Reconstruct();
  void ResidualCalculation();
  void ResidualClicked(int slice, int volume);
  /**
   * @brief PreviewThreshold Generates a preview of the values that are cut off by the thresholds
   * @param threshold
   */
  void PreviewThreshold(int threshold);

protected:

  void ItkTensorReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void TeemTensorReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void TensorReconstructionWithCorr(mitk::DataStorage::SetOfObjects::Pointer inImages);

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkTensorReconstructionViewControls* m_Controls;

  template<int ndirs> itk::VectorContainer<unsigned int, vnl_vector_fixed<double,3> >::Pointer MakeGradientList();

  template<int L>
  void TemplatedAnalyticalTensorReconstruction(mitk::Image* vols,
    float lambda, std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes, int normalization);

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  mitk::DataNode::Pointer m_DiffusionImage;
  mitk::DataNode::Pointer m_TensorImage;
  mitk::DataStorage::SetOfObjects::Pointer m_DiffusionImages;
  mitk::DataStorage::SetOfObjects::Pointer m_TensorImages;
};




#endif // _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED

