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

#include <QObject>
#include <QmitkFunctionality.h>
#include <QmitkDataStorageListModel.h>
#include <QmitkDataStorageComboBox.h>
#include <QmitkTransferFunctionWidget.h>

/**
 * \ingroup org_mitk_gui_qt_volumevisualization_internal
 */
class QmitkVolumeVisualizationView : public QObject, public QmitkFunctionality
{
  Q_OBJECT

public:

  void SetFocus();

  ~QmitkVolumeVisualizationView();

protected:

  void CreateQtPartControl(QWidget* parent);

protected slots:

  void comboIndexChanged(int index);

  void OnTransferFunctionModeChanged( int mode );

private:

  QmitkDataStorageComboBox* m_DataStorageComboBox;
  QmitkTransferFunctionWidget* m_TransferFunctionWidget;

};

#endif /*QMITKVOLUMEVISUALIZATIONVIEW_H_*/
