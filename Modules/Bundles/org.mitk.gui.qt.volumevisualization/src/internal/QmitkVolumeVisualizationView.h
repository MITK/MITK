/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15646 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKVOLUMEVISUALIZATIONVIEW_H_
#define QMITKVOLUMEVISUALIZATIONVIEW_H_

#include <QmitkFunctionality.h>

#include <berryISelectionListener.h>
#include <mitkDataNodeSelection.h>

#include <mitkWeakPointer.h>

#include <mitkImage.h>

#include "mitkDataStorage.h"

#include <QmitkDataStorageListModel.h>
#include <QmitkDataStorageComboBox.h>
#include <QmitkTransferFunctionWidget.h>

#include "ui_QmitkVolumeVisualizationViewControls.h"

/**
 * \ingroup org_mitk_gui_qt_volumevisualization_internal
 */
class QmitkVolumeVisualizationView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:

  void SetFocus();

  QmitkVolumeVisualizationView();

  virtual ~QmitkVolumeVisualizationView();

  virtual void CreateQtPartControl(QWidget *parent);
  
  ///
  /// Invoked when the DataManager selection changed
  ///
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );


protected slots:

  void OnEnableRendering( bool state );
  void OnEnableLOD( bool state );
  void OnEnableGPU( bool state );


protected:

  ///
  /// A selection listener for datatreenode events
  ///
  berry::ISelectionListener::Pointer m_SelectionListener;

  Ui::QmitkVolumeVisualizationViewControls* m_Controls;

private:

  mitk::WeakPointer<mitk::DataNode> m_SelectedNode;
  
  void UpdateInterface();

};

#endif /*QMITKVOLUMEVISUALIZATIONVIEW_H_*/
