/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKPAUSVIEWERVIEW_H_INCLUDED
#define QMITKPAUSVIEWERVIEW_H_INCLUDED

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkBaseRenderer.h>
#include "QmitkRenderWindow.h"

#include "ui_QmitkPAUSViewerViewControls.h"
#include "org_mitk_gui_qt_photoacoustics_pausviewer_Export.h"

#include "mitkCommon.h"

class PHOTOACOUSTICS_PAUSVIEWER_EXPORTS QmitkPAUSViewerView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkPAUSViewerView();
  ~QmitkPAUSViewerView() override;

  void CreateQtPartControl(QWidget *parent) override;
  void InitWindows();

  void SetPADataStorage(mitk::StandaloneDataStorage::Pointer paStore);
  void SetUSDataStorage(mitk::StandaloneDataStorage::Pointer usStore);

  vtkRenderWindow* GetPARenderWindow();
  vtkRenderWindow* GetUSRenderWindow();

  void SetUltrasoundReference(QmitkPAUSViewerView** ultrasoundReference);

protected:

  void AddOverlays();
  void RemoveOverlays();

  mitk::StandaloneDataStorage::Pointer m_PADataStorage;
  mitk::StandaloneDataStorage::Pointer m_USDataStorage;
  mitk::BaseRenderer::Pointer m_PARenderer;
  mitk::BaseRenderer::Pointer m_USRenderer;

  void SetFocus() override;

  void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&) override;

  Ui::QmitkPAUSViewerViewControls* m_Controls;
  QmitkPAUSViewerView** m_UltrasoundReference;
};

#endif // QMITKPAUSVIEWERVIEW_H_INCLUDED

