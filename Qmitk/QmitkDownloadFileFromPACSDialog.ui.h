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

void QmitkDownloadFileFromPACSDialog::init()
{
  mitk::PACSPlugin::Pointer pacs = mitk::PACSPlugin::GetInstance();
  if ( !pacs->GetPluginCapabilities().HasLoadCapability )
  {
    lstFiles->setEnabled( false );
    lblConsider->setEnabled( false );
    lblConsider->setText("PACS does not allow download...");
    btnDownload->setEnabled( false );
  }
  else
  {
    lstFiles->clear();

    mitk::PACSPlugin::StudyInformation study = pacs->GetStudyInformation();
    std::string studyInstanceUID( study.StudyInstanceUID );
    
    // iterate all series, find all files
    mitk::PACSPlugin::SeriesInformationList seriesList = pacs->GetSeriesInformationList( studyInstanceUID );
    for ( mitk::PACSPlugin::SeriesInformationList::iterator seriesIter = seriesList.begin();
          seriesIter != seriesList.end();
          ++seriesIter )
    {
      std::string seriesInstanceUID( seriesIter->SeriesInstanceUID );
      mitk::PACSPlugin::DocumentInformationList documentList = pacs->GetDocumentInformationList( seriesInstanceUID );
      for ( mitk::PACSPlugin::DocumentInformationList::iterator documentIter = documentList.begin();
            documentIter != documentList.end();
            ++documentIter )
      {
        m_Files.insert( std::make_pair(
                        new QListViewItem( lstFiles,
                                           QString("%1").arg(documentIter->InstanceNumber), // unsigned integer
                                           documentIter->ContentDate.c_str(),
                                           documentIter->DocumentTitle.c_str(),
                                           documentIter->MimeType.c_str()
                                           ),
                               std::make_pair( *seriesIter, 
                                               *documentIter) ) );
      }
    }
  }
}

void QmitkDownloadFileFromPACSDialog::lstFiles_selectionChanged()
{
  btnDownload->setEnabled( lstFiles->selectedItem() );
  lblConsider->setEnabled( lstFiles->selectedItem() );
  if ( btnDownload->isEnabled() )
  {
    lblConsider->setText("Click button to download selected file...");
  }
  else
  {
    lblConsider->setText("PACS does not allow download...");
  }
}

void QmitkDownloadFileFromPACSDialog::btnDownload_clicked()
{
  QListViewItem* item = lstFiles->selectedItem();
  if (!item) return;

  std::map< QListViewItem*, 
            std::pair< mitk::PACSPlugin::SeriesInformation, 
                       mitk::PACSPlugin::DocumentInformation> >::iterator documentSearch = m_Files.find( item );
  if ( documentSearch != m_Files.end() )
  {
    mitk::PACSPlugin::SeriesInformation series = documentSearch->second.first;
    mitk::PACSPlugin::DocumentInformation document = documentSearch->second.second;

    std::string seriesInstanceUID = series.SeriesInstanceUID;
    unsigned int docInstanceNumber = document.InstanceNumber;

    std::cout << "Download document " << docInstanceNumber << " from series " << seriesInstanceUID << std::endl;
    mitk::PACSPlugin::Pointer pacs = mitk::PACSPlugin::GetInstance();

    QFileDialog fileDialog;
    QString directory = fileDialog.getExistingDirectory( QString::null, 0, 0, "Select download destination..." );
    
    std::string filename = directory.ascii();
    filename += "/";
    if ( series.SeriesDescription.empty() )
    {
      filename += "DownloadFromPACS";
    }
    else
    {
      filename += series.SeriesDescription;
    }

    pacs->DownloadSingleFile( seriesInstanceUID, docInstanceNumber, filename );
  }
}


