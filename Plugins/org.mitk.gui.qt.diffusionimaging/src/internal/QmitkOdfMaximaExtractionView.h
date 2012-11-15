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

#include <QmitkFunctionality.h>
#include "ui_QmitkOdfMaximaExtractionViewControls.h"
#include <itkVectorImage.h>
#include <itkVectorContainer.h>
#include <itkOrientationDistributionFunction.h>

/*!
\brief View providing several methods to extract peaks from the spherical harmonic representation of ODFs or from tensors

\sa QmitkFunctionality
\ingroup Functionalities
*/

// Forward Qt class declarations


class QmitkOdfMaximaExtractionView : public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkOdfMaximaExtractionView();
  virtual ~QmitkOdfMaximaExtractionView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  typedef itk::Image<unsigned char, 3>                                  ItkUcharImgType;
  typedef itk::Image< itk::DiffusionTensor3D< float >, 3 >              ItkTensorImage;
  typedef itk::Image< itk::Vector< float, 3>, 3 >                       ItkDirectionImage3DType;    ///< contains a 3D vector in each voxel

  protected slots:

  void ConvertShCoeffsFromFsl();    ///< convert fsl spherical harmonic coefficients to the according mitk datatype
  void ConvertPeaksFromFsl();       ///< convert fsl peaks to the according mitk datatype
  void ConvertPeaksFromMrtrix();    ///< convert mrtrix peaks to the according mitk datatype
  void GenerateImage();             ///< semicontinuous ODF maxima extraction
  void StartFiniteDiff();           ///< ODF maxima extraction using finite differences on the densely sampled sphere
  void StartTensor();               ///< extract principal eigenvectors from tensor image

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkOdfMaximaExtractionViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

  std::vector< mitk::DataNode::Pointer > m_BinaryImageNodes;    ///< mask images
  std::vector< mitk::DataNode::Pointer > m_ImageNodes;
  std::vector< mitk::DataNode::Pointer > m_TensorImageNodes;

  void UpdateGui();             ///< update button activity etc. dpending on current datamanager selection
  void GenerateDataFromDwi();   ///< semicontinuous ODF maxima extraction
  template<int shOrder> void TemplatedConvertShCoeffsFromFsl(mitk::Image* mitkImg);
  template<int shOrder> void StartMaximaExtraction();   ///< ODF maxima extraction using finite differences on the densely sampled sphere

private:

 };



