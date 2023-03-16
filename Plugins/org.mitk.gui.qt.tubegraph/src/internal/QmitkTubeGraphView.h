/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTubeGraphView_h
#define QmitkTubeGraphView_h

#include "ui_QmitkTubeGraphViewControls.h"

#include <QmitkAbstractView.h>

#include "mitkColorProperty.h"
#include "mitkTubeGraph.h"
#include "mitkTubeGraphProperty.h"
#include "mitkTubeGraphDataInteractor.h"

#include <QCheckBox>
#include <QSignalMapper>


/*!
*
* \brief QmitkTubeGraphConverterView
*
*/
class QmitkTubeGraphView: public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  typedef  mitk::TubeGraphProperty::LabelGroup  LabelGroupType;
  typedef LabelGroupType::Label                 LabelType;

public:

  static const std::string VIEW_ID;
  /**
  * @brief Constructor.
  **/
  QmitkTubeGraphView();

  /**
  * @brief Destructor.
  */
  ~QmitkTubeGraphView() override;

  void CreateQtPartControl(QWidget *parent) override;

  /** \brief Method to create the connections for the component. This Method is obligatory even if no connections is needed*/
  virtual void CreateConnections();

  protected slots:
    //active tube graph
    void OnActiveGraphChanged(int);

    //activation mode
    void OnActivationModeChanged();
    void OnDeselectAllTubes();
    void OnSetRootToggled(bool);

    void OnTabSwitched(int);

    //attributation tab
    void OnAddingLabelGroup();
    void OnRemoveLabelGroup();

    //annotation tab
    void OnAddingAnnotation();
    void OnRemoveAnnotation();

    //edit tab
    void OnAddTubeBetweenSelection();
    void OnSeperateSelection();
    void OnDeleteSelection();

    //label group
    void OnLabelVisibilityChanged(bool, QString labelName, QString labelGroupName);
    void OnLabelChanged(QString labelName, QString labelGroupName);
    void OnLabelColorChanged(mitk::Color color, QString labelName, QString labelGroupName);

protected:

  mitk::TubeGraphDataInteractor::ActivationMode GetActivationMode();
  void SetFocus() override;
  void NodeRemoved(const mitk::DataNode*) override;
  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkTubeGraphViewControls m_Controls;
  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

private:
  void UpdateActiveTubeGraphInInteractors();
  void CreateLabelGroupWidget(LabelGroupType* labelGroup);
  void InitializeLabelGroups();
  void SetTabsEnable(bool enable);
  void UpdateLabelGroups();
  void UpdateAnnotation();
  void SelectionInformationChanged();
  void UpdateGraphInformation();

  mitk::TubeGraph::Pointer                      m_ActiveTubeGraph;
  mitk::TubeGraphProperty::Pointer              m_ActiveProperty;
  mitk::TubeGraphDataInteractor::Pointer        m_ActiveInteractor;
  mitk::TubeGraphDataInteractor::ActivationMode m_ActivationMode;

  /* std::vector<QString> m_LabelGroupName;
  std::vector<QString> m_LabelName;*/

  unsigned int m_InformationChangedObserverTag;

};


#endif
