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

#include "mitkFiberBundleXReader.h"
#include <itkMetaDataObject.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <itksys/SystemTools.hxx>
#include <tinyxml.h>
#include <vtkCleanPolyData.h>
#include <mitkTrackvis.h>

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
        const std::string& locale = "C";
        const std::string& currLocale = setlocale( LC_ALL, NULL );
        setlocale(LC_ALL, locale.c_str());

        std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
        ext = itksys::SystemTools::LowerCase(ext);

        if (ext==".trk")
        {
            m_OutputCache = OutputType::New();
            TrackVisFiberReader reader;
            reader.open(m_FileName);
            reader.read(m_OutputCache);
            return;
        }

        vtkSmartPointer<vtkDataReader> chooser=vtkSmartPointer<vtkDataReader>::New();
        chooser->SetFileName(m_FileName.c_str() );
        if( chooser->IsFilePolyData())
        {
            vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
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

    if (ext == ".fib" || ext == ".trk")
    {
        return true;
    }

    return false;
}

BaseDataSource::DataObjectPointer FiberBundleXReader::MakeOutput(const DataObjectIdentifierType &name)
{
    itkDebugMacro("MakeOutput(" << name << ")");
    if( this->IsIndexedOutputName(name) )
    {
        return this->MakeOutput( this->MakeIndexFromOutputName(name) );
    }
    return static_cast<itk::DataObject*>(OutputType::New().GetPointer());
}

BaseDataSource::DataObjectPointer FiberBundleXReader::MakeOutput(DataObjectPointerArraySizeType /*idx*/)
{
    return OutputType::New().GetPointer();
}

} //namespace MITK
