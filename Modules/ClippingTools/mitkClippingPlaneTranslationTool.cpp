#include "mitkClippingPlaneTranslationTool.h"

#include "mitkClippingPlaneTranslationTool.xpm"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkGlobalInteraction.h"

namespace mitk {
  MITK_TOOL_MACRO(ClippingTools_EXPORT, ClippingPlaneTranslationTool, "Clipping Plane Translation");
}

mitk::ClippingPlaneTranslationTool::ClippingPlaneTranslationTool()
: Tool( "global" )
{
}

mitk::ClippingPlaneTranslationTool::~ClippingPlaneTranslationTool()
{
}

const char** mitk::ClippingPlaneTranslationTool::GetXPM() const
{
  return mitkClippingPlaneTranslationTool_xpm;
}

const char* mitk::ClippingPlaneTranslationTool::GetName() const
{
  return "Translation";
}

const char* mitk::ClippingPlaneTranslationTool::GetGroup() const
{
  return "ClippingPlane";
}

void mitk::ClippingPlaneTranslationTool::Activated()
{
  Superclass::Activated();

  //check if the Clipping plane is changed.
  if(m_ClippingPlaneNode != m_ToolManager->GetWorkingData( 0 ))
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_AffineInteractor );
    this->ClippingPlaneChanged();
  }

  m_AffineInteractor->SetInteractionModeToTranslation();
  mitk::GlobalInteraction::GetInstance()->AddInteractor( m_AffineInteractor );
}

void mitk::ClippingPlaneTranslationTool::Deactivated()
{
  Superclass::Deactivated();

  mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_AffineInteractor );
}

//Checks the working data node, if it has an interactor. Otherwise initial a new one.
void mitk::ClippingPlaneTranslationTool::ClippingPlaneChanged()
{
  m_ClippingPlaneNode = m_ToolManager->GetWorkingData(0);
  m_AffineInteractor = dynamic_cast<mitk::AffineInteractor3D*>(m_ClippingPlaneNode->GetInteractor());

  if (m_AffineInteractor.IsNull())
  {
    m_AffineInteractor = mitk::AffineInteractor3D::New( 
      "AffineInteractor3D", m_ClippingPlaneNode );
  }
}
