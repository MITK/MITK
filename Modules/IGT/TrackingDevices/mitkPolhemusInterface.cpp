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
  m_pdiDev.SetPnoBuffer(MotionBuf, 0x1FA400);
}

mitk::PolhemusInterface::~PolhemusInterface()
{

}

bool mitk::PolhemusInterface::StartTracking()
{
    //return m_pdiDev.StartContPno(0);
  return true;
}

bool mitk::PolhemusInterface::StopTracking()
{

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
  m_pdiDev.Disconnect();
  return false;
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::GetLastFrame()
{
  PBYTE pBuf;
  DWORD dwSize;

  //read one frame
  if (!m_pdiDev.ReadSinglePnoBuf(pBuf, dwSize)) {MITK_WARN << m_pdiDev.GetLastResultStr();}

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

    currentTrackingData.pos[0] = pPno[0];
    currentTrackingData.pos[1] = pPno[1];
    currentTrackingData.pos[2] = pPno[2];

    double azimuthAngle = pPno[3];
    double elevationAngle = pPno[4];
    double rollAngle = pPno[5];
    vnl_quaternion<double> eulerQuat(rollAngle, elevationAngle, azimuthAngle);
        currentTrackingData.rot = eulerQuat;

    returnValue.push_back(currentTrackingData);
    i += shSize;
  }
  return returnValue;
}
