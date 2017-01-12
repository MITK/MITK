#include <mitkCLResultWritter.h>

mitk::cl::FeatureResultWritter::FeatureResultWritter(std::string file, int mode) :
    m_Mode(mode),
    m_CurrentRow(0),
    m_CurrentElement(0),
    m_Separator(";")
{
  std::string str;
  m_List.push_back(str);

  m_Output.open(file, std::ios::app);
}

mitk::cl::FeatureResultWritter::~FeatureResultWritter()
{
  for (std::size_t i = 0; i < m_List.size()-1; ++i)
  {
    m_Output << m_List[i] << std::endl;
  }
  //m_Output << "EndOfFile" << std::endl;
  m_Output.close();
}


void mitk::cl::FeatureResultWritter::AddColumn(double value) {
  std::ostringstream  ss;
  ss << value;
  AddColumn(ss.str());
}

void mitk::cl::FeatureResultWritter::AddColumn(std::string value) {
  if (m_Mode == 0)
  {
    m_List[m_CurrentRow] = m_List[m_CurrentRow] + value + m_Separator;
  }
  else
  {
    m_CurrentRow++;
    while (m_List.size() <= m_CurrentRow)
    {
      std::string str;
      m_List.push_back(str);
    }
    m_List[m_CurrentRow] = m_List[m_CurrentRow] + value + m_Separator;
  }
}


void mitk::cl::FeatureResultWritter::NewRow(std::string endName) {
  AddColumn(endName);
  if (m_Mode == 0)
  {
    m_CurrentRow++;
    while (m_List.size() <= m_CurrentRow)
    {
      std::string str;
      m_List.push_back(str);
    }
  }
  else
  {
    m_CurrentRow = 0;
  }
}

void mitk::cl::FeatureResultWritter::AddResult(std::string desc, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool withHeader, bool withDescription)
{
  if ((withHeader) && (m_CurrentElement == 0))
  {
    if (withDescription)
    {
      AddColumn("Description");
    }
    if (slice >= 0)
    {
      AddColumn("SliceNumber");
    }
    for (std::size_t i = 0; i < stats.size(); ++i)
    {
      AddColumn(stats[i].first);
    }
    NewRow("EndOfMeasurement");
  }

  if (withDescription)
  {
    AddColumn(desc);
  }
  if (slice >= 0)
  {
    AddColumn(slice);
  }
  for (std::size_t i = 0; i < stats.size(); ++i)
  {
    AddColumn(stats[i].second);
  }
  NewRow("EndOfMeasurement");
  ++m_CurrentElement;
}


