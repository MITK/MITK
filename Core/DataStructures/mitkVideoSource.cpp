
#include "mitkVideoSource.h"

mitk::VideoSource::VideoSource()
{ 
  m_CaptureWidth  = 720;
  m_CaptureHeight = 576;
  m_CapturingInProcess = false;
  m_CurrentVideoTexture = NULL;
}

mitk::VideoSource::~VideoSource()
{
  delete m_CurrentVideoTexture;
}

unsigned char* mitk::VideoSource::GetVideoTexture()
{ // Fetch Frame and return pointer to opengl texture
  return 0;
}

void mitk::VideoSource::StartCapturing()
{   
  m_CapturingInProcess = true;
}

void mitk::VideoSource::StopCapturing()
{
  mitk::VideoSource::m_CapturingInProcess = false;
}

bool mitk::VideoSource::IsCapturingEnabled()
{
  return m_CapturingInProcess;
}

void mitk::VideoSource::FetchFrame()
{

}
