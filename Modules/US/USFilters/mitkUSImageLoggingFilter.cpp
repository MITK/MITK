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

#include "mitkUSImageLoggingFilter.h"
#include "mitkIOUtil.h"
#include "mitkUIDGenerator.h"

mitk::USImageLoggingFilter::USImageLoggingFilter() : m_SystemTimeClock(RealTimeClock::New())
{
}

mitk::USImageLoggingFilter::~USImageLoggingFilter()
{
}

void mitk::USImageLoggingFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetInput();
  //mitk::Image::Pointer output = this->GetOutput();

  if(inputImage.IsNull() || inputImage->IsEmpty())
    {
    MITK_WARN << "Input image is not valid. Cannot save image!";
    return;
    }

  m_LoggedImages.push_back(inputImage->Clone());
  m_LoggedMITKSystemTimes.push_back(m_SystemTimeClock->GetCurrentStamp());

}

void mitk::USImageLoggingFilter::AddMessageToCurrentImage(std::string message)
{
  std::pair<int,std::string> newMessage(m_LoggedImages.size()-1,message);
  m_LoggedMessages.insert(newMessage);
}

void mitk::USImageLoggingFilter::SaveImages(std::string path, std::vector<std::string>& filenames, std::string& csvFileName)
{
  filenames = std::vector<std::string>();

  //generate a unique ID which is used as part of the filenames, so we avoid to overwrite old files by mistake.
  mitk::UIDGenerator myGen = mitk::UIDGenerator("",5);
  std::string uniqueID = myGen.GetUID();

  //first: write the images
  for(int i=0; i<m_LoggedImages.size(); i++)
    {
      std::stringstream name;
      name << path << uniqueID << "_Image_" << i << ".nrrd";
      mitk::IOUtil::SaveImage(m_LoggedImages.at(i),name.str());
      filenames.push_back(name.str());
    }

  //then: write a csv file which contains comments to all the images

  //open file
  std::stringstream csvFilenameStream;
  csvFilenameStream << path << uniqueID << "_ImageMessages.csv";
  csvFileName = csvFilenameStream.str();
  std::filebuf fb;
  fb.open (csvFileName.c_str(),std::ios::out);
  std::ostream os(&fb);

  //write header
  os << "image filename; MITK system timestamp; message\n";

  //write data
  for(int i=0; i<m_LoggedImages.size(); i++)
    {
    std::map<int, std::string>::iterator it = m_LoggedMessages.find(i);
    if (m_LoggedMessages.empty() || (it == m_LoggedMessages.end())) os << filenames.at(i) << ";" << m_LoggedMITKSystemTimes.at(i) << ";" << "" << "\n";
    else os << filenames.at(i) << ";" << m_LoggedMITKSystemTimes.at(i) << ";" << it->second << "\n";
    }

  //close file
  fb.close();
}
