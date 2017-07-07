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

#include <mitkPolhemusInterface.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include <PDI.h>

BYTE  MotionBuf[0x1FA400];

mitk::PolhemusInterface::PolhemusInterface() : m_continousTracking(false)
{
	m_pdiDev = new CPDIdev();

}

mitk::PolhemusInterface::~PolhemusInterface()
{
	delete m_pdiDev;
}

bool mitk::PolhemusInterface::InitializeDevice()
{
  m_pdiDev->ResetTracker();
  m_pdiDev->ResetSAlignment(-1);
  m_pdiDev->Trace(TRUE, 7);
  m_continousTracking = false;
  return true;
}

bool mitk::PolhemusInterface::SetupDevice()
{
  m_pdiDev->SetPnoBuffer(MotionBuf, 0x1FA400);
  m_pdiDev->SetMetric(true); //use cm instead of inches

  m_pdiDev->StartPipeExport();

  CPDImdat pdiMDat;
  pdiMDat.Empty();
  pdiMDat.Append(PDI_MODATA_FRAMECOUNT);
  pdiMDat.Append(PDI_MODATA_POS);
  pdiMDat.Append(PDI_MODATA_ORI);
  m_pdiDev->SetSDataList(-1, pdiMDat);

  CPDIbiterr cBE;
  m_pdiDev->GetBITErrs(cBE);

  if (!(cBE.IsClear())) {m_pdiDev->ClearBITErrs();}

  if (this->m_HemisphereTrackingEnabled) { m_pdiDev->SetSHemiTrack(-1); }
  else { m_pdiDev->SetSHemisphere(-1, { (float)2.54,0,0 }); }

  return true;
}

bool mitk::PolhemusInterface::StartTracking()
{
  LPCTSTR	szWindowClass = _T("PDIconsoleWinClass");
  HINSTANCE hInst = GetModuleHandle(0);
  HWND	hwnd = CreateWindowEx(
    WS_EX_NOACTIVATE,//WS_EX_STATICEDGE, //
    szWindowClass,
    _T("MyWindowName"),
    WS_POPUP,
    0, 0, 1, 1,
    HWND_MESSAGE,
    0,
    hInst,
    0);

  m_continousTracking = true;
  return m_pdiDev->StartContPno(hwnd);
}

bool mitk::PolhemusInterface::StopTracking()
{
  m_continousTracking = false;
  return true;
}

bool mitk::PolhemusInterface::Connect()
{
  if (!InitializeDevice()) { return false; }

  if (m_pdiDev->CnxReady()) { return true; }
  CPDIser	pdiSer;
  m_pdiDev->SetSerialIF(&pdiSer);

  ePiCommType eType = m_pdiDev->DiscoverCnx();
  switch (eType)
  {
  case PI_CNX_USB:
    MITK_INFO << "USB Connection: " << m_pdiDev->GetLastResultStr();
    break;
  case PI_CNX_SERIAL:
    MITK_INFO << "Serial Connection: " << m_pdiDev->GetLastResultStr();
    break;
  default:
    MITK_INFO << "DiscoverCnx result: " << m_pdiDev->GetLastResultStr();
    break;
  }

  if (!SetupDevice()) { return false; }

  return m_pdiDev->CnxReady();
}

bool mitk::PolhemusInterface::Disconnect()
{
  if (m_continousTracking)
  {
    m_continousTracking = false;
    if (!m_pdiDev->Disconnect()) return false;
  }
  return true;
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::GetLastFrame()
{
  PBYTE pBuf;
  DWORD dwSize;

  //read one frame
  if (!m_pdiDev->LastPnoPtr(pBuf, dwSize)) {MITK_WARN << m_pdiDev->GetLastResultStr();}

  std::vector<mitk::PolhemusInterface::trackingData> returnValue = ParsePolhemusRawData(pBuf, dwSize);

  if (returnValue.empty()) { MITK_WARN << "Cannot parse data / no tools present"; }

  return returnValue;
}

unsigned int mitk::PolhemusInterface::GetNumberOfTools()
{
  if (m_continousTracking) return GetLastFrame().size();
  else return GetSingleFrame().size();
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::GetSingleFrame()
{
  if (m_continousTracking)
  {
    MITK_WARN << "Cannot get tool count when continously tracking";
    return std::vector<mitk::PolhemusInterface::trackingData>();
  }
  PBYTE pBuf;
  DWORD dwSize;

  //read one frame
  if (!m_pdiDev->ReadSinglePnoBuf(pBuf, dwSize)) {
    MITK_WARN << m_pdiDev->GetLastResultStr();
    return std::vector<mitk::PolhemusInterface::trackingData>();
  }

  return ParsePolhemusRawData(pBuf, dwSize);
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::ParsePolhemusRawData(PBYTE pBuf, DWORD dwSize)
{
  std::vector<mitk::PolhemusInterface::trackingData> returnValue;

  DWORD i = 0;

  while (i<dwSize)
  {
    BYTE ucSensor = pBuf[i + 2];
    SHORT shSize = pBuf[i + 6];

    // skip rest of header
    i += 8;

    PDWORD pFC = (PDWORD)(&pBuf[i]);
    PFLOAT pPno = (PFLOAT)(&pBuf[i + 4]);

    mitk::PolhemusInterface::trackingData currentTrackingData;

    currentTrackingData.id = ucSensor;

    currentTrackingData.pos[0] = pPno[0] * 10; //from cm to mm
    currentTrackingData.pos[1] = pPno[1] * 10;
    currentTrackingData.pos[2] = pPno[2] * 10;

    double azimuthAngle = pPno[3] / 180 * M_PI; //from degree to rad
    double elevationAngle = pPno[4] / 180 * M_PI;
    double rollAngle = pPno[5] / 180 * M_PI;
    vnl_quaternion<double> eulerQuat(rollAngle, elevationAngle, azimuthAngle);
    currentTrackingData.rot = eulerQuat;

    returnValue.push_back(currentTrackingData);
    i += shSize;
  }
  return returnValue;
}
