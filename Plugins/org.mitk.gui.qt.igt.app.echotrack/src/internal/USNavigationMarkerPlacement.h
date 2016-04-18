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

#ifndef USNAVIGATIONMARKERPLACEMENT_H
#define USNAVIGATIONMARKERPLACEMENT_H

#include <QmitkAbstractView.h>

#include <mitkUSCombinedModality.h>
#include "Widgets/QmitkUSNavigationProcessWidget.h"
#include "IO/mitkUSNavigationLoggingBackend.h"
#include "mitkUSImageLoggingFilter.h"
#include "mitkNavigationDataRecorder.h"
#include "mitkNodeDisplacementFilter.h"
#include <mitkMessage.h>
#include <mitkOverlayManager.h>
#include <mitkTextOverlay2D.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USNavigationStepTimer;
  class USNavigationExperimentLogging;
}

namespace Ui {
  class USNavigationMarkerPlacement;
}

class QmitkUSAbstractNavigationStep;
class QmitkStdMultiWidget;
class QTimer;
class QSignalMapper;

/**
 * \brief View for navigated marker placement using the combined modality.
 * This view utilizes the QmitkUSNavigationProcessWidget to do the navigation
 * process. It can be switched between widgets for marker placement and widgets
 * for punctuation.
 *
 * An experiment mode allows for logging results, durations and the ultrasound
 * images.
 */
class USNavigationMarkerPlacement : public QmitkAbstractView
{
  Q_OBJECT

    protected slots:
  /**
   * \brief Called periodically to update the rendering.
   * The standard multi widget is changed to fit the navigation process once it
   * is available and a reinit on the ultrasound image is done for a new image
   * node.
   */
  void OnTimeout();

  /**
   * \brief Called periodically during an experiment for logging the ultrasound images.
   */
  void OnImageAndNavigationDataLoggingTimeout();

  /**
   * \brief Initializes anything neccessary for an experiment.
   * The user is asked for a directory for storing the results and the logging
   * is started.
   */
  void OnStartExperiment();

  /**
   * \brief Stops logging and saves everything to the file system.
   */
  void OnFinishExperiment();

  void OnCombinedModalityChanged(itk::SmartPointer<mitk::USCombinedModality>);

  /**
   * \brief Switches the navigation step widgets if the navigation application was changed.
   */
  void OnSettingsChanged(itk::SmartPointer<mitk::DataNode>);

  /**
   * \brief Updates the timer for the navigation steps durations every time the active navigation step changes.
   */
  void OnActiveNavigationStepChanged(int);

  /**
   * \brief The data node is given to the experiment logging and scene is saved to the file system.
   */
  void OnIntermediateResultProduced(const itk::SmartPointer<mitk::DataNode>);

  void OnAddAblationZone(int size);

  void OnChangeAblationZone(int id, int newSize);

public:
  static const char* DATANAME_TUMOUR;
  static const char* DATANAME_TARGETSURFACE;
  static const char* DATANAME_ZONES;
  static const char* DATANAME_TARGETS;
  static const char* DATANAME_TARGETS_PATHS;
  static const char* DATANAME_REACHED_TARGETS;

  explicit USNavigationMarkerPlacement();
  ~USNavigationMarkerPlacement();

  virtual void CreateQtPartControl(QWidget *parent);

  static const std::string VIEW_ID;

  void OnCombinedModalityPropertyChanged(const std::string&, const std::string&);

protected:
  /**
   * \brief A reinit on the ultrasound image is performed every time the view gets the focus.
   */
  virtual void SetFocus();

  /**
   * \brief Helper function which performs a reinit on the ultrasound image.
   */
  void ReinitOnImage();

  /**
   * \brief Helper function for being able to serialize the 2d ultrasound image.
   */
  void Convert2DImagesTo3D(mitk::DataStorage::SetOfObjects::ConstPointer nodes);

  void CreateOverlays();

  QWidget*                                             m_Parent;
  QmitkUSNavigationProcessWidget::NavigationStepVector m_NavigationSteps;
  QTimer*                                              m_UpdateTimer;
  QTimer*                                              m_ImageAndNavigationDataLoggingTimer;
  QmitkStdMultiWidget*                                 m_StdMultiWidget;
  itk::SmartPointer<mitk::USCombinedModality>          m_CombinedModality;
  bool                                                 m_ReinitAlreadyDone;
  bool                                                 m_IsExperimentRunning;
  std::string                                          m_CurrentApplicationName;

  itk::SmartPointer<mitk::USNavigationStepTimer>       m_NavigationStepTimer;
  itk::SmartPointer<mitk::USNavigationExperimentLogging> m_ExperimentLogging;

  QPixmap                                              m_IconRunning;
  QPixmap                                              m_IconNotRunning;

  QString                                              m_ResultsDirectory;
  QString                                              m_ExperimentName;
  QString                                              m_ExperimentResultsSubDirectory;
  std::vector<QString>                                 m_NavigationStepNames; //stores the names of the navigation steps which are currently used (for logging purposes)

  mitk::USNavigationLoggingBackend                     m_LoggingBackend;
  mitk::USImageLoggingFilter::Pointer                  m_USImageLoggingFilter;
  mitk::NavigationDataRecorder::Pointer                m_NavigationDataRecorder; //records navigation data files
  mitk::NodeDisplacementFilter::Pointer                m_TargetNodeDisplacementFilter;
  mitk::NodeDisplacementFilter::Pointer                m_AblationZonesDisplacementFilter;
  std::vector<mitk::DataNode::Pointer>                 m_AblationZonesVector;

  int m_NeedleIndex;
  int m_MarkerIndex;

  int                                                  m_SceneNumber;

  itk::SmartPointer<mitk::OverlayManager>              m_OverlayManager;
  itk::SmartPointer<mitk::TextOverlay2D>               m_WarnOverlay;

private:
  mitk::MessageDelegate2<USNavigationMarkerPlacement, const std::string&, const std::string&> m_ListenerDeviceChanged;

  Ui::USNavigationMarkerPlacement *ui;
};

#endif // USNAVIGATIONMARKERPLACEMENT_H
