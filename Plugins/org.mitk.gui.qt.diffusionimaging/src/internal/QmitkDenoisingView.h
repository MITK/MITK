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

#ifndef _QMITKQmitkDenoisingView_H_INCLUDED
#define _QMITKQmitkDenoisingView_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkFunctionality.h>

#include "ui_QmitkDenoisingViewControls.h"

#include <itkVectorImage.h>
#include <itkImage.h>
#include <mitkDiffusionImage.h>
#include <itkNonLocalMeansDenoisingFilter.h>

//

/*!
  \brief View displaying details of the orientation distribution function in the voxel at the current crosshair position.

  \sa QmitkFunctionality
  \ingroup Functionalities
*/
class QmitkDenoisingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDenoisingView();
  virtual ~QmitkDenoisingView();

  typedef short DiffusionPixelType;
  typedef mitk::DiffusionImage< DiffusionPixelType > DiffusionImageType;
  typedef itk::Image<short, 3> MaskImageType;
  typedef itk::NonLocalMeansDenoisingFilter< DiffusionPixelType > NonLocalMeansDenoisingFilterType;
  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();
  virtual void Activated();

protected slots:

  void StartDenoising();
  void SelectFilter(int filter);

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );



  Ui::QmitkDenoisingViewControls*  m_Controls;


  mitk::DataNode::Pointer m_ImageNode;
  mitk::DataNode::Pointer m_BrainMaskNode;

private:

  enum FilterType {
    NOFILTERSELECTED,
    NLMR,
    NLMV
  }m_SelectedFilter;

  void ResetParameterPanel();
};



#endif // _QmitkDenoisingView_H_INCLUDED
