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

#include <mitkCLResultWritter.h>

#include <iostream>
#include <locale>

template <class charT>
class punct_facet : public std::numpunct<charT> {
public:
  punct_facet(charT sep) :
    m_Sep(sep)
  {

  }
protected:
  charT do_decimal_point() const { return m_Sep; }
private:
  charT m_Sep;
};

mitk::cl::FeatureResultWritter::FeatureResultWritter(std::string file, int mode) :
m_Mode(mode),
m_CurrentRow(0),
m_CurrentElement(0),
m_Separator(";"),
m_SubjectInformation(""),
m_UsedSubjectInformation(false),
m_UseSpecialDecimalPoint(false),
m_DecimalPoint('.')
{
  std::string str;
  m_List.push_back(str);

  m_Output.open(file, std::ios::app);
}

mitk::cl::FeatureResultWritter::~FeatureResultWritter()
{
  for (std::size_t i = 0; i < m_List.size() - 1; ++i)
  {
    m_Output << m_List[i] << std::endl;
  }
  //m_Output << "EndOfFile" << std::endl;
  m_Output.close();
}

void mitk::cl::FeatureResultWritter::SetDecimalPoint(char decimal)
{
  m_Output.imbue(std::locale(std::cout.getloc(), new punct_facet<char>(decimal)));
  m_UseSpecialDecimalPoint = true;
  m_DecimalPoint = decimal;
}

void mitk::cl::FeatureResultWritter::AddColumn(double value) {
  std::ostringstream  ss;
  if (m_UseSpecialDecimalPoint)
  {
    ss.imbue(std::locale(std::cout.getloc(), new punct_facet<char>(m_DecimalPoint)));
  }
  ss << value;
  AddColumn(ss.str());
}

void mitk::cl::FeatureResultWritter::AddSubjectInformation(std::string value) {
  if ((m_Mode == 0) || (m_Mode == 1))
  {
    AddColumn(value);
  }
  else
  {
    if (m_UsedSubjectInformation)
    {
      m_SubjectInformation = "";
    }
    m_SubjectInformation += value + m_Separator;
    m_UsedSubjectInformation = false;
  }
}

void mitk::cl::FeatureResultWritter::AddColumn(std::string value) {
  if ((m_Mode == 0) || (m_Mode == 2))
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
  if ((m_Mode == 0) || (m_Mode == 2))
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

void mitk::cl::FeatureResultWritter::AddResult(std::string desc, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool, bool withDescription)
{
  if (m_Mode == 2)
  {
    for (std::size_t i = 0; i < stats.size(); ++i)
    {
      if (withDescription)
      {
        AddColumn(desc);
      }
      if (slice >= 0)
      {
        AddColumn(slice);
      }
      AddColumn(m_SubjectInformation + stats[i].first);
      AddColumn(stats[i].second);
      NewRow("");
      ++m_CurrentElement;
    }
    m_UsedSubjectInformation = true;
  }
  else
  {
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
}

void mitk::cl::FeatureResultWritter::AddHeader(std::string, int slice, mitk::AbstractGlobalImageFeature::FeatureListType stats, bool withHeader, bool withDescription)
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
}