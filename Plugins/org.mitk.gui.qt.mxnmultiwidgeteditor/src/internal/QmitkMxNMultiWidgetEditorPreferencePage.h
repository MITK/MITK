/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNMultiWidgetEditorPreferencePage_h
#define QmitkMxNMultiWidgetEditorPreferencePage_h

#include <berryIExecutableExtension.h>
#include <berryIQtPreferencePage.h>
#include <QProcess>
#include <QScopedPointer>
#include <QPushButton>
#include <memory>

namespace mitk
{
  class IPreferences;
}

namespace Ui
{
  class QmitkMxNMultiWidgetEditorPreferencePage;
}

class QmitkMxNMultiWidgetEditorPreferencePage : public QObject,
                                                public berry::IQtPreferencePage,
                                                public berry::IExecutableExtension
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage berry::IExecutableExtension)

public:

  QmitkMxNMultiWidgetEditorPreferencePage();
  ~QmitkMxNMultiWidgetEditorPreferencePage() override;

  /**
  * \brief Reads the optional 'editorId' parameter: the editor declaration whose
  *        preference node this page edits. Default: the stock MxN editor.
  *
  *   Never throws, because the preferences dialog creates pages without a
  *   handler; bad input is logged and the default used.
  */
  void SetInitializationData(const berry::IConfigurationElement::Pointer& config,
                             const QString& propertyName,
                             const berry::Object::Pointer& data) override;

  void Init(berry::IWorkbench::Pointer) override;
  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

public slots:
  /**
   * @brief ResetColors set default colors and refresh the GUI.
   */
  void ResetPreferencesAndGUI();

private:

  std::unique_ptr<Ui::QmitkMxNMultiWidgetEditorPreferencePage> m_Ui;
  QWidget* m_MainControl;

  QString m_EditorId;

  mitk::IPreferences* GetPreferences() const;

};

#endif
