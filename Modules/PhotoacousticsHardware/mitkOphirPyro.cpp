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

#include "mitkOphirPyro.h"

#include <chrono>
#include <thread>

#include "gclibo.h"

mitk::OphirPyro::OphirPyro() :
m_CurrentWavelength(0)
{
}

mitk::OphirPyro::~OphirPyro()
{
  this->CloseConnection();
  MITK_INFO << "[OphirPyro Debug] destroyed that Pyro";
}

bool mitk::OphirPyro::AcquireData()
{

  return false;
}

double mitk::OphirPyro::GetNextPulseEnergy()
{

  return 0;
}

bool mitk::OphirPyro::OpenConnection()
{
  //todo

  return false;
}

bool mitk::OphirPyro::CloseConnection()
{
  return false;
}