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

#include "mitkPALightSource.h"
#include <cmath>

mitk::pa::LightSource::LightSource() :
  m_IsValid(false)
{
}

mitk::pa::LightSource::LightSource(TiXmlElement* element, bool verbose) :
  m_IsValid(true),
  m_Verbose(verbose)
{
  ParseEnergy(element);
  ParsePhotonSpawnArea(element);
  ParsePhotonDirection(element);

  if (m_IsValid)
  {
    if (m_Verbose)
      std::cout << "Successfully created LightSource" << std::endl;
  }
  else
  {
    if (m_Verbose)
      std::cout << "Failed creating LightSource." << std::endl;
  }
}

mitk::pa::LightSource::~LightSource()
{
}

mitk::pa::LightSource::TransformResult mitk::pa::LightSource::BoxMuellerTransform(double u1, double u2, double mu, double sigma)
{
  TransformResult result;
  result.z0 = sqrt(-2.0 * log(u1)) * cos(TWO_PI * u2) * sigma + mu;
  result.z1 = sqrt(-2.0 * log(u1)) * sin(TWO_PI * u2) * sigma + mu;
  return result;
}

void mitk::pa::LightSource::ParsePhotonDirection(TiXmlElement* element)
{
  TiXmlElement* direction = element->FirstChildElement(XML_TAG_PHOTON_DIRECTION);
  if (direction)
  {
    ParseAngle(direction, XML_TAG_X_ANGLE);
  }
  else
  {
    if (m_Verbose)
      std::cerr << "No \"" << XML_TAG_X_ANGLE << "\" field in xml. Setting to default (0, 0, UNIFORM)." << std::endl;

    m_AngleXMinimum = 0;
    m_AngleXMaximum = 0;
    m_AngleXMode = DistributionMode::UNIFORM;
  }

  direction = element->FirstChildElement(XML_TAG_PHOTON_DIRECTION);
  if (direction)
  {
    ParseAngle(direction, XML_TAG_Y_ANGLE);
  }
  else
  {
    if (m_Verbose)
      std::cerr << "No \"" << XML_TAG_Y_ANGLE << "\" field in xml. Setting to default (0, 0, UNIFORM)." << std::endl;

    m_AngleYMinimum = 0;
    m_AngleYMaximum = 0;
    m_AngleYMode = DistributionMode::UNIFORM;
  }
}

void mitk::pa::LightSource::ParseAngle(TiXmlElement* direction, std::string angle)
{
  double minimum;
  double maximum;
  DistributionMode mode = DistributionMode::GAUSSIAN;

  if (m_Verbose)
    std::cout << "Parsing " << angle << std::endl;
  TiXmlElement* angleElement = direction->FirstChildElement(angle);
  if (angleElement)
  {
    TiXmlElement* angleMin = angleElement->FirstChildElement(XML_TAG_MINIMUM);
    if (angleMin)
    {
      std::string angleMinText = angleMin->GetText();
      minimum = std::stod(angleMinText);
      if (m_Verbose)
        std::cout << "Setting min=" << minimum << std::endl;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_MINIMUM << "\" tag in xml. Setting min=0" << std::endl;
      minimum = 0;
    }

    TiXmlElement* angleMax = angleElement->FirstChildElement(XML_TAG_MAXIMUM);
    if (angleMax)
    {
      std::string angleMaxText = angleMax->GetText();
      maximum = std::stod(angleMaxText);
      if (m_Verbose)
        std::cout << "Setting max=" << maximum << std::endl;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_MAXIMUM << "\" tag in xml. Setting max=0" << std::endl;
      maximum = 0;
    }

    TiXmlElement* angleMode = angleElement->FirstChildElement(XML_TAG_MODE);
    if (angleMode)
    {
      std::string angleModeText = angleMode->GetText();
      if (strcmp("UNIFORM", angleModeText.c_str()) == 0)
      {
        mode = DistributionMode::UNIFORM;
        if (m_Verbose)
          std::cout << "Setting mode=UNIFORM" << std::endl;
      }
      else if (strcmp("GAUSSIAN", angleModeText.c_str()) == 0)
      {
        mode = DistributionMode::GAUSSIAN;
        if (m_Verbose)
          std::cout << "Setting mode=GAUSSIAN" << std::endl;
      }
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_MODE << "\" tag in xml. Setting mode=UNIFORM" << std::endl;
      mode = DistributionMode::UNIFORM;
    }
  }
  else
  {
    if (m_Verbose)
      std::cerr << "No \"" << angle << "\" field in xml. Setting to default (0, 0, UNIFORM)." << std::endl;

    maximum = 0;
    minimum = 0;
    mode = DistributionMode::UNIFORM;
  }

  if (strcmp(XML_TAG_X_ANGLE.c_str(), angle.c_str()) == 0)
  {
    m_AngleXMinimum = minimum;
    m_AngleXMaximum = maximum;
    m_AngleXMode = mode;
  }
  else if (strcmp(XML_TAG_Y_ANGLE.c_str(), angle.c_str()) == 0)
  {
    m_AngleYMinimum = minimum;
    m_AngleYMaximum = maximum;
    m_AngleYMode = mode;
  }
}

void mitk::pa::LightSource::ParseEnergy(TiXmlElement* element)
{
  TiXmlElement* energy = element->FirstChildElement(XML_TAG_ENERGY);
  if (energy)
  {
    std::string energyText = energy->GetText();
    m_Energy = std::stod(energyText);
    if (m_Verbose)
      std::cout << "Setting energy=" << m_Energy;
  }
  else
  {
    if (m_Verbose)
      std::cerr << "No \"" << XML_TAG_ENERGY << "\" field in xml. Setting Energy=1" << std::endl;
    m_Energy = 1.0;
  }
}

void mitk::pa::LightSource::ParsePhotonSpawnArea(TiXmlElement* element)
{
  TiXmlElement* spawnArea = element->FirstChildElement("PhotonSpawnArea");
  if (spawnArea)
  {
    TiXmlElement* spawnType = spawnArea->FirstChildElement(XML_TAG_SPAWN_TYPE);
    if (spawnType)
    {
      std::string spawnTypeText = spawnType->GetText();
      if (strcmp(XML_TAG_SPAWN_TYPE_POINT.c_str(), spawnTypeText.c_str()) == 0)
      {
        m_SpawnType = SpawnType::POINT;
        if (m_Verbose)
          std::cout << "Setting " << XML_TAG_SPAWN_TYPE << " = " << XML_TAG_SPAWN_TYPE_POINT << std::endl;
      }
      else if (strcmp(XML_TAG_SPAWN_TYPE_RECTANGLE.c_str(), spawnTypeText.c_str()) == 0)
      {
        m_SpawnType = SpawnType::RECTANGLE;
        if (m_Verbose)
          std::cout << "Setting " << XML_TAG_SPAWN_TYPE << " = " << XML_TAG_SPAWN_TYPE_RECTANGLE << std::endl;
      }
      else if (strcmp(XML_TAG_SPAWN_TYPE_CIRCLE.c_str(), spawnTypeText.c_str()) == 0)
      {
        m_SpawnType = SpawnType::CIRCLE;
        if (m_Verbose)
          std::cout << "Setting " << XML_TAG_SPAWN_TYPE << " = " << XML_TAG_SPAWN_TYPE_CIRCLE << std::endl;
      }
      else
      {
        std::cerr << "The provided SpawnType (" << spawnTypeText << ") did not match any available spawn type. Light source is not valid." << std::endl;
        m_IsValid = false;
      }
    }
    else
    {
      std::cerr << "The \"" << XML_TAG_SPAWN_TYPE << "\" element was not provided for this light source. Light source is not valid." << std::endl;
      m_IsValid = false;
    }

    TiXmlElement* xLocation = spawnArea->FirstChildElement(XML_TAG_X);
    if (xLocation)
    {
      std::string xLocationText = xLocation->GetText();
      m_SpawnLocationX = std::stod(xLocationText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_X << "=" << m_SpawnLocationX;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_X << "\" field in xml. Setting " << XML_TAG_X << "=0" << std::endl;
      m_SpawnLocationX = 0;
    }

    TiXmlElement* yLocation = spawnArea->FirstChildElement(XML_TAG_Y);
    if (yLocation)
    {
      std::string yLocationText = yLocation->GetText();
      m_SpawnLocationY = std::stod(yLocationText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_Y << "=" << m_SpawnLocationY;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_Y << "\" field in xml. Setting " << XML_TAG_Y << "=0" << std::endl;
      m_SpawnLocationY = 0;
    }

    TiXmlElement* zLocation = spawnArea->FirstChildElement(XML_TAG_Z);
    if (zLocation)
    {
      std::string zLocationText = zLocation->GetText();
      m_SpawnLocationZ = std::stod(zLocationText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_Z << "=" << m_SpawnLocationZ;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_Z << "\" field in xml. Setting " << XML_TAG_Z << "=0.1" << std::endl;
      m_SpawnLocationZ = 0.1;
    }

    TiXmlElement* rLocation = spawnArea->FirstChildElement(XML_TAG_R);
    if (rLocation)
    {
      std::string rLocationText = rLocation->GetText();
      m_SpawnLocationRadius = std::stod(rLocationText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_R << "=" << m_SpawnLocationRadius;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_R << "\" field in xml. Setting " << XML_TAG_R << "=0" << std::endl;
      m_SpawnLocationRadius = 0;
    }

    TiXmlElement* xLength = spawnArea->FirstChildElement(XML_TAG_X_LENGTH);
    if (xLength)
    {
      std::string xLengthText = xLength->GetText();
      m_SpawnLocationXLength = std::stod(xLengthText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_X_LENGTH << "=" << m_SpawnLocationXLength << std::endl;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_X_LENGTH << "\" field in xml. Setting " << XML_TAG_X_LENGTH << "=0" << std::endl;
      m_SpawnLocationXLength = 0;
    }

    TiXmlElement* yLength = spawnArea->FirstChildElement(XML_TAG_Y_LENGTH);
    if (yLength)
    {
      std::string yLengthText = yLength->GetText();
      m_SpawnLocationYLength = std::stod(yLengthText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_Y_LENGTH << "=" << m_SpawnLocationYLength << std::endl;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_Y_LENGTH << "\" field in xml. Setting " << XML_TAG_Y_LENGTH << "=0" << std::endl;
      m_SpawnLocationYLength = 0;
    }

    TiXmlElement* zLength = spawnArea->FirstChildElement(XML_TAG_Z_LENGTH);
    if (zLength)
    {
      std::string zLengthText = zLength->GetText();
      m_SpawnLocationZLength = std::stod(zLengthText);
      if (m_Verbose)
        std::cout << "Setting " << XML_TAG_Z_LENGTH << "=" << m_SpawnLocationZLength << std::endl;
    }
    else
    {
      if (m_Verbose)
        std::cerr << "No \"" << XML_TAG_Z_LENGTH << "\" field in xml. Setting " << XML_TAG_Z_LENGTH << "=0" << std::endl;
      m_SpawnLocationZLength = 0;
    }
  }
  else
    m_IsValid = false;
}

mitk::pa::LightSource::PhotonInformation mitk::pa::LightSource::GetNextPhoton(double rnd1, double rnd2, double rnd3,
  double rnd4, double rnd5, double gau1, double gau2)
{
  PhotonInformation returnValue;

  switch (m_SpawnType)
  {
  case POINT:
    returnValue.xPosition = m_SpawnLocationX;
    returnValue.yPosition = m_SpawnLocationY;
    returnValue.zPosition = m_SpawnLocationZ;
    break;
  case RECTANGLE:
    returnValue.xPosition = m_SpawnLocationX + rnd3 * m_SpawnLocationXLength;
    returnValue.yPosition = m_SpawnLocationY + rnd4 * m_SpawnLocationYLength;
    returnValue.zPosition = m_SpawnLocationZ + rnd5 * m_SpawnLocationZLength;
    break;
  case CIRCLE:
    double radius = rnd3 * m_SpawnLocationRadius;
    double angle = rnd4 * TWO_PI;

    returnValue.xPosition = m_SpawnLocationX + radius * cos(angle);
    returnValue.yPosition = m_SpawnLocationY + radius * sin(angle);
    returnValue.zPosition = m_SpawnLocationZ;
    break;
  }

  switch (m_AngleXMode)
  {
  case UNIFORM:
    returnValue.xAngle = rnd1 * (m_AngleXMaximum - m_AngleXMinimum) + m_AngleXMinimum;
    break;
  case GAUSSIAN:
    TransformResult trans = BoxMuellerTransform(gau1, gau2, (m_AngleXMaximum - m_AngleXMinimum) / 2 + m_AngleXMinimum, (m_AngleXMaximum - m_AngleXMinimum) / 2.355);
    returnValue.xAngle = trans.z0;
    break;
  }

  switch (m_AngleYMode)
  {
  case UNIFORM:
    returnValue.yAngle = rnd2 * (m_AngleYMaximum - m_AngleYMinimum) + m_AngleYMinimum;
    break;
  case GAUSSIAN:
    TransformResult trans = BoxMuellerTransform(gau1, gau2, (m_AngleYMaximum - m_AngleYMinimum) / 2 + m_AngleYMinimum, (m_AngleYMaximum - m_AngleYMinimum) / 2.355);
    returnValue.yAngle = trans.z1;
    break;
  }

  if ((returnValue.xAngle*returnValue.xAngle + returnValue.yAngle*returnValue.yAngle) > 1)
  {
    double unify = sqrt(returnValue.xAngle*returnValue.xAngle + returnValue.yAngle*returnValue.yAngle)*1.001;
    returnValue.xAngle = returnValue.xAngle / unify;
    returnValue.yAngle = returnValue.yAngle / unify;
  }

  returnValue.zAngle = sqrt(1 - returnValue.xAngle*returnValue.xAngle - returnValue.yAngle*returnValue.yAngle);

  if (m_Verbose)
    std::cout << "Created a new photon at (" << returnValue.xPosition << "|" << returnValue.yPosition << "|" <<
    returnValue.zPosition << ") with angle (" << returnValue.xAngle << "|" << returnValue.yAngle << "|" <<
    returnValue.zAngle << ")" << std::endl;

  return returnValue;
}

bool mitk::pa::LightSource::IsValid()
{
  return m_IsValid;
}
