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

#ifndef _QMITKPREPROCESSINGVIEW_H_INCLUDED
#define _QMITKPREPROCESSINGVIEW_H_INCLUDED

#include <QmitkFunctionality.h>

#include <string>

#include "ui_QmitkPreprocessingViewControls.h"

#include "mitkDiffusionImage.h"

#include <berryIPartListener.h>
#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

typedef short DiffusionPixelType;

struct PrpSelListener;

/*!
 * \ingroup org_mitk_gui_qt_preprocessing_internal
 *
 * \brief QmitkPreprocessingView
 *
 * Document your class here.
 *
 * \sa QmitkFunctionality
 */
class QmitkPreprocessingView : public QmitkFunctionality
{

  friend struct PrpSelListener;

  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

  static const std::string VIEW_ID;

  QmitkPreprocessingView();
  QmitkPreprocessingView(const QmitkPreprocessingView& other);
  virtual ~QmitkPreprocessingView();

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

  void AverageGradients();
  void DoAverageGradients(mitk::DataStorage::SetOfObjects::Pointer inImages);

  void ExtractB0();
  void DoExtractB0(mitk::DataStorage::SetOfObjects::Pointer inImages);

  void BrainMask();
  void DoBrainMask(mitk::DataStorage::SetOfObjects::Pointer inImages);

  void ApplyMesurementFrame();

protected:

  Ui::QmitkPreprocessingViewControls* m_Controls;

  QmitkStdMultiWidget* m_MultiWidget;

  void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

  berry::ISelectionListener::Pointer m_SelListener;
  berry::IStructuredSelection::ConstPointer m_CurrentSelection;
  mitk::DiffusionImage<DiffusionPixelType>::Pointer m_DiffusionImage;
};




#endif // _QMITKPREPROCESSINGVIEW_H_INCLUDED

