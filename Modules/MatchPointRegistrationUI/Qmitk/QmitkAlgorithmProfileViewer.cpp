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

#include "QmitkAlgorithmProfileViewer.h"

#include <mapAlgorithmProfileHelper.h>
#include <mapConvert.h>

QmitkAlgorithmProfileViewer::QmitkAlgorithmProfileViewer(QWidget *parent) : QWidget(parent)
{
  this->setupUi(this);
}

void QmitkAlgorithmProfileViewer::OnInfoChanged(const map::deployment::DLLInfo *newInfo)
{
  updateInfo(newInfo);
}

std::string AddInfoTableRow(const std::string &name,
                            const ::map::algorithm::profile::ValueListType &values,
                            const std::string &defaultStr = "")
{
  std::stringstream descriptionString;
  descriptionString << "<tr><td><b>" << name << ":</b></td><td>";

  if (!values.empty())
  {
    for (::map::algorithm::profile::ValueListType::const_iterator pos = values.begin(); pos != values.end(); ++pos)
    {
      if (pos != values.begin())
      {
        descriptionString << "<br/>";
      }
      descriptionString << *pos;
    }
  }
  else
  {
    if (defaultStr.empty())
    {
      descriptionString << "<font color='gray'><i>unknown / not defined</i></font>";
    }
    else
    {
      descriptionString << defaultStr;
    }
  }

  descriptionString << "</td></tr>";

  return descriptionString.str();
}

void QmitkAlgorithmProfileViewer::updateInfo(const map::deployment::DLLInfo *newInfo)
{
  if (!newInfo)
  {
    this->m_teAlgorithmDetails->clear();
  }
  else
  {
    std::stringstream descriptionString;

    ::map::structuredData::Element::Pointer algProfile =
      ::map::algorithm::profile::parseProfileString(newInfo->getAlgorithmProfileStr());
    // create description string
    descriptionString << "<h1>Identification</h1><p><b>Namespace:</b> " << newInfo->getAlgorithmUID().getNamespace()
                      << "<br/>"
                      << "<b>Name:</b> " << newInfo->getAlgorithmUID().getName() << "<br/>"
                      << "<b>Version:</b> " << newInfo->getAlgorithmUID().getVersion() << "<br/>"
                      << "<b>BuildTag:</b> " << newInfo->getAlgorithmUID().getBuildTag() << "<br/>"
                      << "<b>Location:</b> " << newInfo->getLibraryFilePath() << "</p>";

    descriptionString << "<h1>Description</h1><p>" << ::map::algorithm::profile::getDescription(algProfile) << "</p>";
    descriptionString << "<h1>Keywords</h1><p>";
    ::map::algorithm::profile::ValueListType keys = ::map::algorithm::profile::getKeywords(algProfile);
    for (::map::algorithm::profile::ValueListType::const_iterator keyPos = keys.begin(); keyPos != keys.end(); ++keyPos)
    {
      if (keyPos != keys.begin())
      {
        descriptionString << "; ";
      }
      descriptionString << *keyPos;
    }
    descriptionString << "</p>";

    descriptionString << "<h1>Characteristics</h1><table>";
    descriptionString << AddInfoTableRow("data type(s)", ::map::algorithm::profile::getDataType(algProfile));
    descriptionString << AddInfoTableRow("computation style",
                                         ::map::algorithm::profile::getComputationStyle(algProfile));
    if (::map::algorithm::profile::isDeterministic(algProfile))
    {
      descriptionString << "<tr><td><b>deterministic:</b></td><td>yes</td></tr>";
    }
    else
    {
      descriptionString << "<tr><td><b>deterministic:</b></td><td>no</td></tr>";
    }
    descriptionString << AddInfoTableRow("resolution style", ::map::algorithm::profile::getResolutionStyle(algProfile));

    unsigned int dimension;

    descriptionString << "<tr><td><b>moving dim:</b></td><td>";
    if (::map::algorithm::profile::getMovingDimensions(algProfile, dimension))
    {
      descriptionString << map::core::convert::toStr(dimension) << "D";
    }
    else
    {
      descriptionString << "<font color='gray'><i>unknown / not defined</i></font>";
    }
    descriptionString << "</td></tr>";
    descriptionString << AddInfoTableRow("moving modality", ::map::algorithm::profile::getMovingModality(algProfile));

    descriptionString << "<tr><td><b>target dim:</b></td><td>";
    if (::map::algorithm::profile::getTargetDimensions(algProfile, dimension))
    {
      descriptionString << map::core::convert::toStr(dimension) << "D";
    }
    else
    {
      descriptionString << "<font color='gray'><i>unknown / not defined</i></font>";
    }
    descriptionString << "</td></tr>";
    descriptionString << AddInfoTableRow("target modality", ::map::algorithm::profile::getTargetModality(algProfile));

    descriptionString << AddInfoTableRow("subject", ::map::algorithm::profile::getSubject(algProfile));
    descriptionString << AddInfoTableRow("object", ::map::algorithm::profile::getObject(algProfile));
    descriptionString << AddInfoTableRow("transform model", ::map::algorithm::profile::getTransformModel(algProfile));
    descriptionString << AddInfoTableRow("transform domain", ::map::algorithm::profile::getTransformDomain(algProfile));
    descriptionString << AddInfoTableRow("metric", ::map::algorithm::profile::getMetric(algProfile));
    descriptionString << AddInfoTableRow("optimization", ::map::algorithm::profile::getOptimization(algProfile));
    descriptionString << AddInfoTableRow("interaction", ::map::algorithm::profile::getInteraction(algProfile));
    descriptionString << "</table>";

    descriptionString << "<h1>Contact & Usage</h1><table>";
    descriptionString << AddInfoTableRow("contact", ::map::algorithm::profile::getContact(algProfile));
    std::string terms;
    descriptionString << "<tr><td><b>terms:</b></td><td>";
    if (::map::algorithm::profile::getTerms(algProfile, terms))
    {
      descriptionString << terms;
    }
    else
    {
      descriptionString << "<font color='gray'><i>unknown / not defined</i></font>";
    }
    descriptionString << "</td></tr>";
    descriptionString << AddInfoTableRow("citation", ::map::algorithm::profile::getCitation(algProfile));
    descriptionString << "</table>";

    // update the info label
    this->m_teAlgorithmDetails->clear();
    this->m_teAlgorithmDetails->insertHtml(QString::fromStdString(descriptionString.str()));
  }
}
