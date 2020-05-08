/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCLResultWriter_h
#define mitkCLResultWriter_h

#include "MitkCLUtilitiesExports.h"

#include <sstream>
#include <fstream>
#include <vector>

#include <mitkAbstractGlobalImageFeature.h>

namespace mitk
{
  namespace cl
  {
    class MITKCLUTILITIES_EXPORT FeatureResultWriter
    {
    public:
      FeatureResultWriter(std::string, int mode);
      ~FeatureResultWriter();

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
