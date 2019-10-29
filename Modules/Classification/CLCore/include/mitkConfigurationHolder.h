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


#ifndef mitkConfigurationHolder_h
#define mitkConfigurationHolder_h

#include <MitkCLCoreExports.h>

//#include <mitkBaseData.h>

// STD Includes
#include <string>
#include <map>
#include <vector>

namespace mitk
{
  class MITKCLCORE_EXPORT ConfigurationHolder // : public BaseData
  {
  public:
    enum ValueType
    {
      DT_UNINIZIALIZED,
      DT_BOOL,
      DT_UINT,
      DT_INT,
      DT_DOUBLE,
      DT_STRING,
      DT_GROUP
    };
    ConfigurationHolder();


    void SetBool(bool value);
    void SetUnsignedInt(unsigned int value);
    void SetInt(int value);
    void SetDouble(double value);
    void SetString(std::string value);

    void ClearGroup();
    void AddToGroup(std::string id, const ConfigurationHolder &value);

    bool AsBool();
    unsigned int AsUnsignedInt();
    int AsInt();
    double AsDouble();
    std::string AsString();

    bool AsBool(bool value);
    unsigned int AsUnsignedInt(unsigned int value);
    int AsInt(int value);
    double AsDouble(double value);
    std::string AsString(std::string value);

    std::vector<std::string> AsStringVector();

    ConfigurationHolder& At(std::string id);

  private:
    bool m_BoolValue;
    unsigned int m_UIntValue;
    int m_IntValue;
    double m_DoubleValue;
    std::string m_StringValue;

    std::map<std::string, ConfigurationHolder> m_GroupValue;

    ValueType m_ValueType;
  };
}

#endif //mitkConfigurationHolder_h
