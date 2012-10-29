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

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkTensorReconstructionViewControls.h"

#include <mitkDiffusionImage.h>
#include <mitkTensorImage.h>

typedef short DiffusionPixelType;

struct TrSelListener;

/*!
 * \ingroup org_mitk_gui_qt_tensorreconstruction_internal
 *
 * \brief QmitkTensorReconstructionView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkTensorReconstructionView : public QmitkFunctionality
{

  friend struct TrSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkTensorReconstructionView();
  QmitkTensorReconstructionView(const QmitkTensorReconstructionView& other);
  virtual ~QmitkTensorReconstructionView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  static const int nrconvkernels;

protected slots:

  void TensorsToQbi();
  void TensorsToDWI();
  void DoTensorsToDWI(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void Advanced1CheckboxClicked();
  void Reconstruct();
  void ResidualCalculation();
  void ResidualClicked(int slice, int volume);

protected:

  void ItkTensorReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void TeemTensorReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void TensorReconstructionWithCorr(mitk::DataStorage::SetOfObjects::Pointer inImages);

  void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkTensorReconstructionViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  template<int ndirs> std::vector<itk::Vector<double,3> > MakeGradientList();

  template<int L>
  void TemplatedAnalyticalTensorReconstruction(mitk::DiffusionImage<DiffusionPixelType>* vols,
    float lambda, std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes, int normalization);

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  mitk::DataNode::Pointer m_DiffusionImage;
  mitk::DataNode::Pointer m_TensorImage;
  mitk::DataStorage::SetOfObjects::Pointer m_DiffusionImages;
  mitk::DataStorage::SetOfObjects::Pointer m_TensorImages;
};




#endif // _QMITKTENSORRECONSTRUCTIONVIEW_H_INCLUDED

