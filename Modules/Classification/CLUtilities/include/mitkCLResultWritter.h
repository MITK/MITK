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

#ifndef mitkCLResultWritter_h
#define mitkCLResultWritter_h

#include "MitkCLUtilitiesExports.h"

#include <sstream>
#include <fstream>
#include <vector>

#include <mitkAbstractGlobalImageFeature.h>

namespace mitk
{
  namespace cl
  {
    class MITKCLUTILITIES_EXPORT FeatureResultWritter
    {
    public:
      FeatureResultWritter(std::string, int mode);
      ~FeatureResultWritter();

      void SetDecimalPoint(char decimal);

      void AddSubjectInformation(std::string value);
      void AddColumn(std::string value);
      void AddColumn(double value);
      void NewRow(std::string endName);

      void AddResult(std::string desc, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool , bool withDescription);
      void AddHeader(std::string, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool withHeader, bool withDescription);

    private:
      int m_Mode;
      std::size_t m_CurrentRow;
      int m_CurrentElement;
      std::string m_Separator;
      std::ofstream m_Output;
      std::vector<std::string> m_List;
      std::string m_SubjectInformation;
      bool m_UsedSubjectInformation;
      bool m_UseSpecialDecimalPoint;
      char m_DecimalPoint;
    };
  }
}

#endif //mitkCLResultWritter_h