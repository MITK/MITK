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

#include <QmitkFunctionality.h>
#include "ui_QmitkDwiSoftwarePhantomViewControls.h"
#include <itkImage.h>

/*!
\brief View for diffusion software phantom generation using binary ROIs.

\sa QmitkFunctionality
\ingroup Functionalities
*/

class QmitkDwiSoftwarePhantomView : public QmitkFunctionality
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDwiSoftwarePhantomView();
  virtual ~QmitkDwiSoftwarePhantomView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
  typedef itk::Image<float, 3>          ItkFloatImgType;
  typedef itk::Vector<double,3>         GradientType;
  typedef std::vector<GradientType>     GradientListType;


  protected slots:

  void GeneratePhantom();                   ///< Start image generation
  void OnSimulateBaselineToggle(int state); ///< change from SNR to noise variance and vice versa

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  /** Generate gradient directions distributed on half sphere (suboptimal distribution but arbitrary number of gradients) **/
  GradientListType GenerateHalfShell(int NPoints);

  /** Generate gradient directions (n-fold icosaedron tesselation) **/
  template<int ndirs> std::vector<itk::Vector<double,3> > MakeGradientList();

  /** Update button activity etc. depending on current datamanager selection **/
  void UpdateGui();

  Ui::QmitkDwiSoftwarePhantomViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

  std::vector< mitk::DataNode::Pointer >    m_SignalRegionNodes;    ///< contains binary signal region nodes
  std::vector< ItkUcharImgType::Pointer >   m_SignalRegions;        ///< contains binary signal region images

  /** List of gui elements generated dynamically depending on the number of selected signal regions **/
  std::vector< QLabel* >            m_Labels;
  std::vector< QDoubleSpinBox* >    m_SpinFa;
  std::vector< QDoubleSpinBox* >    m_SpinAdc;
  std::vector< QDoubleSpinBox* >    m_SpinX;
  std::vector< QDoubleSpinBox* >    m_SpinY;
  std::vector< QDoubleSpinBox* >    m_SpinZ;
  std::vector< QDoubleSpinBox* >    m_SpinWeight;

 };



