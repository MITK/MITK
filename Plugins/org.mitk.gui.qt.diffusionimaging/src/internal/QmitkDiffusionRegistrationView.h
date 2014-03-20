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



#include <berryISelectionListener.h>
#include <berryIStructuredSelection.h>

#include <QmitkAbstractView.h>
#include "ui_QmitkDiffusionRegistrationViewControls.h"

#include "mitkDiffusionImage.h"
#include <mitkDWIHeadMotionCorrectionFilter.h>

#include <QThread>
#include <QTime>

typedef short DiffusionPixelType;

/*!
\brief View for diffusion image registration / head motion correction

\sa QmitkFunctionality
\ingroup Functionalities
*/

// Forward Qt class declarations

using namespace std;

class QmitkDiffusionRegistrationView;

class QmitkRegistrationWorker : public QObject
{
  Q_OBJECT

public:

  QmitkRegistrationWorker(QmitkDiffusionRegistrationView* view);

  public slots:

    void run();

private:

  QmitkDiffusionRegistrationView* m_View;
};


class QmitkDiffusionRegistrationView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const string VIEW_ID;

  QmitkDiffusionRegistrationView();
  virtual ~QmitkDiffusionRegistrationView();

  virtual void CreateQtPartControl(QWidget *parent);
  void SetFocus();

  typedef mitk::DWIHeadMotionCorrectionFilter< DiffusionPixelType >       DWIHeadMotionCorrectionFilterType;

protected slots:

    void StartRegistration();
    void StopRegistration();

    void AfterThread();                       ///< update gui etc. after registrations has finished
    void BeforeThread();                      ///< start timer etc.
    void TimerUpdate();

    void AddInputFolderName();
    void AddOutputFolderName();
    void StartBatch();

    void AdvancedSettings();


protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer, const QList<mitk::DataNode::Pointer>&);

  Ui::QmitkDiffusionRegistrationViewControls* m_Controls;

  mitk::DiffusionImage<DiffusionPixelType>::Pointer m_DiffusionImage;
  std::vector< mitk::DataNode::Pointer >            m_SelectedDiffusionNodes;

private:

  void UpdateRegistrationStatus();  ///< update textual status display of the Registration process
  void UpdateGUI();             ///< update button activity etc. dpending on current datamanager selection

  /** flags etc. */
  bool            m_IsBatch, m_IsAborted;
  QStringList     m_BatchList;
  bool            m_ThreadIsRunning;
  QTimer*         m_RegistrationTimer;
  QTime           m_RegistrationTime;
  unsigned long   m_ElapsedTime;
  unsigned long   m_Steps;
  int             m_LastStep;
  unsigned int    m_CurrentFile;
  unsigned int    m_TotalFiles;

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  /** global Registerer and friends */
  itk::SmartPointer<DWIHeadMotionCorrectionFilterType> m_GlobalRegisterer;
  QmitkRegistrationWorker m_RegistrationWorker;
  QThread m_RegistrationThread;
  friend class QmitkRegistrationWorker;

};
