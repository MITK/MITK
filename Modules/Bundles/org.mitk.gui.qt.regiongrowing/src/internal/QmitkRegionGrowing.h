/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 13136 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkRegionGrowing_h
#define QmitkRegionGrowing_h

#include "QmitkFunctionality.h"

#include "mitkPointSet.h"
#include "mitkPointSetInteractor.h"
#include "mitkDataTreeNodeSelection.h"

#include <itkImage.h>

#include <berryISelectionListener.h>

#include "ui_QmitkRegionGrowingControls.h"

/*!
  \brief QmitkRegionGrowing 

  Functionality for demonstration of MITK basics.

  This functionality allows the user to set some seed points that are used for a simple region growing algorithm from ITK.

  \warning  This is only for demonstration, it is NOT meant to be useful!

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkRegionGrowing : public QObject, public QmitkFunctionality
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
  public:  

    QmitkRegionGrowing();
    virtual ~QmitkRegionGrowing();

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
    virtual void StdMultiWidgetNotAvailable();

    void SelectionChanged( berry::IWorkbenchPart::Pointer, berry::ISelection::ConstPointer selection );

  protected slots:
  
    /// \brief Called when the user clicks the GUI button
    void DoRegionGrowing();

  protected:

    /*!
      \brief ITK image processing function
      This function is templated like an ITK image. The MITK-Macro AccessByItk determines the actual pixel type and dimensionality of
      a given MITK image and calls this function for further processing (in our case region growing)
     */
    template < typename TPixel, unsigned int VImageDimension >
    void ItkImageProcessing( itk::Image< TPixel, VImageDimension >* itkImage, mitk::Geometry3D* imageGeometry );

    void UpdateFromCurrentDataManagerSelection();

    /// \brief This node is created once and used for storing seed points
    mitk::DataTreeNode::Pointer m_PointSetNode;

    /// \brief This is the actual seed point data object
    mitk::PointSet::Pointer m_PointSet;

    mitk::PointSetInteractor::Pointer m_Interactor;

    Ui::QmitkRegionGrowingControls* m_Controls;

    QmitkStdMultiWidget* m_MultiWidget;

    ///
    /// A selection listener for datatreenode events
    ///
    berry::ISelectionListener::Pointer m_SelectionListener;
  
  private:

    mitk::WeakPointer<mitk::DataTreeNode> m_SelectedNode;

};

#endif // !defined(QmitkRegionGrowing_h)
