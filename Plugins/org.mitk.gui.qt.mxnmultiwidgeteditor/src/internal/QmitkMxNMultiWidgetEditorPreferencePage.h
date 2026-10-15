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
#include <QStringList>
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
  * \brief Reads the optional 'editorId' parameter, the editor whose
  *        preference node this page edits.
  *
  *   Without parameters the page edits the stock MxN editor. An unknown
  *   parameter, a blank or unregistered 'editorId', or unnamed data is a
  *   configuration error: the page shows it instead of its controls and
  *   leaves all preferences untouched. Never throws, because the preferences
  *   dialog uses created pages without error handling.
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
   * @brief Clears the editor's preferences and refreshes the GUI.
   */
  void ResetPreferencesAndGUI();

private:

  std::unique_ptr<Ui::QmitkMxNMultiWidgetEditorPreferencePage> m_Ui;
  QWidget* m_MainControl = nullptr;

  QString m_EditorId;
  QString m_PageContext;
  QStringList m_ConfigurationErrors;

  mitk::IPreferences* GetPreferences() const;
  void RecordConfigurationError(const QString& parameter, const QString& problem);
  void ShowConfigurationErrors(QWidget* parent);

};

#endif
