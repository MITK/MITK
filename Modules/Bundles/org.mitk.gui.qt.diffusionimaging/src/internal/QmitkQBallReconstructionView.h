/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED
#define _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkQBallReconstructionViewControls.h"

#include "mitkDiffusionImage.h"

#include <berryIPartListener.h>
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

typedef short DiffusionPixelType;

struct QbrSelListener;

/*!
 * \ingroup org_mitk_gui_qt_qballreconstruction_internal
 *
 * \brief QmitkQBallReconstructionView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkQBallReconstructionView : public QObject, public QmitkFunctionality
{

  friend struct QbrSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkQBallReconstructionView();
  virtual ~QmitkQBallReconstructionView();

  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Creation of the connections of main and control widget
  virtual void CreateConnections();

  /// \brief Called when the functionality is activated
  virtual void Activated();

  virtual void Deactivated();

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  static const int nrconvkernels;

protected slots:

  void PreprocCheckboxClicked();

  void AverageGradients();
  void DoAverageGradients(mitk::DataStorage::SetOfObjects::Pointer inImages);
  void ReconstructStandard();
  //void ReconstructNormalized1();
  //void ReconstructNormalized2();
  //void ReconstructNonNormalized();
  //void AnalyticallyReconstructStandard();
  //void AnalyticallyReconstructSolidAngle();
  //void AnalyticallyReconstructNonNegSolidAngle();
  //void AnalyticallyReconstructAdc();
  //void AnalyticallyReconstructRaw();
  void MultiLambdasClicked();
  void AdvancedCheckboxClicked();
  void MethodChoosen(int method);
  
  void Reconstruct(int method, int normalization);
  
  void NumericalQBallReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages, int normalization);
  void AnalyticalQBallReconstruction(mitk::DataStorage::SetOfObjects::Pointer inImages, int normalization);

  void ExtractB0();
  void DoExtractB0(mitk::DataStorage::SetOfObjects::Pointer inImages);

protected:

  Ui::QmitkQBallReconstructionViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  template<int L>
  void TemplatedAnalyticalQBallReconstruction(mitk::DiffusionImage<DiffusionPixelType>* vols, 
    float lambda, std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes, int normalization);

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;

};




#endif // _QMITKQBALLRECONSTRUCTIONVIEW_H_INCLUDED

