/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __DOSE_VISUALIZATION_PREFERENCE_PAGE_H
#define __DOSE_VISUALIZATION_PREFERENCE_PAGE_H

#include "berryIQtPreferencePage.h"
#include "berryIPreferences.h"

#include "ui_DoseVisualizationPreferencePageControls.h"

#include <mitkIsoDoseLevelCollections.h>
#include "mitkDoseVisPreferenceHelper.h"

/*forward declarations*/
class QmitkIsoDoseLevelSetModel;
class QmitkDoseColorDelegate;
class QmitkDoseValueDelegate;
class QmitkDoseVisualStyleDelegate;

class QWidget;

/**
* \class DoseVisualizationPreferencePage
* \brief Preference page for RT Dose visualization
*/
class DoseVisualizationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
    Q_INTERFACES(berry::IPreferencePage)

public:
  DoseVisualizationPreferencePage();
  ~DoseVisualizationPreferencePage() override;

  /**
  * \brief Called by framework to initialize this preference page, but currently does nothing.
  * \param workbench The workbench.
  */
  void Init(berry::IWorkbench::Pointer workbench) override;

  /**
  * \brief Called by framework to create the GUI, and connect signals and slots.
  * \param widget The Qt widget that acts as parent to all GUI components, as this class itself is not derived from QWidget.
  */
  void CreateQtControl(QWidget* widget) override;

  /**
  * \brief Required by framework to get hold of the GUI.
  * \return QWidget* the top most QWidget for the GUI.
  */
  QWidget* GetQtControl() const override;

  /**
  * \see IPreferencePage::PerformOk
  */
  bool PerformOk() override;

  /**
  * \see IPreferencePage::PerformCancel
  */
  void PerformCancel() override;

  /**
  * \see IPreferencePage::Update
  */
  void Update() override;

  public slots:

    void OnCurrentItemChanged ( QListWidgetItem * currentItem, QListWidgetItem * previousItem);
    void OnShowContextMenuIsoSet(const QPoint& pos);
    void OnAddPresetClicked(bool checked);
    void OnDelPresetClicked(bool checked);
    void OnResetPresetClicked(bool checked);
    void OnAddLevelClicked(bool checked);
    void OnDelLevelClicked(bool checked);
    void OnReferenceDoseChanged(double dose);
    void OnGlobalVisChanged(bool vis);

protected:

  /** Method updates the presets widgets according to the internal members*/
  void UpdatePresetsWidgets();
  /** Method actualizes the level set model and edit buttons according to the currently selected item in the presets list view*/
  void UpdateLevelSetWidgets();

  /** Method returns the iso dose level set selected in the preset lists. May return nullptr if no preset is selected.*/
  mitk::IsoDoseLevelSet* GetSelectedIsoLevelSet();

  QWidget                 *m_MainControl;
  Ui::DoseVisualizationPreferencePageControls* m_Controls;

  berry::IPreferences::Pointer m_DoseVisNode;

  typedef mitk::PresetMapType PresetMapType;
  PresetMapType m_Presets;
  std::string m_selectedPresetName;

  bool m_referenceDoseChanged;
  bool m_presetMapChanged;
  bool m_globalVisChanged;

  QmitkIsoDoseLevelSetModel* m_LevelSetModel;
  QmitkDoseColorDelegate* m_DoseColorDelegate;
  QmitkDoseValueDelegate* m_DoseValueDelegate;
  QmitkDoseVisualStyleDelegate* m_DoseVisualDelegate;

};

#endif
