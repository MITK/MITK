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



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkAbstractView.h>
#include "ui_QmitkOdfMaximaExtractionViewControls.h"
#include <itkVectorImage.h>
#include <itkVectorContainer.h>
#include <itkOrientationDistributionFunction.h>
#include <QmitkDataStorageComboBoxWithSelectNone.h>
#include <mitkTensorImage.h>

/*!
\brief View providing several methods to extract peaks from the spherical harmonic representation of ODFs or from tensors
*/

// Forward Qt class declarations


class QmitkOdfMaximaExtractionView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkOdfMaximaExtractionView();
  virtual ~QmitkOdfMaximaExtractionView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  typedef itk::Image<unsigned char, 3>                                  ItkUcharImgType;
  typedef itk::Image< itk::DiffusionTensor3D< float >, 3 >              ItkTensorImage;
  typedef itk::Image< itk::Vector< float, 3>, 3 >                       ItkDirectionImage3DType;    ///< contains a 3D vector in each voxel

  protected slots:

  void ConvertShCoeffs();           ///< convert spherical harmonic coefficients to the according mitk datatype
  void StartPeakExtraction();
  void OnImageSelectionChanged();

protected:

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  void StartMaximaExtraction(mitk::Image* img);         ///< ODF maxima extraction using finite differences on the densely sampled sphere
  void StartTensorPeakExtraction(mitk::TensorImage* img);     ///< extract principal eigenvectors from tensor image

  /// \brief called by QmitkAbstractView when DataManager's selection has changed

  Ui::QmitkOdfMaximaExtractionViewControls* m_Controls;

  template<int shOrder> void TemplatedConvertShCoeffs(mitk::Image* mitkImg);
  template<int shOrder> void StartMaximaExtraction(mitk::Image* image);   ///< ODF maxima extraction using finite differences on the densely sampled sphere

private:

 };



