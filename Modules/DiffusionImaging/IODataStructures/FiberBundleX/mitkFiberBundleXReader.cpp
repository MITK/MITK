/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkFiberBundleXReader.h"
#include <itkMetaDataObject.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <itksys/SystemTools.hxx>

namespace mitk
{

  void FiberBundleXReader
    ::GenerateData()
  {
    if ( ( ! m_OutputCache ) )
    {
      Superclass::SetNumberOfRequiredOutputs(0);
      this->GenerateOutputInformation();
    }

    if (!m_OutputCache)
    {
      itkWarningMacro("Output cache is empty!");
    }


    Superclass::SetNumberOfRequiredOutputs(1);
    Superclass::SetNthOutput(0, m_OutputCache.GetPointer());
  }

  void FiberBundleXReader::GenerateOutputInformation()
  {
    try
    {
      MITK_INFO << "Reading fiber bundle";
      const std::string& locale = "C";
      const std::string& currLocale = setlocale( LC_ALL, NULL );
      setlocale(LC_ALL, locale.c_str());

      std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
      ext = itksys::SystemTools::LowerCase(ext);

      vtkSmartPointer<vtkDataReader> chooser=vtkDataReader::New();
      chooser->SetFileName(m_FileName.c_str() );
      if( chooser->IsFilePolyData())
      {
        vtkSmartPointer<vtkPolyDataReader> reader = vtkPolyDataReader::New();
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          vtkSmartPointer<vtkPolyData> fiberPolyData = reader->GetOutput();
          m_OutputCache = OutputType::New(fiberPolyData);
        }
      }

      setlocale(LC_ALL, currLocale.c_str());
      MITK_INFO << "Fiber bundle read";
    }
    catch(...)
    {
      throw;
    }
  }

  void FiberBundleXReader::Update()
  {
    this->GenerateData();
  }

  const char* FiberBundleXReader
    ::GetFileName() const
  {
    return m_FileName.c_str();
  }


  void FiberBundleXReader
    ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }


  const char* FiberBundleXReader
    ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }


  void FiberBundleXReader
    ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }


  const char* FiberBundleXReader
    ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }


  void FiberBundleXReader
    ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }


  bool FiberBundleXReader
    ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".fib")
    {
      return true;
    }

    return false;
  }

} //namespace MITK
