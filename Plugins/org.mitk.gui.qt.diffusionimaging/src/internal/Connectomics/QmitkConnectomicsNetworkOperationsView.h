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

#ifndef QmitkConnectomicsNetworkOperationsView_h
#define QmitkConnectomicsNetworkOperationsView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkConnectomicsNetworkOperationsViewControls.h"

#include "mitkConnectomicsNetworkMapper3D.h"

// ####### ITK includes #######
#include <itkImage.h>

/*!
\brief QmitkConnectomicsNetworkOperationsView

This bundle provides GUI for the brain network analysis algorithms.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkConnectomicsNetworkOperationsView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkConnectomicsNetworkOperationsView();
  virtual ~QmitkConnectomicsNetworkOperationsView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void OnConvertToRGBAImagePushButtonClicked();

    /// \brief Create modularization of network
    void OnModularizePushButtonClicked();

    /// \brief Prune network
    void OnPrunePushButtonClicked();

    /// \brief Create 2D Connecivity Matrix image from network
    void OnCreateConnectivityMatrixImagePushButtonClicked();

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


  Ui::QmitkConnectomicsNetworkOperationsViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  mitk::ConnectomicsNetworkMapper3D::Pointer m_NetworkMapper;

  // Demo/Developer mode toggle
  bool m_demomode;

  // The selected synthetic network type
  int m_currentIndex;
};



#endif // _QMITKBRAINNETWORKANALYSISVIEW_H_INCLUDED

