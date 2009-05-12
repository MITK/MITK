/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15780 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkPACSLoadDialog.h"

#include "mitkColorProperty.h"
#include "mitkCoreObjectFactory.h"
#include "mitkMaterialProperty.h"
#include "mitkPACSPlugin.h"
#include "mitkPACSPluginEvents.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkSurface.h"

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <itkCommand.h>

#include "series_import.xpm"
#include "series_image_import.xpm"
#include "series_doc_import.xpm"

std::vector<mitk::DataTreeNode*> QmitkPACSLoadDialog::m_ImportedDataTreeNodes;

QmitkPACSLoadDialog::QmitkPACSLoadDialog(QWidget* parent, bool mainAppFunctionalityIntegration, const char* name)
:QDialog(parent, name),
 m_MainAppFunctionalityIntegration(mainAppFunctionalityIntegration)
{
  m_Plugin = mitk::PACSPlugin::GetInstance();
  if (!m_Plugin) throw std::logic_error ("There should always be an instance of mitk::PACSPlugin.");

  itk::ReceptorMemberCommand<QmitkPACSLoadDialog>::Pointer command = itk::ReceptorMemberCommand<QmitkPACSLoadDialog>::New();
  command->SetCallbackFunction( this, &QmitkPACSLoadDialog::PluginEventNewStudySelected );
  m_ObserverTag = m_Plugin->AddObserver( mitk::PluginStudySelected(), command );

  QGridLayout* grid;
  if (mainAppFunctionalityIntegration)
    grid = new QGridLayout( this, 8, 1 ); // parent, rows, cols
  else
    grid = new QGridLayout( this, 3, 2 ); // parent, rows, cols
    
  m_PACSContentList = new QmitkPACSContentList( this, mainAppFunctionalityIntegration );
  if (mainAppFunctionalityIntegration)
    grid->addMultiCellWidget( m_PACSContentList, 0, 7, 1, 1 ); // widget, fromRow, toRow, fromCol, toCol
  else
    grid->addMultiCellWidget( m_PACSContentList, 0, 0, 0, 1 ); // widget, fromRow, toRow, fromCol, toCol
  
  if (mainAppFunctionalityIntegration)
  {
    { QLabel* dummylabel = new QLabel( " ", this );
    grid->addMultiCellWidget( dummylabel, 0, 1, 0, 0 ); }
    { QLabel* dummylabel = new QLabel( " ", this );
    grid->addMultiCellWidget( dummylabel, 2, 3, 0, 0 ); }

    QToolButton* btnImportSeries = new QToolButton( this );
    QToolTip::add( btnImportSeries, "Import series images and documents to MITK");
    grid->addMultiCellWidget( btnImportSeries, 4, 4, 0, 0 );
    btnImportSeries->setPixmap( QPixmap(series_import_xpm) );
    connect( btnImportSeries, SIGNAL(clicked()), this, SLOT(ImportSeries()) );

    QToolButton* btnImportSeriesImages = new QToolButton( this );
    QToolTip::add( btnImportSeriesImages, "Import only series' images to MITK");
    grid->addMultiCellWidget( btnImportSeriesImages, 5, 5, 0, 0 );
    btnImportSeriesImages->setPixmap( QPixmap(series_image_import_xpm) );
    connect( btnImportSeriesImages, SIGNAL(clicked()), this, SLOT(ImportSeriesImages()) );
   
    QToolButton* btnImportSeriesDocuments = new QToolButton( this );
    QToolTip::add( btnImportSeriesDocuments, "Import documents to MITK");
    grid->addMultiCellWidget( btnImportSeriesDocuments, 6, 7, 0, 0 );
    btnImportSeriesDocuments->setPixmap( QPixmap(series_doc_import_xpm) );
    connect( btnImportSeriesDocuments, SIGNAL(clicked()), this, SLOT(ImportSeriesDocuments()) );
  }
  else
  {
    QToolButton* btnImportSeries = new QToolButton( this );
    btnImportSeries->setUsesTextLabel(true);
    btnImportSeries->setTextLabel("Import all from selected series");
    btnImportSeries->setTextPosition( QToolButton::BesideIcon );
    QToolTip::add( btnImportSeries, "Import series images and documents to MITK");
    grid->addMultiCellWidget( btnImportSeries, 2, 2, 0, 1 );
    btnImportSeries->setPixmap( QPixmap(series_import_xpm) );
    connect( btnImportSeries, SIGNAL(clicked()), this, SLOT(ImportSeriesAndClose()) );

    QToolButton* btnImportSeriesImages = new QToolButton( this );
    btnImportSeriesImages->setUsesTextLabel(true);
    btnImportSeriesImages->setTextLabel("Import images from series");
    btnImportSeriesImages->setTextPosition( QToolButton::BesideIcon );
    QToolTip::add( btnImportSeriesImages, "Import only series' images to MITK");
    grid->addMultiCellWidget( btnImportSeriesImages, 1, 1, 0, 0 );
    btnImportSeriesImages->setPixmap( QPixmap(series_image_import_xpm) );
    connect( btnImportSeriesImages, SIGNAL(clicked()), this, SLOT(ImportSeriesImagesAndClose()) );

    QToolButton* btnImportSeriesDocuments = new QToolButton( this );
    btnImportSeriesDocuments->setUsesTextLabel(true);
    btnImportSeriesDocuments->setTextLabel("Import documents");
    btnImportSeriesDocuments->setTextPosition( QToolButton::BesideIcon );
    QToolTip::add( btnImportSeriesDocuments, "Import documents to MITK");
    grid->addMultiCellWidget( btnImportSeriesDocuments, 1, 1, 1, 1 );
    btnImportSeriesDocuments->setPixmap( QPixmap(series_doc_import_xpm) );
    connect( btnImportSeriesDocuments, SIGNAL(clicked()), this, SLOT(ImportSeriesDocumentsAndClose()) );
  }

  m_ImportedDataTreeNodes.clear();
}

QmitkPACSLoadDialog::~QmitkPACSLoadDialog()
{
  if (!m_MainAppFunctionalityIntegration)
  {
    if( m_Plugin )
    {
      m_Plugin->RemoveObserver( m_ObserverTag );
    }
  }
}
    
void QmitkPACSLoadDialog::exec()
{
  m_PACSContentList->UpdateLists();
  QDialog::exec();
}

void QmitkPACSLoadDialog::PluginEventNewStudySelected( const itk::EventObject& )
{
}
    
void QmitkPACSLoadDialog::SetDataStorageParent( mitk::DataTreeNode* node )
{
  m_DataStorageParent = node;
}

void QmitkPACSLoadDialog::ImportSeriesAndClose()
{
  ImportSeries();
  QDialog::accept();
}

void QmitkPACSLoadDialog::ImportSeriesImagesAndClose()
{
  ImportSeriesImages();
  QDialog::accept();
}

void QmitkPACSLoadDialog::ImportSeriesDocumentsAndClose()
{
  ImportSeriesDocuments();
  QDialog::accept();
}
 
void QmitkPACSLoadDialog::ImportSeries()
{
  ImportSeriesImages();
  ImportSeriesDocuments(true, false); // all without warning for missing selection
}

void QmitkPACSLoadDialog::ImportSeriesImages()
{
  m_ImportedDataTreeNodes.clear();

  std::cout << "ImportSeriesImages()" << std::endl;
  std::list< std::string > seriesInstanceUIDs = m_PACSContentList->GetSelectedSeriesInstanceUIDs();
  
  if (seriesInstanceUIDs.empty())
  {
    QMessageBox::information( NULL, "Series import", "Please select series before importing them.",
                              QMessageBox::Ok );
  }
  
  std::vector<std::string> uids;
  for ( std::list< std::string >::iterator iter = seriesInstanceUIDs.begin();
        iter != seriesInstanceUIDs.end();
        ++iter )
  {
    uids.push_back( *iter );
  }
    
  std::vector<mitk::DataTreeNode::Pointer> m_ImportedNodes = m_Plugin->LoadImagesFromSeries( uids );
    for ( std::vector<mitk::DataTreeNode::Pointer>::iterator niter = m_ImportedNodes.begin();
          niter != m_ImportedNodes.end();
          ++niter )
    {
      if ( niter->IsNotNull() )
      {
        DecorateNode( niter->GetPointer() );
        m_ImportedDataTreeNodes.push_back( niter->GetPointer() );
        mitk::DataStorage::GetInstance()->Add( *niter, m_DataStorageParent );
      }
      else
      {
        //std::cout << "  Problem importing series " << *iter << std::endl;
        std::cout << "    Import classes returned an empty DataTreeNode" << std::endl;
      }
    }
/*
  for ( std::list< std::string >::iterator iter = seriesInstanceUIDs.begin();
        iter != seriesInstanceUIDs.end();
        ++iter )
  {
    std::cout << "  Import all images of series " << *iter << std::endl;
    std::vector<mitk::DataTreeNode::Pointer> m_ImportedNodes = m_Plugin->LoadImagesFromSeries( *iter );
    for ( std::vector<mitk::DataTreeNode::Pointer>::iterator niter = m_ImportedNodes.begin();
          niter != m_ImportedNodes.end();
          ++niter )
    {
      if ( niter->IsNotNull() )
      {
        DecorateNode( niter->GetPointer() );
        m_ImportedDataTreeNodes.push_back( niter->GetPointer() );
        mitk::DataStorage::GetInstance()->Add( *niter, m_DataStorageParent );
      }
      else
      {
        std::cout << "  Problem importing series " << *iter << std::endl;
        std::cout << "    Import classes returned an empty DataTreeNode" << std::endl;
      }
    }
  }
*/

  mitk::RenderingManager::GetInstance()->InitializeViews( mitk::DataStorage::GetInstance() );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkPACSLoadDialog::ImportSeriesDocuments(bool all, bool warnIfNoSelection)
{
  if (warnIfNoSelection) // this is default, warnIfNoSelection is only false, when this method is called right after image import
    m_ImportedDataTreeNodes.clear(); 

  std::cout << "ImportSeriesDocuments()" << std::endl;
  std::list< std::pair<std::string,unsigned int> > documents = m_PACSContentList->GetSelectedSeriesInstanceUIDsAndDocumentInstanceNumbers(all);

  if (warnIfNoSelection && documents.empty())
  {
    QMessageBox::information( NULL, "Series import", "Please select documents before importing them.",
                              QMessageBox::Ok );
  }

  for ( std::list< std::pair<std::string,unsigned int> >::iterator iter = documents.begin();
        iter != documents.end();
        ++iter )
  {
    std::cout << "  Import all document #" << iter->second << " of series " << iter->first << std::endl;
    mitk::DataTreeNode::Pointer node = m_Plugin->LoadSingleText( iter->first, iter->second );
    if (node.IsNotNull())
    {
      DecorateNode( node.GetPointer() );
      m_ImportedDataTreeNodes.push_back( node.GetPointer() );
      mitk::DataStorage::GetInstance()->Add( node, m_DataStorageParent );
    }
    else
    {
      std::cout << "  Problem importing document #" << iter->second << " of series " << iter->first << std::endl;
      std::cout << "    Import classes returned an empty DataTreeNode" << std::endl;
    }
  }
      
  mitk::RenderingManager::GetInstance()->InitializeViews( mitk::DataStorage::GetInstance() );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
    
std::vector<mitk::DataTreeNode*> QmitkPACSLoadDialog::GetImportedDataTreeNodes()
{
  return m_ImportedDataTreeNodes;
}

void QmitkPACSLoadDialog::DecorateNode(mitk::DataTreeNode* node)
{
  if (!node) return;

  mitk::CoreObjectFactory::GetInstance()->SetDefaultProperties( node );

  mitk::BaseData* data = node->GetData();
  if ( data )
  {
    // special treatment for images
    if ( mitk::Image* image = dynamic_cast<mitk::Image*>(data) )
    {
      // test whether this image contains only 2 values ==> mark as binary image and segmentation
      mitk::ScalarType max = image->GetScalarValueMax();
      mitk::ScalarType max2 = image->GetScalarValue2ndMax();
      mitk::ScalarType min2 = image->GetScalarValue2ndMin();
      mitk::ScalarType min = image->GetScalarValueMin();

      if (    fabs( min - max2 ) < mitk::eps 
           && fabs( max - min2 ) < mitk::eps 
         )
      {
        node->SetProperty( "binary", mitk::BoolProperty::New(true) );
        node->SetProperty( "segmentation", mitk::BoolProperty::New(true) );
        node->SetProperty( "layer", mitk::IntProperty::New(10) );
        node->SetProperty( "color", mitk::ColorProperty::New(1.0, 0.0, 0.0) );
        node->SetProperty( "opacity", mitk::FloatProperty::New(0.3) );
      }
    }
    
    // special treatment for surface objects
    if ( /* mitk::Surface* surface = */ dynamic_cast<mitk::Surface*>(data) )
    {
      node->SetProperty( "layer", mitk::IntProperty::New(10) );
      node->SetProperty( "color", mitk::ColorProperty::New(0.0, 1.0, 0.0) );
      node->SetProperty( "opacity", mitk::FloatProperty::New(0.3) );
      mitk::Color color; color.Set( 0.0, 0.1, 0.0 );
      node->SetProperty( "material", mitk::MaterialProperty::New( color, 0.3, node ) );
    }
  }
}

