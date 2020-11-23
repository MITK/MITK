/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPAProbe.h"
#include <tinyxml2.h>

mitk::pa::Probe::Probe(std::string file, bool verbose) :
  m_TotalEnergy(0),
  m_Verbose(verbose)
{
  tinyxml2::XMLDocument xmlDoc;
  bool success = xmlDoc.LoadFile(file.c_str()) == tinyxml2::XML_SUCCESS;
  if (m_Verbose)
  {
    std::cout << "reading of document was " << (success ? "" : "not ") << "successful" << std::endl;
    std::cout << "Content of the Xml file:" << std::endl;
    xmlDoc.Print();
  }
  if (success)
  {
    InitProbe(xmlDoc);
  }
  else
  {
    if (m_Verbose)
    {
      std::cerr << "Could not load xml file" << std::endl;
    }
  }
}

mitk::pa::Probe::Probe(const char* fileStream, bool verbose) :
  m_TotalEnergy(0),
  m_Verbose(verbose)
{
  tinyxml2::XMLDocument xmlDoc;
  auto success = xmlDoc.Parse(fileStream);
  if (m_Verbose)
  {
    std::cout << "reading document was " << (success == tinyxml2::XML_SUCCESS ? "" : "not ") << "successful" << std::endl;
    std::cout << "Content of the Xml file:" << std::endl;
    xmlDoc.Print();
  }
  if (success == tinyxml2::XML_SUCCESS)
  {
    InitProbe(xmlDoc);
  }
  else
  {
    if (m_Verbose)
    {
      std::cerr << "Could not load xml file" << std::endl;
    }
  }
}

mitk::pa::Probe::~Probe()
{
}

mitk::pa::LightSource::PhotonInformation mitk::pa::Probe::GetNextPhoton(double rng1, double rnd2,
  double rnd3, double rnd4, double rnd5,
  double rnd6, double rnd7, double rnd8)
{
  rng1 = rng1*m_TotalEnergy;
  double currentEnergy = 0;
  for (LightSource::Pointer lightSource : m_LightSources)
  {
    currentEnergy += lightSource->GetEnergy();
    if (currentEnergy >= rng1)
      return lightSource->GetNextPhoton(rnd2, rnd3, rnd4, rnd5, rnd6, rnd7, rnd8);
  }

  //Last resort: If something goes wrong, return a position from the first source.
  return m_LightSources[0]->GetNextPhoton(rnd2, rnd3, rnd4, rnd5, rnd6, rnd7, rnd8);
}

bool mitk::pa::Probe::IsValid()
{
  return m_IsValid;
}

void mitk::pa::Probe::InitProbe(const tinyxml2::XMLDocument& xmlDoc)
{
  m_IsValid = true;

  auto* root = xmlDoc.FirstChildElement(XML_TAG_PROBE.c_str());
  if (root)
  {
    for (auto* element = root->FirstChildElement(XML_TAG_LIGHT_SOURCE.c_str());
      element != nullptr; element = element->NextSiblingElement(XML_TAG_LIGHT_SOURCE.c_str()))
    {
      LightSource::Pointer lightSource = LightSource::New(element, m_Verbose);
      if (lightSource.IsNotNull() && lightSource->IsValid())
      {
        m_LightSources.push_back(lightSource);
        m_TotalEnergy += lightSource->GetEnergy();
      }
      else
      {
        m_IsValid = false;
      }
    }
  }
  else
  {
    m_IsValid = false;
  }

  if (!m_IsValid)
  {
    std::cerr << "Creation of a valid Photoacoustic Probe failed." << std::endl;
  }
  else
  {
    if (m_Verbose)
    {
      std::cout << "Successfully created Photoacoustic Probe." << std::endl;
    }
  }
}
