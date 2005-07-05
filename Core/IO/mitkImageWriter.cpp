/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkImageWriter.h"
#include "mitkItkImageWrite.h"

#include "mitkImage.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageAccessByItk.h"
#include "mitkPicFileWriter.h"

#include <itksys/SystemTools.hxx>

#include <sstream>

mitk::ImageWriter::ImageWriter()
{
  this->SetNumberOfRequiredInputs( 1 );
  
  SetDefaultExtension();
}

mitk::ImageWriter::~ImageWriter()
{
}

void mitk::ImageWriter::SetDefaultExtension()
{
  m_Extension = ".mhd";
}

void mitk::ImageWriter::GenerateData()
{
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  mitk::Image::Pointer input = const_cast<mitk::Image*>(this->GetInput());

  if ( m_Extension.find(".pic") == std::string::npos )
  {
    if(input->GetDimension() > 3)
    {
      int t, timesteps;

      timesteps = input->GetDimension(3);
      ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
      timeSelector->SetInput(input);
      mitk::Image::Pointer image = timeSelector->GetOutput();
      for(t = 0; t < timesteps; ++t)
      {
        ::itk::OStringStream filename;
        timeSelector->SetTimeNr(t);
        timeSelector->Update();
        if(input->GetTimeSlicedGeometry()->IsValidTime(t))
        {
          const mitk::TimeBounds& timebounds = input->GetTimeSlicedGeometry()->GetGeometry3D(t)->GetTimeBoundsInMS();
          filename <<  m_FileName.c_str() << "_S" << std::setprecision(0) << timebounds[0] << "_E" << std::setprecision(0) << timebounds[1] << "_T" << t << m_Extension;
        }
        else
        {
          itkWarningMacro(<<"Error on write: TimeSlicedGeometry invalid of image " << filename << ".");
          filename <<  m_FileName.c_str() << "_T" << t << m_Extension;
        }
        AccessByItk_1( image, _mitkItkImageWrite, filename.str() );
      }
    }
    else
    {
      ::itk::OStringStream filename;
      filename <<  m_FileName.c_str() << m_Extension;
      AccessByItk_1( input, _mitkItkImageWrite, filename.str() );
    }
  }
  else
  {
    ::itk::OStringStream filename;
    filename <<  m_FileName.c_str() << m_Extension;

    PicFileWriter::Pointer picWriter = PicFileWriter::New();
    picWriter->SetInput(input);
    picWriter->SetFileName(filename.str().c_str());
    picWriter->Write();
  }
}

void mitk::ImageWriter::SetInput( mitk::Image* image )
{
  this->ProcessObject::SetNthInput( 0, image );
}

const mitk::Image* mitk::ImageWriter::GetInput()
{
  if ( this->GetNumberOfInputs() < 1 )
  {
    return NULL;
  }
  else
  {
    return static_cast< const mitk::Image * >( this->ProcessObject::GetInput( 0 ) );
  }
}
