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
#include "ui_QmitkHeadMotionCorrectionViewControls.h"

#include <mitkImage.h>
#include <mitkDWIHeadMotionCorrectionFilter.h>

#include <QThread>
#include <QTime>

typedef short DiffusionPixelType;

/*!
\brief View for diffusion image registration / head motion correction
*/
class QmitkHeadMotionCorrectionView : public QmitkAbstractView
{

  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkHeadMotionCorrectionView();
  virtual ~QmitkHeadMotionCorrectionView();

  virtual void CreateQtPartControl(QWidget *parent) override;
  void SetFocus() override;

  typedef mitk::DWIHeadMotionCorrectionFilter       DWIHeadMotionCorrectionFilterType;

protected slots:

    void StartRegistration();
    void UpdateGUI();             ///< update button activity etc. dpending on current datamanager selection


protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkHeadMotionCorrectionViewControls* m_Controls;

  mitk::Image::Pointer      m_DiffusionImage;
  std::vector< mitk::DataNode::Pointer >            m_SelectedDiffusionNodes;

private:

  void UpdateRegistrationStatus();  ///< update textual status display of the Registration process

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

};
