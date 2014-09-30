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
#include <mitkIOUtil.h>
#include <mitkImageWriter.h>
#include <mitkUIDGenerator.h>
#include <Poco/Path.h>

mitk::USImageLoggingFilter::USImageLoggingFilter() : m_SystemTimeClock(RealTimeClock::New()),
                                                     m_ImageExtension(".nrrd")
{
}

mitk::USImageLoggingFilter::~USImageLoggingFilter()
{
}

void mitk::USImageLoggingFilter::GenerateData()
{
  mitk::Image::ConstPointer inputImage = this->GetInput();
  mitk::Image::Pointer outputImage = this->GetOutput();

  if(inputImage.IsNull() || inputImage->IsEmpty())
    {
    MITK_WARN << "Input image is not valid. Cannot save image!";
    return;
    }

  //a clone is needed for a output and to store it.
  mitk::Image::Pointer inputClone = inputImage->Clone();


  //simply redirecy the input to the output
  //this->SetNumberOfRequiredOutputs(1);
  //this->SetNthOutput(0, inputClone->Clone());
  //outputImage->Graft(inputImage);
  //this->SetOutput(this->GetInput());
  /*if (!this->GetOutput()->IsInitialized())
    {
    this->SetNumberOfRequiredOutputs(1);
    mitk::Image::Pointer newOutput = mitk::Image::New();
    this->SetNthOutput(0, newOutput);
    }
  memcpy(this->GetOutput(),this->GetInput());*/

  //this->SetNthOutput(0,inputImage.);
  //this->AllocateOutputs();
  //this->GraftOutput(inputClone);

  /*
  if (!this->GetOutput()->IsInitialized())
    {
    mitk::Image::Pointer newOutput = mitk::Image::New();
    this->SetNthOutput(0, newOutput);
    }
  this->GetOutput()Graft(this->GetInput());
  */


  m_LoggedImages.push_back(inputClone);
  m_LoggedMITKSystemTimes.push_back(m_SystemTimeClock->GetCurrentStamp());

}

void mitk::USImageLoggingFilter::AddMessageToCurrentImage(std::string message)
{
  m_LoggedMessages.insert(std::make_pair(static_cast<int>(m_LoggedImages.size()-1),message));
}

void mitk::USImageLoggingFilter::SaveImages(std::string path)
{
std::vector<std::string> dummy1;
std::string dummy2;
this->SaveImages(path,dummy1,dummy2);
}

void mitk::USImageLoggingFilter::SaveImages(std::string path, std::vector<std::string>& filenames, std::string& csvFileName)
{
  filenames = std::vector<std::string>();

  //test if path is valid
  Poco::Path testPath(path);
  if(!testPath.isDirectory())
    {
    mitkThrow() << "Attemting to write to directory " << path << " which is not valid! Aborting!";
    }

  //generate a unique ID which is used as part of the filenames, so we avoid to overwrite old files by mistake.
  mitk::UIDGenerator myGen = mitk::UIDGenerator("",5);
  std::string uniqueID = myGen.GetUID();

  //first: write the images
  for(size_t i=0; i<m_LoggedImages.size(); i++)
    {
      std::stringstream name;
      name << path << uniqueID << "_Image_" << i << m_ImageExtension;
      mitk::IOUtil::Save(m_LoggedImages.at(i),name.str());
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
  os.precision(15); //set high precision to avoid loss of digits

  //write header
  os << "image filename; MITK system timestamp; message\n";

  //write data
  for(size_t i=0; i<m_LoggedImages.size(); i++)
    {
    std::map<int, std::string>::iterator it = m_LoggedMessages.find(i);
    if (m_LoggedMessages.empty() || (it == m_LoggedMessages.end())) os << filenames.at(i) << ";" << m_LoggedMITKSystemTimes.at(i) << ";" << "" << "\n";
    else os << filenames.at(i) << ";" << m_LoggedMITKSystemTimes.at(i) << ";" << it->second << "\n";
    }

  //close file
  fb.close();
}

bool mitk::USImageLoggingFilter::SetImageFilesExtension(std::string extension)
 {
 mitk::ImageWriter::Pointer imageWriter = mitk::ImageWriter::New();
 if(!imageWriter->IsExtensionValid(extension))
  {
  MITK_WARN << "Extension " << extension << " is not supported; still using " << m_ImageExtension << " as before.";
  return false;
  }
 else
  {
  m_ImageExtension = extension;
  return true;
  }
 }
