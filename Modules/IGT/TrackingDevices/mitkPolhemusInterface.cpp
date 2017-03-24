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

BYTE  MotionBuf[0x1FA400];

mitk::PolhemusInterface::PolhemusInterface()
{
  m_pdiDev.Trace(TRUE, 7);
  m_pdiDev.SetPnoBuffer(MotionBuf, 0x1FA400);
  m_pdiDev.SetMetric(true); //use cm instead of inches
  CPDImdat pdiMDat;
  pdiMDat.Empty();
  pdiMDat.Append(PDI_MODATA_FRAMECOUNT);
  pdiMDat.Append(PDI_MODATA_POS);
  pdiMDat.Append(PDI_MODATA_ORI);
  m_pdiDev.SetSDataList(-1, pdiMDat);

  DWORD dwFrameSize = 8 + 12 + 12 + 4;
}

mitk::PolhemusInterface::~PolhemusInterface()
{

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
  return m_pdiDev.StartContPno(hwnd);
}

bool mitk::PolhemusInterface::StopTracking()
{
  m_continousTracking = false;
  return true;
}

bool mitk::PolhemusInterface::Connect()
{
  m_pdiDev.Trace(TRUE, 7);
  if (m_pdiDev.CnxReady()) { return true; }

  ePiCommType eType = m_pdiDev.DiscoverCnx();
  switch (eType)
  {
  case PI_CNX_USB:
    MITK_INFO << "USB Connection: " << m_pdiDev.GetLastResultStr();
    break;
  case PI_CNX_SERIAL:
    MITK_INFO << "Serial Connection: " << m_pdiDev.GetLastResultStr();
    break;
  default:
    MITK_INFO << "DiscoverCnx result: " << m_pdiDev.GetLastResultStr();
    break;
  }

  return m_pdiDev.CnxReady();
}

bool mitk::PolhemusInterface::Disconnect()
{
  if (m_continousTracking)
  {
    m_continousTracking = false;
    if (!m_pdiDev.Disconnect()) return false;
  }
  return true;
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::GetLastFrame()
{
  PBYTE pBuf;
  DWORD dwSize;

  //read one frame
  if (!m_pdiDev.LastPnoPtr(pBuf, dwSize)) {MITK_WARN << m_pdiDev.GetLastResultStr();}

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
  if (m_continousTracking) return std::vector<mitk::PolhemusInterface::trackingData>();

  PBYTE pBuf;
  DWORD dwSize;

  //read one frame
  if (!m_pdiDev.ReadSinglePnoBuf(pBuf, dwSize)) { MITK_WARN << m_pdiDev.GetLastResultStr(); }

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

    currentTrackingData.pos[0] = pPno[0] * 10;
    currentTrackingData.pos[1] = pPno[1] * 10;
    currentTrackingData.pos[2] = pPno[2] * 10;

    double azimuthAngle = pPno[3] / 180 * 3.14;
    double elevationAngle = pPno[4] / 180 * 3.14;
    double rollAngle = pPno[5] / 180 * 3.14;
    vnl_quaternion<double> eulerQuat(rollAngle, elevationAngle, azimuthAngle);
    currentTrackingData.rot = eulerQuat;

    returnValue.push_back(currentTrackingData);
    i += shSize;
  }
  return returnValue;
}
