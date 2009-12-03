/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QmitkFiducialRegistrationWidget_H_INCLUDED
#define _QmitkFiducialRegistrationWidget_H_INCLUDED

#include "ui_QmitkFiducialRegistrationWidget.h"
#include "QmitkStdMultiWidget.h"
#include "MitkIGTUIExports.h"
/*!
 * \brief IGT Fiducial Registration Widget
 *
 * Widget used to set fiducial landmarks in the image and to confirm the corresponding landmarks on the world object (patient/phantom).
 * 
 * SetImageFiducialsNode(), SetTrackerFiducialsNode() and SetMultiWidget() must be called, otherwise QmitkPointListWidget can not work. 
 * 
 * 
 *
 * \sa IGT
*/
class MitkIGTUI_EXPORT QmitkFiducialRegistrationWidget : public QWidget
{  
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public: 

  QmitkFiducialRegistrationWidget(QWidget* parent);
  virtual ~QmitkFiducialRegistrationWidget();

  void SetMultiWidget(QmitkStdMultiWidget* multiWidget); ///< Set the default stdMultiWidget (needed for the PointListwidget)

  void SetImageFiducialsNode(mitk::DataTreeNode::Pointer imageFiducialsNode); ///< specify data tree node for the image fiducials
  void SetTrackerFiducialsNode(mitk::DataTreeNode::Pointer trackerFiducialsNode); ///< specify data tree node for the tracker fiducials
  itkGetMacro(ImageFiducialsNode, mitk::DataTreeNode::Pointer); ///< returns data tree node for the image fiducials
  itkGetMacro(TrackerFiducialsNode, mitk::DataTreeNode::Pointer); ///< returns data tree node for the tracker fiducials

  void SetQualityDisplayText(QString text); ///< sets specific text on the UI (useful to display FRE/TRE...)
  bool UseICPIsChecked(); ///< returns true if automatic correspondences search is activated else false

  signals:
    void AddedTrackingFiducial();      ///< signal if a world instrument position was added to a tracking space fiducial
    void PerformFiducialRegistration();        ///< signal if all fiducial were added and registration can be performed
    void FindFiducialCorrespondences(bool on); ///< signal if automatic correspondences search is toggled

protected:

  void CreateQtPartControl(QWidget *parent);

  void CreateConnections();
  
  Ui::QmitkFiducialRegistrationWidget* m_Controls;  ///< gui widget
  QmitkStdMultiWidget* m_MultiWidget;
  mitk::DataTreeNode::Pointer m_ImageFiducialsNode;
  mitk::DataTreeNode::Pointer m_TrackerFiducialsNode;
 
};
#endif // _QmitkFiducialRegistrationWidget_H_INCLUDED
