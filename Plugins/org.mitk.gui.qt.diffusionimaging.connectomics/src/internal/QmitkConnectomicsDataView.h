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

#ifndef QmitkConnectomicsDataView_h
#define QmitkConnectomicsDataView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkConnectomicsDataViewControls.h"

#include "mitkConnectomicsNetworkCreator.h"
#include "mitkConnectomicsNetworkMapper3D.h"

// ####### ITK includes #######
#include <itkImage.h>

/*!
\brief QmitkConnectomicsDataView

This view provides functionality for the generation of networks. Either from parcellation and fiber bundle or synthetically.
*/
class QmitkConnectomicsDataView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkConnectomicsDataView();
  virtual ~QmitkConnectomicsDataView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  protected slots:

    /// \brief Align two images by copying the geometry
    void OnNetworkifyPushButtonClicked();

    /// \brief Create synthetic networks
    void OnSyntheticNetworkCreationPushButtonClicked();

    /// \brief Adjust parameters depending on synthetic network type
    void OnSyntheticNetworkComboBoxCurrentIndexChanged(int currentIndex);

    /// \brief Create #voxel x #voxel correlation matrix of a timeseries image
    void OnCreateCorrelationMatrixPushButtonClicked();

protected:

  // ####### Functions #######
  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  /// \brief Converts an image into a RGBA image
  template < typename TPixel, unsigned int VImageDimension >
  void TurnIntoRGBA( itk::Image<TPixel, VImageDimension>* inputImage);

  /// \brief Wipe display and empty statistics
  void WipeDisplay();

  template< typename TPixel, unsigned int VImageDimension >
  void DoWholeCorrelation( itk::Image<TPixel, VImageDimension>* itkTimeSeriesImage );

  template< typename TPixel, unsigned int VImageDimension >
  void DoParcelCorrelation( itk::Image<TPixel, VImageDimension>* itkTimeSeriesImage, mitk::Image::Pointer parcelImage );

  // ####### Variables #######


  Ui::QmitkConnectomicsDataViewControls* m_Controls;

  mitk::ConnectomicsNetworkCreator::Pointer m_ConnectomicsNetworkCreator;

  mitk::ConnectomicsNetworkMapper3D::Pointer m_NetworkMapper;

  // Demo/Developer mode toggle
  bool m_demomode;

  // The selected synthetic network type
  int m_currentIndex;
};



#endif // _QMITKBRAINNETWORKANALYSISVIEW_H_INCLUDED

