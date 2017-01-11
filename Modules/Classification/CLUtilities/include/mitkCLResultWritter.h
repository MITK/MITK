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

      void AddColumn(std::string value);
      void AddColumn(double value);
      void NewRow(std::string endName);

      void AddResult(std::string desc, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool withHeader);

    private:
      int m_Mode;
      int m_CurrentRow;
      int m_CurrentElement;
      std::string m_Separator;
      std::ofstream m_Output;
      std::vector<std::string> m_List;
    };
  }
}

#endif mitkCLResultWritter_h