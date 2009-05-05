/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <qfiledialog.h>
#include <qfile.h>
#include <qdir.h>
#include <qmessagebox.h>

#include <itkCommand.h>

#include "mitkPACSPlugin.h"
#include "mitkPACSPluginEvents.h"

void QmitkUploadFileToPACSDialog::init()
{
  mitk::PACSPlugin::Pointer pacs = mitk::PACSPlugin::GetInstance();
  if ( !pacs->GetPluginCapabilities().HasSaveCapability )
  {
    edtFilename->setEnabled( false );
    btnSelectFile->setEnabled( false );
    lblCurrentStudyBla->setEnabled( false );
    lblStudyDescription->setEnabled( false );
    lblLongValidationText->setEnabled( false );
    edtStudyDescription->setEnabled( false );
    lblConsider->setEnabled( false );
    lblConsider->setText("PACS does not allow upload...");
    btnUpload->setEnabled( false );
  }
  else
  {
    mitk::PACSPlugin::StudyInformation study = pacs->GetStudyInformation();
    
    if ( !study.StudyDescription.empty() )
    {
      lblStudyDescription->setText( study.StudyDescription.c_str() );
    }
    else if ( !study.StudyID.empty() )
    {
      lblStudyDescription->setText( study.StudyID.c_str() );

      // replace description by ID in the labels
      QString currentDescription = lblCurrentStudyBla->text();
      currentDescription.replace( "description", "ID" );
      lblCurrentStudyBla->setText( currentDescription );
      
      QString currentLongDescription = lblLongValidationText->text();
      currentLongDescription.replace( "description", "ID" );
      lblLongValidationText->setText( currentLongDescription );
    }
    else
    {
      QMessageBox::information(0, "Something missing...", "You need to select a study in your PACS first.\n\n"
                                                          
                                                          "(This may be a known bug, try to select\n"
                                                          "the same study again, if one is already selected)");
      lblFilename->setEnabled( false );
      edtFilename->setEnabled( false );
      btnSelectFile->setEnabled( false );
    }
  }
}

void QmitkUploadFileToPACSDialog::btnSelectFile_clicked()
{
  //
  // open file selection dialog
  // display filename in lineedit (trigger text changed)
  //

  QFileDialog fileDialog;
  QString filename = fileDialog.getOpenFileName( QString::null, QString::null, 0, 0, "Select file for upload to PACS..." );
  edtFilename->setText( filename );
}


void QmitkUploadFileToPACSDialog::edtFilename_textChanged( const QString & filename )
{
  //
  // if entered file exists
  //

  QFile file( filename );
  QDir dir( filename );
  if ( file.exists() && !dir.exists() )
  {
    edtFilename->setPaletteForegroundColor( Qt::green );

    lblCurrentStudyBla->setEnabled( true );
    lblStudyDescription->setEnabled( true );
    lblLongValidationText->setEnabled( true );
    edtStudyDescription->setText( "" );
    edtStudyDescription->setEnabled( true );
    edtStudyDescription->setFocus();
    lblConsider->setEnabled( false );
    lblConsider->setText("Validate study first...");
    btnUpload->setEnabled( false );
  }
  else
  {
    edtFilename->setPaletteForegroundColor( Qt::red );

    lblCurrentStudyBla->setEnabled( false );
    lblStudyDescription->setEnabled( false );
    lblLongValidationText->setEnabled( false );
    edtStudyDescription->setEnabled( false );
    lblConsider->setEnabled( false );
    lblConsider->setText("Select a file first...");
    btnUpload->setEnabled( false );
  }
}


void QmitkUploadFileToPACSDialog::edtStudyDescription_textChanged( const QString & )
{
  //
  // check if first 6 chars of entered string and content of lblStudyDesctiption match
  //  if so
  //      enable upload button, print green
  // else
  //     disable upload button, print red
  //
 
  unsigned int validationLength( 8 );

  QString usersSuggestion( edtStudyDescription->text() );
  usersSuggestion.truncate( validationLength );

  QString chilisSuggestion( lblStudyDescription->text() );
  chilisSuggestion.truncate( validationLength );
  
  //std::cout << "Comparing '" << usersSuggestion << "' and '" << chilisSuggestion << "'" << std::endl;
  if ( usersSuggestion == chilisSuggestion )
  {
    edtStudyDescription->setPaletteForegroundColor( Qt::green );
    
    lblConsider->setEnabled( true );
    lblConsider->setText("Click button to upload selected file...");
    btnUpload->setEnabled( true );
  }
  else
  {
    edtStudyDescription->setPaletteForegroundColor( Qt::red );
    
    lblConsider->setEnabled( false );
    lblConsider->setText("Validate study first...");
    btnUpload->setEnabled( false );
  }
}


void QmitkUploadFileToPACSDialog::btnUpload_clicked()
{
  //
  // call neccessary methods of mitk::PACSPlugin to create a new series in current study
  //

  mitk::PACSPlugin::Pointer pacs = mitk::PACSPlugin::GetInstance();
  mitk::PACSPlugin::StudyInformation study = pacs->GetStudyInformation();

  std::string filename = edtFilename->text().ascii();
  QFileInfo fileinfo( filename.c_str() );
  std::string filebasename = fileinfo.fileName();
  
  std::cout << "Upload '" << filename << "' to study '" << study.StudyDescription << "' as '" << filebasename << "'" << std::endl;

  //
  // find out, whether there already is a series called "filename"
  // if so
  //   overwrite this series
  // else
  //   create a new series called "filename"
  //

  std::string mimeType = pacs->GuessMIMEType( filename );
  if ( mimeType.empty() )
  {
    mimeType = "application/vnd.mitk-upload";
  }

  if ( mimeType == "Windows EXE" )
  {
    QMessageBox::information(0, "Something wrong...", 
                                "Looks like you want to upload a Windows executable file. This will not work.");
    return;
  }

  std::cout << "MIME type is probably '" << mimeType << "'" << std::endl;

  bool success( false );

  mitk::PACSPlugin::SeriesInformationList seriesList = pacs->GetSeriesInformationList( study.StudyInstanceUID );
  for ( mitk::PACSPlugin::SeriesInformationList::iterator seriesIter = seriesList.begin();
        seriesIter != seriesList.end();
        ++seriesIter )
  {
    std::string currentDescription = seriesIter->SeriesDescription;
    if ( currentDescription == filebasename )
    {
      // overwrite this series
      // (ask user first)

      pacs->UploadFileToSeries( filename, filebasename, mimeType, seriesIter->SeriesInstanceUID, true );

      success = true;
    }
  }

  if (!success)
  {
    // create a new series called "filename"
      
    pacs->UploadFileAsNewSeries( filename, mimeType, study.StudyInstanceUID, 128, filebasename );
  }
}

