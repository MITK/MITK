/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPolhemusInterface.h>
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
  pdiMDat.Append(PDI_MODATA_DISTLEV);
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

bool mitk::PolhemusInterface::OpenConnection()
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
  return returnValue;
}

bool mitk::PolhemusInterface::Connect()
{
  bool returnValue = OpenConnection();

  if (!returnValue)
  {
    return returnValue;
  }

  m_numberOfTools = this->GetNumberOfTools();

  //Get the tracking data to find out which tools are available.
  std::vector<mitk::PolhemusInterface::trackingData> _trackingData = GetFrame();

  //if we have more/less tools than before, reset all data.
  //check with toolStorage changes is nto enough, 'cause a sensor could just have been unplugged.
  if (m_ToolPorts.size() != _trackingData.size())
  {
    m_ToolPorts.clear();
    m_Hemispheres.clear();
    m_HemisphereTracking.clear();
  }

  //if we have the same number of tools as before, check if they are still the same.
  if (m_ToolPorts.size() == _trackingData.size())
  {
    for (size_t i = 0; i < _trackingData.size(); ++i)
    {
      //if they are not the same, clear hemispheres and toolNames and break.
      if (m_ToolPorts[i] != _trackingData.at(i).id)
      {
        m_ToolPorts.clear();
        m_Hemispheres.clear();
        m_HemisphereTracking.clear();
        break;
      }
    }
  }

  //if we don't have old tool names or if the old ones don't match any more, assign them again.
  if (m_ToolPorts.size() == 0)
  {
    for (size_t i = 0; i < _trackingData.size(); ++i)
    {
      m_ToolPorts.push_back(_trackingData.at(i).id);
    }
    //and reset the hemisphere parameters
    m_Hemispheres.clear();
    m_HemisphereTracking.clear();
    mitk::Vector3D temp;
    mitk::FillVector3D(temp, 1, 0, 0);
    m_Hemispheres.assign(m_numberOfTools, temp);
    m_HemisphereTracking.assign(m_numberOfTools, false);
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

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::AutoDetectTools()
{
  OpenConnection();
  std::vector<mitk::PolhemusInterface::trackingData> frame = GetSingleFrame();
  m_pdiDev->Disconnect();
  return frame;
}

unsigned int mitk::PolhemusInterface::GetNumberOfTools()
{
  std::vector<mitk::PolhemusInterface::trackingData> _trackingData = GetFrame();
  return _trackingData.size();
}

std::vector<mitk::PolhemusInterface::trackingData> mitk::PolhemusInterface::GetFrame()
{
  if (m_continousTracking)
    return this->GetLastFrame();
  else
    return this->GetSingleFrame();
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
    PINT pDistLevel = (PINT)(&pBuf[i + 28]);

    mitk::PolhemusInterface::trackingData currentTrackingData;

    currentTrackingData.id = ucSensor;

    currentTrackingData.pos[0] = pPno[0] * 10; //from cm to mm
    currentTrackingData.pos[1] = pPno[1] * 10;
    currentTrackingData.pos[2] = pPno[2] * 10;

    double azimuthAngle = pPno[3] / 180 * itk::Math::pi; //from degree to rad
    double elevationAngle = pPno[4] / 180 * itk::Math::pi;
    double rollAngle = pPno[5] / 180 * itk::Math::pi;
    vnl_quaternion<double> eulerQuat(rollAngle, elevationAngle, azimuthAngle);
    currentTrackingData.rot = eulerQuat;
    currentTrackingData.distortionLevel = *pDistLevel;

    returnValue.push_back(currentTrackingData);
    i += shSize;
  }
  return returnValue;
}

void mitk::PolhemusInterface::SetHemisphereTrackingEnabled(bool _HemisphereTrackingEnabled, int _tool)
{
  //only if connection is ready!
  if (!this->m_pdiDev->CnxReady())
    return;

  if (m_Hemispheres.empty())
  {
    MITK_ERROR << "No Hemispheres. This should never happen when connected. Check your code!";
  }

  //HemisphereTracking is switched on by SetSHemiTrack(-1). "-1" means for all sensors.
  //To switch hemisphere tracking off, you need to set a hemisphere vector e.g. by calling SetSHemisphere(-1, { (float)1,0,0 })
  if (_HemisphereTrackingEnabled)
  {
    m_pdiDev->SetSHemiTrack(_tool);
    if (_tool != -1)
    {
      m_HemisphereTracking.at(GetToolIndex(_tool)) = true;
    }
    else
    {
      m_HemisphereTracking.assign(m_numberOfTools, true);
    }
  }

  //switch HemiTracking OFF
  else
  {
    //Get Tool Position. ToDo, this should not be the tool tip but the sensor position. Any chance, to get that from Polhemus interface?!
    std::vector<mitk::PolhemusInterface::trackingData> _position = GetFrame();

    for (int index : GetToolIterator(_tool))
    {
      //Scalar product between mitk::point and mitk::vector
      double _scalarProduct = _position.at(index).pos.GetVectorFromOrigin() * m_Hemispheres.at(index);
      //if scalar product is negative, then the tool is in the opposite sphere then when we started to track.
      //Hence, we have to set the inverted hemisphere.
      //For default (1|0|0) this means, if x is negative, we have to set (-1|0|0). But we want to keep it generic if user sets different hemisphere...
      if (_scalarProduct < 0)
      {
        m_Hemispheres.at(index) = -1. * m_Hemispheres.at(index);
      }
      else if (_scalarProduct == 0)
        MITK_ERROR << "Something went wrong. Hemisphere or Position should not be zero.";

      SetHemisphere(m_ToolPorts[index], m_Hemispheres.at(index));
    }
  }
}

void mitk::PolhemusInterface::ToggleHemisphere(int _tool)
{
  //only if connection is ready!
  if (!this->m_pdiDev->CnxReady())
    return;

  //toggle.
  for (int index : GetToolIterator(_tool))
  {
    if (m_HemisphereTracking.at(index))
    {
      SetHemisphereTrackingEnabled(false, m_ToolPorts[index]);
      this->SetHemisphere(m_ToolPorts[index], -1.*m_Hemispheres.at(index));
      SetHemisphereTrackingEnabled(true, m_ToolPorts[index]);
    }
    else
    {
      this->SetHemisphere(m_ToolPorts[index], -1.*m_Hemispheres.at(index));
    }
  }
}

void mitk::PolhemusInterface::AdjustHemisphere(int _tool)
{
  //only if connection is ready!
  if (!this->m_pdiDev->CnxReady())
    return;

  mitk::Vector3D _hemisphere;
  mitk::FillVector3D(_hemisphere, 1, 0, 0);

  for (int index : GetToolIterator(_tool))
  {
    if (m_HemisphereTracking.at(index))
    {
      SetHemisphereTrackingEnabled(false, m_ToolPorts[index]);
      this->SetHemisphere(m_ToolPorts[index], _hemisphere);
      SetHemisphereTrackingEnabled(true, m_ToolPorts[index]);
    }
    else
    {
      this->SetHemisphere(m_ToolPorts[index], _hemisphere);
    }
  }
}

void mitk::PolhemusInterface::SetHemisphere(int _tool, mitk::Vector3D _hemisphere)
{
  //only if connection is ready!
  if (!this->m_pdiDev->CnxReady())
    return;

  m_pdiDev->SetSHemisphere(_tool, { (float)_hemisphere[0], (float)_hemisphere[1], (float)_hemisphere[2] });

  for (int index : GetToolIterator(_tool))
  {
    if (_hemisphere.GetNorm() != 0)
    {
      m_HemisphereTracking.at(index) = false;
      m_Hemispheres.at(index) = _hemisphere;
    }
    else
    {
      m_HemisphereTracking.at(index) = true;
      //don't set the Hemisphere to (0|0|0), as we want to remember the old one.
    }
  }
}

mitk::Vector3D mitk::PolhemusInterface::GetHemisphere(int _tool)
{
  if (_tool == -1)
  {
    MITK_WARN << "Can't return hemisphere for all tools. Returning Hemisphere of first tool " << m_ToolPorts[0];
    return m_Hemispheres.at(0);
  }
  return m_Hemispheres.at(GetToolIndex(_tool));
}

bool mitk::PolhemusInterface::GetHemisphereTrackingEnabled(int _tool)
{
  //if tool is -1, this means "All Tools". We return true if HemiTracking is enabled for all tools, and false if it is off for at least one tool.
  if (_tool == -1)
  {
    bool _returnValue = true;
    for (bool currentValue : m_HemisphereTracking)
      _returnValue = _returnValue && currentValue;
    return _returnValue;
  }
  else
    return m_HemisphereTracking.at(GetToolIndex(_tool));
}

std::vector<int> mitk::PolhemusInterface::GetToolPorts()
{
  return m_ToolPorts;
}

int  mitk::PolhemusInterface::GetToolIndex(int _tool)
{
  if (_tool == -1)
    return -1;
  else
    return std::find(m_ToolPorts.begin(), m_ToolPorts.end(), _tool) - m_ToolPorts.begin();
}

std::vector<int> mitk::PolhemusInterface::GetToolIterator(int _tool)
{
  std::vector<int> _iterator;
  if (_tool == -1)
  {
    for (int i = 0; i < static_cast<int>(m_numberOfTools); ++i)
      _iterator.push_back(i);
  }
  else
  {
    _iterator.push_back(GetToolIndex(_tool));
  }
  return _iterator;
}

void mitk::PolhemusInterface::PrintStatus()
{
  MITK_INFO << "Polhemus status: " << this->m_pdiDev->CnxReady();
}
