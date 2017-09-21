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
  m_numberOfTools = 0;
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

  if (!(cBE.IsClear())) { m_pdiDev->ClearBITErrs(); }

  return true;
}

bool mitk::PolhemusInterface::StartTracking()
{
  m_continousTracking = true;
  return m_pdiDev->StartContPno(0);
}

bool mitk::PolhemusInterface::StopTracking()
{
  m_continousTracking = false;
  m_pdiDev->StopContPno();
  return true;
}

bool mitk::PolhemusInterface::Connect()
{
  bool returnValue;
  //Initialize, and if it is not successful, return false.
  if (!InitializeDevice())
  {
    returnValue = false;
  }
  //Connect
  else if (m_pdiDev->CnxReady())
  {
    returnValue = true;
  }
  //If it is not successful, search for connections.
  else
  {
    CPDIser  pdiSer;
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

    //Setup device
    if (!SetupDevice())
    {
      returnValue = false;
    }
    else
    {
      returnValue = m_pdiDev->CnxReady();
    }
  }

  if (returnValue)
  {
    m_numberOfTools = this->GetNumberOfTools();
    //On first startUp or if number of tools changed
    if (m_Hemispheres.empty() || m_Hemispheres.size() != m_numberOfTools)
    {
      //Set Hemisphere for all tools to default 1,0,0...
      mitk::Vector3D temp;
      mitk::FillVector3D(temp, 1, 0, 0);
      m_Hemispheres.clear();
      m_Hemispheres.assign(m_numberOfTools, temp);
      this->SetHemisphere(-1, temp);
    }
  }

  return returnValue;

}

bool mitk::PolhemusInterface::Disconnect()
{
  bool returnValue = true;
  //If Tracking is running, stop tracking first
  if (m_continousTracking)
  {
    this->StopTracking();
  }

  returnValue = m_pdiDev->Disconnect();
  MITK_INFO << "Disconnect: " << m_pdiDev->GetLastResultStr();
  return returnValue;
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::GetLastFrame()
{
  PBYTE pBuf;
  DWORD dwSize;

  //read one frame
  if (!m_pdiDev->LastPnoPtr(pBuf, dwSize)) { MITK_WARN << m_pdiDev->GetLastResultStr(); }

  std::vector<mitk::PolhemusInterface::trackingData> returnValue = ParsePolhemusRawData(pBuf, dwSize);

  if (returnValue.empty())
  {
    MITK_WARN << "Cannot parse data / no tools present";
  }

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
    MITK_WARN << "Cannot get a single frame when continuous tracking is on!";
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

  while (i < dwSize)
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

void mitk::PolhemusInterface::SetHemisphereTrackingEnabled(bool _HeisphereTrackingEnabeled)
{
  //HemisphereTracking is switched on by SetSHemiTrack(-1). "-1" means for all sensors.
  //To switch heisphere tracking of, you need to set a hemisphere vector by calling SetSHemisphere(-1, { (float)1,0,0 })
  if (_HeisphereTrackingEnabeled)
  {
    m_pdiDev->SetSHemiTrack(-1);
  }
  //TODO: Logik umbauen. Vermutlich wenn im negativen Bereich -1, sonst 1 setzen?!
  //else if (!m_Hemispheres.empty())
  //{
  //  for (int i = 0; i < m_numberOfTools; ++i)
  //    SetHemisphere(i + 1, m_Hemispheres.at(i));
  //}
  else
  {
    //Default Hemisphere
    mitk::Vector3D temp;
    mitk::FillVector3D(temp, 1, 0, 0);
    m_Hemispheres.clear();
    m_Hemispheres.assign(m_numberOfTools, temp);
    this->SetHemisphere(-1, temp);
    //MITK_INFO << m_pdiDev->GetLastResultStr();
  }
}

void mitk::PolhemusInterface::ToggleHemisphere(int _tool)
{
  BOOL _hemiTrack;
  //-1 == all tools
  if (_tool == -1)
  {
    for (int i = 0; i < m_numberOfTools; ++i)
    {
      //is hemiTrack on?
      m_pdiDev->GetSHemiTrack(i,_hemiTrack);

      m_Hemispheres.at(i)[0] = -m_Hemispheres.at(i)[0];
      m_Hemispheres.at(i)[1] = -m_Hemispheres.at(i)[1];
      m_Hemispheres.at(i)[2] = -m_Hemispheres.at(i)[2];
      SetHemisphere(i, m_Hemispheres.at(i));
      if (_hemiTrack) m_pdiDev->SetSHemiTrack(i);
    }
  }
  else
  {
    //is hemiTrack on?
    m_pdiDev->GetSHemiTrack(_tool, _hemiTrack);

    m_Hemispheres.at(_tool)[0] = -m_Hemispheres.at(_tool)[0];
    m_Hemispheres.at(_tool)[1] = -m_Hemispheres.at(_tool)[1];
    m_Hemispheres.at(_tool)[2] = -m_Hemispheres.at(_tool)[2];
    SetHemisphere(_tool, m_Hemispheres.at(_tool));
    if (_hemiTrack) m_pdiDev->SetSHemiTrack(_tool);
  }



}

void mitk::PolhemusInterface::SetHemisphere(int _tool, mitk::Vector3D _hemisphere)
{
  m_pdiDev->SetSHemisphere(_tool, { (float)_hemisphere[0], (float)_hemisphere[1], (float)_hemisphere[2] });
}

mitk::Vector3D mitk::PolhemusInterface::GetHemisphere(int _tool)
{
  PDI3vec _hemisphere;
  mitk::Vector3D _returnVector;

  //Doesn't work in continuous mode. Don't know why, but so it is... Hence: stop and restart...
  if (m_continousTracking)
  {
    m_continousTracking = false;
    m_pdiDev->StopContPno();

    m_pdiDev->GetSHemisphere(_tool, _hemisphere);
    MITK_DEBUG << "Get Hemisphere: " << m_pdiDev->GetLastResultStr();
    mitk::FillVector3D(_returnVector, _hemisphere[0], _hemisphere[1], _hemisphere[2]);

    m_pdiDev->StartContPno(0);
    m_continousTracking = true;
  }
  else
  {
    m_pdiDev->GetSHemisphere(_tool, _hemisphere);
    MITK_DEBUG << "Get Hemisphere: " << m_pdiDev->GetLastResultStr();
    mitk::FillVector3D(_returnVector, _hemisphere[0], _hemisphere[1], _hemisphere[2]);
  }

  return _returnVector;
}

void mitk::PolhemusInterface::PrintStatus()
{
  MITK_INFO << "Polhemus status: " << this->m_pdiDev->CnxReady();
}