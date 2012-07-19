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

#include "mitkException.h"

void mitk::Exception::AddRethrowData(const char *file, unsigned int lineNumber, const char *message)
    {
    mitk::Exception::ReThrowData data = {file, lineNumber, message};
    m_RethrowData.push_back(data);
    }
    
int mitk::Exception::GetNumberOfRethrows()
    {
    return m_RethrowData.size();
    }
    
void mitk::Exception::GetRethrowData(int rethrowNumber, std::string &file, int &line, std::string &message)
    {
    if (rethrowNumber > m_RethrowData.size())
        {
        file = "";
        line = 0;
        message = "";
        return;
        }    
        
    file = m_RethrowData.at(rethrowNumber).RethrowClassname;
    line = m_RethrowData.at(rethrowNumber).RethrowLine;
    message = m_RethrowData.at(rethrowNumber).RethrowMessage;
    }
