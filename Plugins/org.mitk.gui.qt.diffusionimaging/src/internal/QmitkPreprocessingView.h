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

#ifndef _QMITKPREPROCESSINGVIEW_H_INCLUDED
#define _QMITKPREPROCESSINGVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkPreprocessingViewControls.h"

#include "mitkDiffusionImage.h"

typedef short DiffusionPixelType;

struct PrpSelListener;

/*!
 * \ingroup org_mitk_gui_qt_preprocessing_internal
 *
 * \brief QmitkPreprocessingView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkPreprocessingView : public QmitkFunctionality
{

  friend struct PrpSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  typedef vnl_vector_fixed< double, 3 > GradientDirectionType;
  typedef itk::VectorContainer< unsigned int, GradientDirectionType > GradientDirectionContainerType;

  QmitkPreprocessingView();
  QmitkPreprocessingView(const QmitkPreprocessingView& other);
  virtual ~QmitkPreprocessingView();

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

  void AverageGradients();
  void ExtractB0();
  void MergeDwis();
  void DoApplyMesurementFrame();
  void DoReduceGradientDirections();
  void DoShowGradientDirections();
  void DoHalfSphereGradientDirections();

protected:
  /** Called by ExtractB0 if check-box activated, extracts all b0 images without averaging */
  void DoExtractBOWithoutAveraging();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkPreprocessingViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  mitk::DiffusionImage<DiffusionPixelType>::Pointer m_DiffusionImage;
  std::vector< mitk::DataNode::Pointer >            m_SelectedDiffusionNodes;

  QList<QCheckBox*> m_ReduceGradientCheckboxes;
  QList<QSpinBox*> m_ReduceGradientSpinboxes;
};




#endif // _QMITKPREPROCESSINGVIEW_H_INCLUDED

