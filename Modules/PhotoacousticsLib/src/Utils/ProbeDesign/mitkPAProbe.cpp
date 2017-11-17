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

#include "mitkPAProbe.h"

mitk::pa::Probe::Probe(std::string file, bool verbose) :
  m_TotalEnergy(0),
  m_Verbose(verbose)
{
  TiXmlDocument xmlDoc(file);
  bool success = xmlDoc.LoadFile(TIXML_ENCODING_UTF8);
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
  TiXmlDocument xmlDoc;
  const char* success = xmlDoc.Parse(fileStream, nullptr, TIXML_ENCODING_UTF8);
  if (m_Verbose)
  {
    std::cout << "reading document was " << (success == nullptr ? "" : "not ") << "successful (" << (success == nullptr ? "NULL" : success) << ")" << std::endl;
    std::cout << "Content of the Xml file:" << std::endl;
    xmlDoc.Print();
  }
  if (success == nullptr || atoi(success) == 0)
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

void mitk::pa::Probe::InitProbe(TiXmlDocument xmlDoc)
{
  m_IsValid = true;

  TiXmlElement* root = xmlDoc.FirstChildElement(XML_TAG_PROBE);
  if (root)
  {
    for (TiXmlElement* element = root->FirstChildElement(XML_TAG_LIGHT_SOURCE);
      element != nullptr; element = element->NextSiblingElement(XML_TAG_LIGHT_SOURCE))
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
