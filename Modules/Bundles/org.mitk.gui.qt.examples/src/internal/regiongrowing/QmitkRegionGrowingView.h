/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkRegionGrowingView_h
#define QmitkRegionGrowingView_h

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "mitkPointSet.h"

#include <itkImage.h>

#include "ui_QmitkRegionGrowingViewControls.h"

#include "mitkPointSetInteractor.h"
#include "mitkPointSet.h"
#include "mitkInteractionConst.h"



/*!
  \brief QmitkRegionGrowingView 

  Functionality for demonstration of MITK basics.

  This functionality allows the user to set some seed points that are used for a simple region growing algorithm from ITK.

  \warning  This is only for demonstration, it is NOT meant to be useful!

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkRegionGrowingView : public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    static const std::string VIEW_ID;

    QmitkRegionGrowingView();
    QmitkRegionGrowingView(const QmitkRegionGrowingView& other);
    virtual ~QmitkRegionGrowingView();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();
    virtual void Deactivated();

  protected slots:
  
    /// \brief Called when the user clicks the GUI button
    void DoImageProcessing();

    void OnSeedPointAdded();

    void ActivatePointSetInteractor();

  protected:

    /*!
      \brief ITK image processing function
      This function is templated like an ITK image. The MITK-Macro AccessByItk determines the actual pixel type and dimensionality of
      a given MITK image and calls this function for further processing (in our case region growing)
     */
    template < typename TPixel, unsigned int VImageDimension >
    void ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry, mitk::DataNode* parent, int thresholdOffset, unsigned int t );

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );

    void OnNodeDeleted();
    void OnNodeModified();

    void OnPointAdded();
    void OnPointMoved();

    /// \brief This is the actual seed point data object
    mitk::PointSet::Pointer m_PointSet;

    Ui::QmitkRegionGrowingViewControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    mitk::ScalarType m_InitialThreshold;

    mitk::PointSetInteractor::Pointer m_PointSetInteractor;
    mitk::DataNode::Pointer m_PointSetNode;

    long m_NodeDeleteObserverTag;
    long m_NodeModifyObserverTag;
    long m_PointSetAddObserverTag;
    long m_PointSetMovedObserverTag;
};



#endif // _QMITKREGIONGROWINGVIEW_H_INCLUDED

