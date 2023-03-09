/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTrackingSourcesCheckBoxPanelWidget_h
#define QmitkTrackingSourcesCheckBoxPanelWidget_h

#include "ui_QmitkTrackingSourcesCheckBoxPanelWidgetControls.h"
#include "MitkIGTUIExports.h"

#include <mitkNavigationDataSource.h>
#include <QCheckBox>




/*!
\brief QmitkTrackingSourcesCheckBoxPanelWidget

Widget for setting up and controlling an update timer in an IGT-Pipeline.

*/
class MITKIGTUI_EXPORT QmitkTrackingSourcesCheckBoxPanelWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public:


  typedef std::vector< QCheckBox* > TrackingSourcesCheckboxes; /// vector with checkboxes for all set NDs

  /*!
  \brief default constructor
  */
  QmitkTrackingSourcesCheckBoxPanelWidget( QWidget* parent );

  /*!
  \brief default destructor
  */
  ~QmitkTrackingSourcesCheckBoxPanelWidget() override;

  /*!
  \brief Shows the checkboxes
  */
  void ShowSourceCheckboxes();

  /*!
  \brief Sets the ND for this widget
  */
  void SetNavigationDatas(std::vector<mitk::NavigationData::Pointer>* navDatas);

  /*!
  \brief Adds a ND.
  */
  void AddNavigationData(mitk::NavigationData::Pointer nd);

  /**
  \brief Sets this widget's info text.
  */
  void SetInfoText(QString text);

  /**
  \brief Sets this widget's action perform button text.
  */
  void SetActionPerformButtonText(QString text);

  /**
  \brief Sets whether the action perform button is checkable or not.
  */
  void SetActionPerformButtonCheckable(bool checkable);

  /**
  \brief Hides or shows the action perfom button.
  */
  void HideActionPerformButton(bool hide);

  /**
  \brief Returns the selected tracking sources IDs.
  */
  const std::vector<int>* GetSelectedTrackingSourcesIDs();

  /**
  \brief Selects all checkboxes in this widget.
  */
  void SelectAll();

  /**
  \brief Deselects all checkboxes in this widget.
  */
  void DeselectAll();

  /**
  \brief Selets the checkbox at the given position.
  */
  void SelectCheckbox(unsigned int idx);

  /**
  \brief Deselects the checkbox at the given position
  */
  void DeselectCheckbox(unsigned int idx);

  /**
  \brief Enables or disabless the checkboxes in this widget.
  */
  void EnableCheckboxes(bool enable);

  /**
  \brief Clears the vector that holds the selected IDs
  */
  void ClearSelectedIDs();

  /**
  \brief Returns whether this widgets "perform action" button is checked
  */
  bool IsActionButtonChecked();

signals:
  void Selected(int id); /// when a checkbox is selected
  void Deselected(int id); /// when a checkbox is deselected
  void PerformAction(); /// when action perfom button is pressed
  void StopAction();  /// when action perform button is released
  void Action(); /// when action perfom button is clicked

public slots:
  void ClearPanel(); /// clearing checkboxes from panel

  protected slots:
    void OnCheckboxClicked(bool checked);
    void OnPerformActionClicked(bool toggled);
    void OnPerformActionClicked();

protected:
  void CreateConnections();
  void CreateQtPartControl( QWidget *parent );
  Ui::QmitkTrackingSourcesCheckBoxPanelWidgetControls* m_Controls;  ///< gui widgets





private:
  TrackingSourcesCheckboxes* m_SourceCheckboxes;
  std::vector<mitk::NavigationData::Pointer>*  m_NavigationDatas;
  std::vector<int>* m_SelectedIds;


};
#endif
