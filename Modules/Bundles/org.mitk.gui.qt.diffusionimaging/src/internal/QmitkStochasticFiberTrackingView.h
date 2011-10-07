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

#ifndef QmitkStochasticFiberTrackingView_h
#define QmitkStochasticFiberTrackingView_h

#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>


#include <QmitkFunctionality.h>
#include <itkStochasticTractographyFilter.h>

#include "ui_QmitkStochasticFiberTrackingViewControls.h"

#include <itkVectorImage.h>
#include <itkImage.h>

#include <mitkDiffusionImage.h>
#include <mitkTensorImage.h>

#include <mitkDataStorage.h>
#include <itkDTITubeSpatialObject.h>
#include <itkDTITubeSpatialObjectPoint.h>
#include <itkSceneSpatialObject.h>

#include <MitkDiffusionImagingExports.h>






//define the input/output types
typedef itk::VectorImage< short int, 3 > DWIVectorImageType;
typedef itk::Image< float, 3 > FloatImageType;
typedef itk::Image< unsigned int, 3 > CImageType;
typedef itk::StochasticTractographyFilter< DWIVectorImageType, FloatImageType, CImageType > PTFilterType;
typedef itk::DTITubeSpatialObject<3>      DTITubeType;
typedef itk::DTITubeSpatialObjectPoint<3>  DTITubePointType;
typedef itk::SceneSpatialObject<3>        SceneSpatialObjectType;

/*!
\brief QmitkFiberTrackingView

\warning  This application module is not yet documented. Use "svn blame/praise/annotate" and ask the author to provide basic documentation.

\sa QmitkFunctionality
\ingroup Functionalities
*/
class /*MitkDiffusionImaging_EXPORT*/ QmitkStochasticFiberTrackingView : public QmitkFunctionality
{


  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkStochasticFiberTrackingView();
  virtual ~QmitkStochasticFiberTrackingView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  protected slots:

    /// \brief Called when the user clicks the GUI button
  void DoFiberTracking();
//  void handleAlgoSelection();



protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

  Ui::QmitkStochasticFiberTrackingViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

private:


//    \brief vector containing related (DiffusionImage) nodes of interest
  std::vector<mitk::DataNode*> vPselDWImg;
//    \brief vector containing SeedRegions(binary) nodes of interest
  std::vector<mitk::DataNode*> vSeedROI;

//   \brief flag to de-/activate manual seedPoint index-coordinates
  bool m_singleSeedpoint;
  bool checkDWIType( mitk::DataNode::Pointer );
  void refreshTableWidget(std::vector<mitk::DataNode*> ,  QString );
  bool checkSeedROI( mitk::DataNode::Pointer  );
  typedef itk::Image< unsigned char, 3 > BinaryImageType;
  // \brief container storing all found tracts
  PTFilterType::TractContainerType::Pointer m_tractcontainer;


};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

