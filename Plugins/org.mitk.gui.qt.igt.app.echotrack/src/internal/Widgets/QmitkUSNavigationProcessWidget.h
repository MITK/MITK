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

#ifndef USNAVIGATIONPROCESSWIDGET_H
#define USNAVIGATIONPROCESSWIDGET_H

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkUSCombinedModality.h>

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class NodeDisplacementFilter;
}

namespace Ui {
class QmitkUSNavigationProcessWidget;
}

class QmitkUSAbstractNavigationStep;
class QmitkUSNavigationAbstractSettingsWidget;
class QmitkUSNavigationStepCombinedModality;

class QmitkStdMultiWidget;

class QTimer;
class QSignalMapper;

/**
 * \brief Widget for handling navigation steps.
 * The navigation steps (subclasses of QmitkUSAbstractNavigationStep) can be set
 * in a vector. The coordination of this steps is done by this widget then.
 */
class QmitkUSNavigationProcessWidget : public QWidget
{
  Q_OBJECT

signals:
  /**
   * \brief Signals a replacement of the combined modality by one of the navigation steps.
   */
  void SignalCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>);

  /**
   * \brief Signals a change of the navigation settings.
   */
  void SignalSettingsChanged(itk::SmartPointer<mitk::DataNode>);

  /**
   * \brief Signals a change of the currently active navigation step.
   */
  void SignalActiveNavigationStepChanged(int);

  /**
   * \brief Signals that the navigation step with the given id was finished.
   */
  void SignalNavigationStepFinished(int, bool);

  /**
   * \brief Signals the creation of an intermediate result.
   * The result data is given as properties of the data node.
   */
  void SignalIntermediateResult(const itk::SmartPointer<mitk::DataNode>);

protected slots:
  void OnNextButtonClicked();
  void OnPreviousButtonClicked();
  void OnRestartStepButtonClicked();
  void OnTabChanged(int index);
  void OnSettingsButtonClicked();
  void OnSettingsWidgetReturned();
  void OnSettingsNodeChanged(itk::SmartPointer<mitk::DataNode>);

  void OnStepReady(int);
  void OnStepNoLongerReady(int);

  void OnCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>);
  void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode>);

public:
  typedef std::vector<QmitkUSAbstractNavigationStep*> NavigationStepVector;
  typedef NavigationStepVector::iterator              NavigationStepIterator;

  explicit QmitkUSNavigationProcessWidget(QWidget* parent = 0);
  ~QmitkUSNavigationProcessWidget();

  /**
   * \brief Setter for the data storage used for storing the navigation progress.
   */
  void SetDataStorage(itk::SmartPointer<mitk::DataStorage> dataStorage);

  void SetSettingsWidget(QmitkUSNavigationAbstractSettingsWidget*);

  /**
   * \brief Setter for a vector of navigation step widgets.
   * These widgets are used for the navigation process in the order of their
   * appearance in this vector.
   */
  void SetNavigationSteps(NavigationStepVector navigationSteps);

  /**
   * \brief Forget the current progress of the navigation process.
   * The process will then start again at the beginning.
   */
  void ResetNavigationProcess();

  /** Enables/disables the (navigation step) interaction with this widget.
   *  The settings button is not affected by this flag.
   */
  void EnableInteraction(bool enable);

  /** Finishes the current navigation step. */
  void FinishCurrentNavigationStep();

    /** Updates the navigation process widget, which includes updating the
   *  navigation pipeline. Has to be called from outside this class with
   *  a given update rate. So no additional internal timer is needed.
   */
  void UpdateNavigationProgress();

protected:
  void InitializeNavigationStepWidgets();
  void UpdatePrevNextButtons();
  void UpdateFilterPipeline();
  void SetSettingsWidgetVisible(bool visible);

  itk::SmartPointer<mitk::DataStorage>            m_DataStorage;
  NavigationStepVector                            m_NavigationSteps;
  QmitkUSNavigationAbstractSettingsWidget*        m_SettingsWidget;

  itk::SmartPointer<mitk::DataNode>               m_BaseNode;
  itk::SmartPointer<mitk::DataNode>               m_SettingsNode;

  int                                             m_CurrentTabIndex;
  int                                             m_CurrentMaxStep;

  itk::SmartPointer<mitk::DataNode>               m_ImageStreamNode;
  bool                                            m_ImageAlreadySetToNode;

  itk::SmartPointer<mitk::USCombinedModality>     m_CombinedModality;

  itk::SmartPointer<mitk::NavigationDataSource>   m_LastNavigationDataFilter;

  QWidget* m_Parent;
  QSignalMapper* m_ReadySignalMapper;
  QSignalMapper* m_NoLongerReadySignalMapper;

  QmitkStdMultiWidget* m_StdMultiWidget;

  bool m_UsePlanningStepWidget;

private:
  Ui::QmitkUSNavigationProcessWidget *ui;
};

#endif // USNAVIGATIONPROCESSWIDGET_H
