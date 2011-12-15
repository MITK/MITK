#include "QmitkStatisticsAction.h"

QmitkStatisticsAction::QmitkStatisticsAction(): m_BlueBerryView(NULL)
{
}

QmitkStatisticsAction::~QmitkStatisticsAction()
{
}

void QmitkStatisticsAction::Run(const std::vector<mitk::DataNode*>& selectedNodes)
{
  berry::IBundle::Pointer imageStatisticsBundle = berry::Platform::GetBundle("org.mitk.gui.qt.imagestatistics");

  if (m_BlueBerryView && imageStatisticsBundle.IsNotNull())
  {
    m_BlueBerryView->GetSite()->GetWorkbenchWindow()->GetActivePage()->ShowView("org.mitk.views.imagestatistics");
  }
}

void QmitkStatisticsAction::SetFunctionality(berry::QtViewPart* functionality)
{
  this->m_BlueBerryView = functionality;
}

void QmitkStatisticsAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  //not needed
}

void QmitkStatisticsAction::SetStdMultiWidget(QmitkStdMultiWidget *)
{
  // not needed
}

void QmitkStatisticsAction::SetSmoothed(bool smoothed)
{
  //not needed
}

void QmitkStatisticsAction::SetDecimated(bool decimated)
{
  //not needed
}

