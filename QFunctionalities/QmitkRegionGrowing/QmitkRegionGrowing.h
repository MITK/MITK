/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

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

class QmitkStdMultiWidget;
class QmitkRegionGrowingControls;

/*!

  \brief QmitkRegionGrowing 

  Functionality for demonstration of MITK basics.

  This functionality allows the user to set some seed points that are used for a simple region growing algorithm from ITK.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkRegionGrowing : public QmitkFunctionality
{  

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT
  
  public:  
  /// \brief Constructor  
  QmitkRegionGrowing(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /// \brief Destructor  
  virtual ~QmitkRegionGrowing();

  /// \brief Creation the widget containing the application   controls, like sliders, buttons etc.  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /// \brief Creation the applications main widget  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /// \brief Creation of the connections of main and control widget  
  virtual void CreateConnections();

  /// \brief Creation of an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  virtual QAction * CreateAction(QActionGroup *parent);

  /// \brief Called when the functionality is activated
  virtual void Activated();

  /// \brief Called when the functionality is deactivated
  virtual void Deactivated();

protected slots:  
  
  /// \brief Called by the application whenever the data tree changes
  void TreeChanged();

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

  /*!  
    Default main widget containing 4 windows showing 3   
    orthogonal slices of the volume and a 3D render window  
   */  
  QmitkStdMultiWidget * m_MultiWidget;

  /*!  
   Control widgets of this functionality
   */  
  QmitkRegionGrowingControls * m_Controls;

  /// \brief This node is created once and used for storing seed points
  mitk::DataTreeNode::Pointer m_PointSetNode;

  /// \brief This is the actual seed point data object
  mitk::PointSet::Pointer m_PointSet;

  mitk::PointSetInteractor::Pointer m_Interactor;

};

#endif // !defined(QmitkRegionGrowing_H__INCLUDED)

