/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>
#include <fstream>
#include <ios>
#include "mitkTemplateReportWriter.h"

//#include <qregexp.h>
 

mitk::TemplateReportWriter::TemplateReportWriter()
: m_OutputFileName(""), m_TemplateFileName("")
{
}


void mitk::TemplateReportWriter::AddKey(std::string key,std::string value) 
{
  m_KeyValueMap.insert(KeyValueMapType::value_type(key,value));
}


void mitk::TemplateReportWriter::ReadKeywordsFromTemplateFile()
{
  if (m_TemplateFileName.empty())
    return;

  std::ifstream templateFile(m_TemplateFileName.c_str(), std::ios::in);
  if (!templateFile)
    return;

  //QRegExp reg("(#.+#)");
  //std::string line;
  //while (!std::getline(templateFile,line).eof())    // for each line in the file
  //{
  //  int pos = 0;
  //  while ( (pos = reg.search(line, pos)) != -1 )   // if it contains a keyword
  //  {
  //    this->AddKey(reg.cap(1),"");                      // add keyword and search in remainder of string
  //    pos += rx.matchedLength();
  //  }
  //}
  templateFile.close();
}


void mitk::TemplateReportWriter::Generate()
{
  std::ifstream templateFile(m_TemplateFileName.c_str(), std::ios::in);
  std::ofstream reportFile(m_OutputFileName.c_str(), std::ios::out | std::ios::trunc);
  std::string line;
  int i = 0;

  if ((!templateFile) || (!reportFile))
    return;

  while (!std::getline(templateFile,line).eof())
  {
    for (KeyValueMapType::iterator it = m_KeyValueMap.begin(); it != m_KeyValueMap.end() ; it++)
    {
      std::string key = it->first;
      std::string::size_type idx = line.find(key);
      if (idx != std::string::npos)
      {
        std::stringstream replacedLine;
        replacedLine << line.substr(0, idx);            // the changed line starts with beginning of old line until the found keyword
        replacedLine << it->second;                     // then replacement value for keyword
        replacedLine << line.substr(idx + key.size());  // then remainder of old line after keyword
        std::cout << "changed line '" << line << "' to " << replacedLine.str() << std::endl;
        line = replacedLine.str();                      // change line and continue (there could be other keywords in the same line)
      }
    }
    reportFile << line << std::endl;
  }
  templateFile.close();
  reportFile.close();
}
