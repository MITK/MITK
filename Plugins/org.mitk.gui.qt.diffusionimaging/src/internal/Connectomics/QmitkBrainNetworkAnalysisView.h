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

#ifndef QmitkBrainNetworkAnalysisView_h
#define QmitkBrainNetworkAnalysisView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkBrainNetworkAnalysisViewControls.h"

#include "mitkConnectomicsNetworkCreator.h"
#include "mitkConnectomicsNetworkMapper3D.h"

#include "mitkConnectomicsHistogramCache.h"

// ####### ITK includes #######
#include <itkImage.h>

/*!
\brief QmitkBrainNetworkAnalysisView 

This bundle provides GUI for the brain network analysis algorithms.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkBrainNetworkAnalysisView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:  

  static const std::string VIEW_ID;

  QmitkBrainNetworkAnalysisView();
  virtual ~QmitkBrainNetworkAnalysisView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void OnConvertToRGBAImagePushButtonClicked();

    /// \brief Align two images by copying the geometry
    void OnNetworkifyPushButtonClicked();

    /// \brief Create synthetic networks 
    void OnSyntheticNetworkCreationPushButtonClicked();

    /// \brief Adjust parameters depending on synthetic network type
    void OnSyntheticNetworkComboBoxCurrentIndexChanged(int currentIndex);

    /// \brief Create modularization of network
    void OnModularizePushButtonClicked();

protected:

  // ####### Functions #######
  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  /// \brief Converts an image into a RGBA image
  template < typename TPixel, unsigned int VImageDimension >
  void TurnIntoRGBA( itk::Image<TPixel, VImageDimension>* inputImage);

  /// \brief Wipe display and empty statistics
  void WipeDisplay();


  // ####### Variables #######


  Ui::QmitkBrainNetworkAnalysisViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  mitk::ConnectomicsNetworkCreator::Pointer m_ConnectomicsNetworkCreator;

  mitk::ConnectomicsNetworkMapper3D::Pointer m_NetworkMapper;

  /// Cache for histograms
  mitk::ConnectomicsHistogramCache histogramCache;

  // Demo/Developer mode toggle
  bool m_demomode;

  // The selected synthetic network type
  int m_currentIndex;
};



#endif // _QMITKBRAINNETWORKANALYSISVIEW_H_INCLUDED

