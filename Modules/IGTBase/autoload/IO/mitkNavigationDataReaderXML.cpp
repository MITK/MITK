/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkNavigationDataReaderXML.h"
#include <mitkIGTMimeTypes.h>
#include <mitkLocaleSwitch.h>

// Third Party
#include <itksys/SystemTools.hxx>
#include <fstream>
#include <tinyxml2.h>

namespace
{
  mitk::NavigationData::Pointer ReadNavigationData(const tinyxml2::XMLElement* ndElem)
  {
    if (nullptr == ndElem)
      return nullptr;

    mitk::NavigationData::TimeStampType timeStamp = -1;

    ndElem->QueryDoubleAttribute("Time", &timeStamp);

    if (-1 == timeStamp)
      return nullptr;

    mitk::NavigationData::PositionType position;
    position.Fill(0.0);

    ndElem->QueryDoubleAttribute("X", &position[0]);
    ndElem->QueryDoubleAttribute("Y", &position[1]);
    ndElem->QueryDoubleAttribute("Z", &position[2]);

    mitk::NavigationData::OrientationType orientation;

    ndElem->QueryDoubleAttribute("QX", &orientation[0]);
    ndElem->QueryDoubleAttribute("QY", &orientation[1]);
    ndElem->QueryDoubleAttribute("QZ", &orientation[2]);
    ndElem->QueryDoubleAttribute("QR", &orientation[3]);

    mitk::NavigationData::CovarianceMatrixType matrix;
    matrix.SetIdentity();

    ndElem->QueryDoubleAttribute("C00", &matrix[0][0]);
    ndElem->QueryDoubleAttribute("C01", &matrix[0][1]);
    ndElem->QueryDoubleAttribute("C02", &matrix[0][2]);
    ndElem->QueryDoubleAttribute("C03", &matrix[0][3]);
    ndElem->QueryDoubleAttribute("C04", &matrix[0][4]);
    ndElem->QueryDoubleAttribute("C05", &matrix[0][5]);
    ndElem->QueryDoubleAttribute("C10", &matrix[1][0]);
    ndElem->QueryDoubleAttribute("C11", &matrix[1][1]);
    ndElem->QueryDoubleAttribute("C12", &matrix[1][2]);
    ndElem->QueryDoubleAttribute("C13", &matrix[1][3]);
    ndElem->QueryDoubleAttribute("C14", &matrix[1][4]);
    ndElem->QueryDoubleAttribute("C15", &matrix[1][5]);

    int attrib = 0;
    ndElem->QueryIntAttribute("Valid", &attrib);
    bool isValid = 0 != attrib;

    attrib = 0;
    ndElem->QueryIntAttribute("hP", &attrib);
    bool hasPosition = 0 != attrib;

    attrib = 0;
    ndElem->QueryIntAttribute("hO", &attrib);
    bool hasOrientation = 0 != attrib;

    auto navData = mitk::NavigationData::New();

    navData->SetIGTTimeStamp(timeStamp);
    navData->SetPosition(position);
    navData->SetOrientation(orientation);
    navData->SetCovErrorMatrix(matrix);
    navData->SetDataValid(isValid);
    navData->SetHasPosition(hasPosition);
    navData->SetHasOrientation(hasOrientation);

    return navData;
  }
}

mitk::NavigationDataReaderXML::NavigationDataReaderXML()
  : AbstractFileReader(IGTMimeTypes::NAVIGATIONDATASETXML_MIMETYPE(), "MITK NavigationData Reader (XML)")
{
  this->RegisterService();
}

mitk::NavigationDataReaderXML::~NavigationDataReaderXML()
{
}

mitk::NavigationDataReaderXML::NavigationDataReaderXML(const mitk::NavigationDataReaderXML& other)
  : AbstractFileReader(other)
{
}

mitk::NavigationDataReaderXML* mitk::NavigationDataReaderXML::Clone() const
{
  return new NavigationDataReaderXML(*this);
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::NavigationDataReaderXML::DoRead()
{
  mitk::NavigationDataSet::Pointer dataset = nullptr == this->GetInputStream()
    ? this->Read(this->GetInputLocation())
    : this->Read(*this->GetInputStream());

  std::vector<mitk::BaseData::Pointer> result;
  result.emplace_back(dataset.GetPointer());

  return result;
}

mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderXML::Read(const std::string& fileName)
{
  std::ifstream stream(fileName);
  stream.imbue(std::locale::classic());

  return this->Read(stream);
}

mitk::NavigationDataSet::Pointer mitk::NavigationDataReaderXML::Read(std::istream& stream)
{
  std::string string(std::istreambuf_iterator<char>(stream), {});
  tinyxml2::XMLDocument doc;

  if (tinyxml2::XML_SUCCESS != doc.Parse(string.c_str()))
    mitkThrowException(IGTIOException) << "Could not parse stream.";

  const auto* rootElem = doc.RootElement();
  decltype(rootElem) dataElem = nullptr;

  if (nullptr == rootElem)
    return nullptr;

  int version = 0;
  auto err = tinyxml2::XML_SUCCESS;
    
  std::string rootElemVal = rootElem->Value();

  if ("Version" == rootElemVal)
  {
    err = rootElem->QueryIntAttribute("Ver", &version);
    dataElem = rootElem->NextSiblingElement("Data");
  }
  else if ("Data" == rootElemVal)
  {
    err = rootElem->QueryIntAttribute("version", &version);
    dataElem = rootElem;
  }

  if (err != tinyxml2::XML_SUCCESS)
    mitkThrowException(IGTIOException) << "Could not parse file format version.";

  if (version != 1)
    mitkThrowException(IGTIOException) << "File format version " << version << " is not supported.";

  if (nullptr == dataElem)
    mitkThrowException(IGTIOException) << "Data element not found.";

  int toolCount = 0;

  if (tinyxml2::XML_SUCCESS != dataElem->QueryIntAttribute("ToolCount", &toolCount))
    mitkThrowException(IGTIOException) << "ToolCount attribute missing from Data element.";

  if (0 >= toolCount)
    mitkThrowException(IGTIOException) << "Invalid ToolCount: " << toolCount << ".";

  auto navDataSet = NavigationDataSet::New(static_cast<unsigned int>(toolCount));
  NavigationData::Pointer navData;

  const auto* ndElem = dataElem->FirstChildElement();

  if (nullptr != ndElem)
  {
    do
    {
      std::vector<NavigationData::Pointer> navDatas(toolCount);

      for (decltype(toolCount) i = 0; i < toolCount; ++i)
      {
        navData = ReadNavigationData(ndElem);

        if (navData.IsNull())
        {
          if (0 != i)
            MITK_WARN("mitkNavigationDataReaderXML") << "Different number of NavigationData objects for different tools. Ignoring last ones.";

          break;
        }

        navDatas[i] = navData;
        ndElem = ndElem->NextSiblingElement();
      }

      if (navData.IsNotNull())
        navDataSet->AddNavigationDatas(navDatas);

    } while (nullptr != ndElem && navData.IsNotNull());
  }

  return navDataSet;
}
