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


#ifndef RTDoseVisualizer_h
#define RTDoseVisualizer_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <mitkDataNodeFactory.h>

#include <mitkDicomRTReader.h>

#include "ui_RTDoseVisualizerControls.h"

#include <mitkIsoDoseLevelCollections.h>

#include <mitkImage.h>

#include "mitkDoseVisPreferenceHelper.h"

// Shader
#include <mitkCoreServices.h>
#include <mitkIShaderRepository.h>
#include <mitkShaderProperty.h>

/*forward declarations*/
class QmitkIsoDoseLevelSetModel;
class QmitkDoseColorDelegate;
class QmitkDoseValueDelegate;
class QmitkDoseVisualStyleDelegate;

/**
  \brief RTDoseVisualizer

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class RTDoseVisualizer : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    RTDoseVisualizer();
    virtual ~RTDoseVisualizer();
    static const std::string VIEW_ID;

  protected slots:

    void OnConvertButtonClicked();

    void OnAddFreeValueClicked();

    void OnRemoveFreeValueClicked();

    void OnUsePrescribedDoseClicked();

    void OnAbsDoseToggled(bool);

    void OnGlobalVisColorWashToggled(bool);

    void OnGlobalVisIsoLineToggled(bool);

    void OnShowContextMenuIsoSet(const QPoint&);

    void OnCurrentPresetChanged(const QString&);

    void OnReferenceDoseChanged(double);

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes );

    /** Method updates the list widget according to the current m_freeIsoValues.*/
    void UpdateFreeIsoValues();

    /** Update the members according to the currently selected node */
    void UpdateBySelectedNode();

    /** Update the member widgets according to the information stored in the application preferences*/
    void UpdateByPreferences();

    /**helper function that iterates throug all data nodes and sets there iso level set property
     according to the selected preset.
     @TODO: should be moved outside the class, to be available for other classes at well.*/
    void ActualizeIsoLevelsForAllDoseDataNodes();

    /**helper function that iterates throug all data nodes and sets there reference dose value
     according to the preference.
     @TODO: should be moved outside the class, to be available for other classes at well.*/
    void ActualizeReferenceDoseForAllDoseDataNodes();

    /**helper function that iterates through all data nodes and sets there dose display style (relative/absolute)
     according to the preference.
     @TODO: should be moved outside the class, to be available for other classes at well.*/
    void ActualizeDisplayStyleForAllDoseDataNodes();

    Ui::RTDoseVisualizerControls m_Controls;
    mitk::DataNode::Pointer m_selectedNode;
    mitk::IsoDoseLevelVector::Pointer m_freeIsoValues;
    /** Iso level set of the current node. Should normaly be a clone of the
     * current iso preset. It held as own member because visibility
     * settings may differ.*/
    mitk::IsoDoseLevelSet::Pointer m_selectedNodeIsoSet;
    mitk::rt::PresetMapType m_Presets;
    std::string m_selectedPresetName;

    /** Prescribed Dose of the selected data.*/
    mitk::DoseValueAbs m_PrescribedDose_Data;

    QmitkIsoDoseLevelSetModel* m_LevelSetModel;
    QmitkDoseColorDelegate* m_DoseColorDelegate;
    QmitkDoseValueDelegate* m_DoseValueDelegate;
    QmitkDoseVisualStyleDelegate* m_DoseVisualDelegate;


    bool m_internalUpdate;
};

#endif // RTDoseVisualizer_h
