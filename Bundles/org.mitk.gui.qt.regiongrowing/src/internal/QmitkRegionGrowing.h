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

#if !defined(QmitkRegionGrowing_H__INCLUDED)
#define QmitkRegionGrowing_H__INCLUDED

#include "QmitkFunctionality.h"

#include "mitkPointSet.h"
#include "mitkPointSetInteractor.h"

#include <itkImage.h>

#include "ui_QmitkRegionGrowingControls.h"
#include "../regiongrowingDll.h"

/*!

  \brief QmitkRegionGrowing 

  Functionality for demonstration of MITK basics.

  This functionality allows the user to set some seed points that are used for a simple region growing algorithm from ITK.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class REGIONGROWING_EXPORTS QmitkRegionGrowing : public QObject, public QmitkFunctionality
{  

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT
  
  public:  

  QmitkRegionGrowing();
  virtual ~QmitkRegionGrowing();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget  
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  /// \brief Called when the functionality is deactivated
  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

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

  /// \brief This node is created once and used for storing seed points
  mitk::DataTreeNode::Pointer m_PointSetNode;

  /// \brief This is the actual seed point data object
  mitk::PointSet::Pointer m_PointSet;

  mitk::PointSetInteractor::Pointer m_Interactor;

  Ui::QmitkRegionGrowingControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;
};

#endif // !defined(QmitkRegionGrowing_H__INCLUDED)

