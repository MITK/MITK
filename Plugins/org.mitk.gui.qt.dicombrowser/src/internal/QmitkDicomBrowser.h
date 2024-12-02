/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDicomBrowser_h
#define QmitkDicomBrowser_h

#include <berryIPartListener.h>
#include <berryQtEditorPart.h>

namespace Ui
{
  class QmitkDicomBrowser;
}

/**
 * \brief An editor for DICOM storage, import, and query/retrieve.
 */
class QmitkDicomBrowser : public berry::QtEditorPart, virtual public berry::IPartListener
{
  Q_OBJECT

public:
  static const std::string EDITOR_ID;

  enum Tabs
  {
    LocalStorage,
    Import,
    QueryRetrieve
  };

  berryObjectMacro(QmitkDicomBrowser)

  QmitkDicomBrowser();
  ~QmitkDicomBrowser() override;

  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;
  void SetFocus() override;
  void DoSave() override;
  void DoSaveAs() override;
  bool IsDirty() const override;
  bool IsSaveAsAllowed() const override;

private:
  void CreateQtPartControl(QWidget* parent) override;
  Events::Types GetPartEventTypes() const override;

  void OnIndexingComplete();
  void OnViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series);

  Ui::QmitkDicomBrowser* m_Ui;
};

#endif
