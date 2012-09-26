/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkFunctionality.h>
#include "ui_QmitkOdfMaximaExtractionViewControls.h"
#include <itkVectorImage.h>
#include <itkVectorContainer.h>
#include <itkOrientationDistributionFunction.h>

/*!
\brief QmitkOdfMaximaExtractionView

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

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

  typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
  typedef itk::Image<short, 3>          ItkShortImgType;
  typedef itk::Image<float, 3>          ItkFloatImgType;
  typedef itk::Image<itk::Vector<float, QBALL_ODFSIZE>, 3>  ItkQBallImgType;
  typedef itk::Image< itk::DiffusionTensor3D< float >, 3 >  ItkTensorImage;

  typedef itk::Image<itk::Vector<float,3>, 3>     OutputImageType;
  typedef std::vector< OutputImageType::Pointer > OutputImageContainerType;

  typedef vnl_vector_fixed< double, 3 >  DirectionType;
  typedef itk::VectorContainer< int, DirectionType > DirectionContainerType;
  typedef itk::VectorContainer< int, DirectionContainerType::Pointer > ContainerType;
  typedef itk::Image< itk::Vector< float, QBALL_ODFSIZE >, 3 > ItkQballImageType;

  typedef itk::Image< itk::Vector< float, 3>, 3 > ItkDirectionImage3DType;
  typedef itk::VectorContainer< int, ItkDirectionImage3DType::Pointer > ItkDirectionImageContainerType;

  protected slots:

  void ConvertShCoeffsFromFsl();
  void ConvertPeaksFromFsl();
  void GenerateImage();
  void StartFiniteDiff();
  void StartTensor();

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkOdfMaximaExtractionViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  std::vector< mitk::DataNode::Pointer > m_BinaryImageNodes;
  std::vector< mitk::DataNode::Pointer > m_ImageNodes;
  std::vector< mitk::DataNode::Pointer > m_TensorImageNodes;

  void UpdateGui();
  void GenerateDataFromDwi();
  template<int shOrder> void TemplatedConvertShCoeffsFromFsl(mitk::Image* mitkImg);
  template<int shOrder> void StartMaximaExtraction();

private:

 };



