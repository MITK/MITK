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

#ifndef QMITK_FUNCTIONALITYCOMPONENTCONTAINER_H
#define QMITK_FUNCTIONALITYCOMPONENTCONTAINER_H

#include "QmitkBaseFunctionalityComponent.h"
#include "MitkQtWidgetsExtExports.h"

#include <qwidget.h>
 #include <qstackedwidget.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qobject.h>
#include <mitkDataNode.h>
#include <qlayout.h>
#include <qlabel.h>
#include "ui_QmitkFunctionalityComponentContainerControls.h"

#include <QmitkStdMultiWidget.h>

/**
* \brief ContainerClass for components
* \ingroup QmitkFunctionalityComponent
*
*
* - \ref QmitkBaseFunctionalityComponent

*
* \section QmitkFunctionalityComponentContainer Overview
*
* The FunctionalityComponentContainer is a containerclass for several components inherit from it.
* All Components including to one FunctionalityComponentContainer can be used together as one applied
* functionality. I.e. in particular that new functionalities can be combined with different components.
* As common ground for all inherit classes the FunctionalityComponentContainer includes a selector for
* marked data like image(s), segmentation(s) or model(s).
*/


class MitkQtWidgetsExt_EXPORT QmitkFunctionalityComponentContainer : public QmitkBaseFunctionalityComponent
{
  Q_OBJECT

public:
  /** \brief Standard-Constructor. */
  QmitkFunctionalityComponentContainer(QObject *parent=0, const char * parentName = 0,  bool updateSelector = true, bool showSelector = true);

  /** \brief Destructor. */
  virtual ~QmitkFunctionalityComponentContainer();

  /*!
  \brief Method to create all Signale-Slot-Connections
  */
  virtual void CreateConnections();

  /*!
  \brief Method to create the GUI-Object
  */
  virtual void CreateQtPartControl(QWidget *parent, mitk::DataStorage::Pointer dataStorage);

  /** \brief Method to create the forward and backward -buttons to navigate through the wizard */
  void CreateNavigationButtons();

  /** \brief Method to create a textLabel at the Top of the wizard where a description can be created */
  void SetWizardText(const QString & text);

   /** \brief also the Graphical User Interface for the component, like m_GUI, but with its specific type */
  Ui::QmitkFunctionalityComponentContainerGUI* GetFunctionalityComponentContainerGUI();

  /** \brief Method to set the DataStorage*/
  virtual void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  /** \brief Method to get the DataStorage*/
  virtual mitk::DataStorage::Pointer GetDataStorage();

  /** \brief Method to set the Name of the FunctionalityComponent */
  virtual void SetComponentName(QString name);

  /** \brief Method to get the Name of the FunctionalityComponent */
  virtual QString GetComponentName();

  /** \brief Method to get the GUI of this component.*/
  QWidget* GetGUI();

  virtual QmitkDataStorageComboBox * GetTreeNodeSelector();

  /** \brief Method to set the Image Selector visible or invisible */
  virtual void SetSelectorVisibility(bool visibility);

  QmitkStdMultiWidget * GetMultiWidget();

  /** \brief Method to return the ComboBox that includes all GUI-elements instead of the outermost checkable CheckBox and that can be set visible or not*/
  virtual QGroupBox * GetContentContainer();

  /** \brief Method to return the outermost checkable ComboBox that is to decide whether the content shall be shown or not */
  virtual QGroupBox * GetMainCheckBoxContainer();

  /** \brief Method to set the Tree-Node-Selector visible or not as his parent group-box is set shown or not. */
  virtual void SetShowTreeNodeSelector(bool show);

  /** \brief Method to return the group-box that contains the tree-node-selector */
  virtual QGroupBox* GetImageContent();

  /** \brief Method to return the Image Selected in the Container Combo Box */
  virtual mitk::Image* GetParentMitkImage();

  /** \brief Method to return the NextButton to switch to the next widgetStackPage*/
  QPushButton* GetNextButton();

  /** \brief Method to return the BackButton to switch to the last widgetStackPage*/
  QPushButton* GetBackButton();

  /** \brief method to add components into this component. */
  virtual void AddComponent(QmitkFunctionalityComponentContainer* componentContainer);

  /** \brief method to add components into this component. */
  virtual void AddComponent(QmitkFunctionalityComponentContainer* componentContainer, QString label, int stackPage);

  virtual void TreeChanged(const itk::EventObject & treeChangedEvent);

  virtual void Activated();

  virtual void Deactivated();

  /** \brief Attribute to decide whether the selector shall be updated when a parent-Selector is updatet or not */
  bool m_UpdateSelector;

  /** \brief Attribute to decide whether the selector shall be shown or not */
  bool m_ShowSelector;

  /** \brief Slot method that will be called if TreeNodeSelector widget was activated. */
  void ImageSelected(const mitk::DataNode* item);

public slots:

  /** \brief Method to set the "GetContentContainer"-visible or not, addicted to the visibility of a parent-component and the status of the checkable ComboBox from "GetMainCheckBoxContainer()" */
  virtual void SetContentContainerVisibility(bool);

  virtual void TreeChanged();

  virtual  void DataStorageChanged(mitk::DataStorage::Pointer ds);

protected:

  /** \brief Method to update the content of all DataTreeComboBoxes. */
  virtual void UpdateDataTreeComboBoxes();

  unsigned long m_ObserverTag;
  QWidget* m_GUI;
  bool m_Active;
  QmitkDataStorageComboBox * m_SelectedItem;
  mitk::DataStorage::Pointer m_DataStorage;

private:

  /** \brief also the Graphical User Interface for the component, like m_GUI, but with its specific type */
  Ui::QmitkFunctionalityComponentContainerGUI * m_FunctionalityComponentContainerGUI;

  QObject *m_Parent;
  QString m_ComponentName;
  QSpacerItem* m_Spacer;
  QmitkStdMultiWidget* m_MulitWidget;
  QPushButton* m_BackButton;
  QPushButton* m_NextButton;
  int m_MaximumWidgedStackSize;
};

#endif

