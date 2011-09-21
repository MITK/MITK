/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkDataNodeObject.h"
#include "mitkProperties.h"
#include "mitkSegTool2D.h"
#include "mitkGlobalInteraction.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkNewSegmentationDialog.h"

#include <QMessageBox>

#include <berryIWorkbenchPage.h>

#include "QmitkSegmentationView.h"
#include "QmitkSegmentationPostProcessing.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

#include <mitkSurfaceToImageFilter.h>
#include <vtkPolyData.h>

#include "mitkVtkResliceInterpolationProperty.h"

const std::string QmitkSegmentationView::VIEW_ID =
"org.mitk.views.segmentation";

// public methods

QmitkSegmentationView::QmitkSegmentationView()
:m_Parent(NULL)
,m_Controls(NULL)
,m_MultiWidget(NULL)
,m_RenderingManagerObserverTag(0)
{
}

QmitkSegmentationView::~QmitkSegmentationView()
{
  // delete m_PostProcessing;
  delete m_Controls;
}

void QmitkSegmentationView::NewNodesGenerated()
{
//  ForceDisplayPreferencesUponAllImages();
}

void QmitkSegmentationView::NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType* nodes)
{
  if (!nodes) return;

  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  if (!toolManager) return;
  for (mitk::ToolManager::DataVectorType::iterator iter = nodes->begin(); iter != nodes->end(); ++iter)
  {
    this->FireNodeSelected( *iter );
    // only last iteration meaningful, multiple generated objects are not taken into account here
  }
}

void QmitkSegmentationView::Activated()
{
  // should be moved to ::BecomesVisible() or similar
  if( m_Controls )
  {
    m_Controls->m_ManualToolSelectionBox->setEnabled( true );
    m_Controls->m_OrganToolSelectionBox->setEnabled( true );
    m_Controls->m_LesionToolSelectionBox->setEnabled( true );

    m_Controls->m_SlicesInterpolator->EnableInterpolation( m_Controls->widgetStack->currentWidget() == m_Controls->pageManual );

    itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer command1 = itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
    command1->SetCallbackFunction( this, &QmitkSegmentationView::RenderingManagerReinitialized );
    m_RenderingManagerObserverTag = mitk::RenderingManager::GetInstance()->AddObserver( mitk::RenderingManagerViewsInitializedEvent(), command1 );
  }
}

void QmitkSegmentationView::Deactivated()
{
  if( m_Controls )
  {
    mitk::RenderingManager::GetInstance()->RemoveObserver( m_RenderingManagerObserverTag );

    m_Controls->m_ManualToolSelectionBox->setEnabled( false );
    //deactivate all tools
    m_Controls->m_ManualToolSelectionBox->GetToolManager()->ActivateTool(-1);
    m_Controls->m_OrganToolSelectionBox->setEnabled( false );
    m_Controls->m_LesionToolSelectionBox->setEnabled( false );

    m_Controls->m_SlicesInterpolator->EnableInterpolation( false );
  }
}

void QmitkSegmentationView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  SetMultiWidget(&stdMultiWidget);
}

void QmitkSegmentationView::StdMultiWidgetNotAvailable()
{
  SetMultiWidget(NULL);
}

void QmitkSegmentationView::StdMultiWidgetClosed( QmitkStdMultiWidget& /*stdMultiWidget*/ )
{
  SetMultiWidget(NULL);
}

void QmitkSegmentationView::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
  if (m_MultiWidget)
  {
  mitk::SlicesCoordinator* coordinator = m_MultiWidget->GetSlicesRotator();
  if (coordinator)
  {
    coordinator->RemoveObserver( m_SlicesRotationObserverTag1 );
  }
  coordinator = m_MultiWidget->GetSlicesSwiveller();
  if (coordinator)
  {
    coordinator->RemoveObserver( m_SlicesRotationObserverTag2 );
  }
  }

  // save the current multiwidget as the working widget
  m_MultiWidget = multiWidget;

  if (m_MultiWidget)
  {
    mitk::SlicesCoordinator* coordinator = m_MultiWidget->GetSlicesRotator();
    if (coordinator)
    {
      itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
      command2->SetCallbackFunction( this, &QmitkSegmentationView::SliceRotation );
      m_SlicesRotationObserverTag1 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
    }

    coordinator = m_MultiWidget->GetSlicesSwiveller();
    if (coordinator)
    {
      itk::ReceptorMemberCommand<QmitkSegmentationView>::Pointer command2 = itk::ReceptorMemberCommand<QmitkSegmentationView>::New();
      command2->SetCallbackFunction( this, &QmitkSegmentationView::SliceRotation );
      m_SlicesRotationObserverTag2 = coordinator->AddObserver( mitk::SliceRotationEvent(), command2 );
    }
  }

  if (m_Parent)
  {
    m_Parent->setEnabled(m_MultiWidget);
  }

  // tell the interpolation about toolmanager and multiwidget (and data storage)
  if (m_Controls && m_MultiWidget)
  {
    mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
    m_Controls->m_SlicesInterpolator->SetDataStorage( *(this->GetDefaultDataStorage()));
    m_Controls->m_SlicesInterpolator->Initialize( toolManager, m_MultiWidget );
  }
}

void QmitkSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
  ForceDisplayPreferencesUponAllImages();
}

void QmitkSegmentationView::RenderingManagerReinitialized(const itk::EventObject&)
{
  CheckImageAlignment();
}

void QmitkSegmentationView::SliceRotation(const itk::EventObject&)
{
  CheckImageAlignment();
}


// protected slots

void QmitkSegmentationView::CreateNewSegmentation()
{
  mitk::DataNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
    if (image.IsNotNull())
    {
      if (image->GetDimension()>2)
      {
        // ask about the name and organ type of the new segmentation
        QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( m_Parent ); // needs a QWidget as parent, "this" is not QWidget

        QString storedList = QString::fromStdString( this->GetPreferences()->GetByteArray("Organ-Color-List","") );
        QStringList organColors;
        if (storedList.isEmpty())
        {
          organColors = GetDefaultOrganColorString();
        }
        else
        {
          /*
             a couple of examples of how organ names are stored:

             a simple item is built up like 'name#AABBCC' where #AABBCC is the hexadecimal notation of a color as known from HTML

             items are stored separated by ';'
             this makes it necessary to escape occurrences of ';' in name.
             otherwise the string "hugo;ypsilon#AABBCC;eugen#AABBCC" could not be parsed as two organs
             but we would get "hugo" and "ypsilon#AABBCC" and "eugen#AABBCC"

             so the organ name "hugo;ypsilon" is stored as "hugo\;ypsilon"
             and must be unescaped after loading

             the following lines could be one split with Perl's negative lookbehind
          */

          // recover string list from BlueBerry view's preferences
          QString storedString = QString::fromStdString( this->GetPreferences()->GetByteArray("Organ-Color-List","") );
          MITK_DEBUG << "storedString: " << storedString.toStdString();
          // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
          QRegExp onePart("(?:[^;]|\\\\;)*");
          MITK_DEBUG << "matching " << onePart.pattern().toStdString();
          int count = 0;
          int pos = 0;
          while( (pos = onePart.indexIn( storedString, pos )) != -1 )
          {
            ++count;
            int length = onePart.matchedLength();
            if (length == 0) break;
            QString matchedString = storedString.mid(pos, length);
            MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
            pos += length + 1; // skip separating ';'

            // unescape possible occurrences of '\;' in the string
            matchedString.replace("\\;", ";");

            // add matched string part to output list
            organColors << matchedString;
          }
          MITK_DEBUG << "Captured " << count << " organ name/colors";
        }

        dialog->SetSuggestionList( organColors );

        int dialogReturnValue = dialog->exec();

        if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

        // ask the user about an organ type and name, add this information to the image's (!) propertylist
        // create a new image of the same dimensions and smallest possible pixel type
        mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
        mitk::Tool* firstTool = toolManager->GetToolById(0);
        if (firstTool)
        {
          try
          {
            mitk::DataNode::Pointer emptySegmentation =
              firstTool->CreateEmptySegmentationNode( image, dialog->GetSegmentationName().toStdString(), dialog->GetColor() );

            //Here we change the reslice interpolation mode for a segmentation, so that contours in rotated slice can be shown correctly
            emptySegmentation->SetProperty( "reslice interpolation", mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_LINEAR) );
            // initialize showVolume to false to prevent recalculating the volume while working on the segmentation
            emptySegmentation->SetProperty( "showVolume", mitk::BoolProperty::New( false ) );

            if (!emptySegmentation) return; // could be aborted by user

            UpdateOrganList( organColors, dialog->GetSegmentationName(), dialog->GetColor() );

            /*
               escape ';' here (replace by '\;'), see longer comment above
            */
            std::string stringForStorage = organColors.replaceInStrings(";","\\;").join(";").toStdString();
            MITK_DEBUG << "Will store: " << stringForStorage;
            this->GetPreferences()->PutByteArray("Organ-Color-List", stringForStorage );
            this->GetPreferences()->Flush();

            this->GetDefaultDataStorage()->Add( emptySegmentation, node ); // add as a child, because the segmentation "derives" from the original

            this->FireNodeSelected( emptySegmentation );
            this->OnSelectionChanged( emptySegmentation );
          }
          catch (std::bad_alloc&)
          {
            QMessageBox::warning(NULL,"Create new segmentation","Could not allocate memory for new segmentation");
          }
        }
      }
      else
      {
        QMessageBox::information(NULL,"Segmentation","Segmentation is currently not supported for 2D images");
      }
    }
  }
  else
  {
    MITK_ERROR << "'Create new segmentation' button should never be clickable unless a patient image is selected...";
  }
}

void QmitkSegmentationView::CreateSegmentationFromSurface()
{
  mitk::DataNode::Pointer surfaceNode =
      m_Controls->MaskSurfaces->GetSelectedNode();
  mitk::Surface::Pointer surface(0);
  if(surfaceNode.IsNotNull())
    surface = dynamic_cast<mitk::Surface*> ( surfaceNode->GetData() );
  if(surface.IsNull())
  {
    this->HandleException( "No surface selected.", m_Parent, true);
    return;
  }

  mitk::DataNode::Pointer imageNode
      = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  mitk::Image::Pointer image(0);
  if (imageNode.IsNotNull())
    image = dynamic_cast<mitk::Image*>( imageNode->GetData() );
  if(image.IsNull())
  {
    this->HandleException( "No image selected.", m_Parent, true);
    return;
  }

  mitk::SurfaceToImageFilter::Pointer s2iFilter
      = mitk::SurfaceToImageFilter::New();

  s2iFilter->MakeOutputBinaryOn();
  s2iFilter->SetInput(surface);
  s2iFilter->SetImage(image);
  s2iFilter->Update();

  mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
  std::string nameOfResultImage = imageNode->GetName();
  nameOfResultImage.append(surfaceNode->GetName());
  resultNode->SetProperty("name", mitk::StringProperty::New(nameOfResultImage) );
  resultNode->SetProperty("binary", mitk::BoolProperty::New(true) );
  resultNode->SetData( s2iFilter->GetOutput() );

  this->GetDataStorage()->Add(resultNode, imageNode);

}

void QmitkSegmentationView::ManualToolSelected(int id)
{
  // disable crosshair movement when a manual drawing tool is active (otherwise too much visual noise)
  if (m_MultiWidget)
  {
    if (id >= 0)
    {
      m_MultiWidget->DisableNavigationControllerEventListening();
      m_MultiWidget->SetWidgetPlaneMode(0);
    }
    else
    {
      m_MultiWidget->EnableNavigationControllerEventListening();
    }
  }
}

void QmitkSegmentationView::ToolboxStackPageChanged(int id)
{
  // interpolation only with manual tools visible
  m_Controls->m_SlicesInterpolator->EnableInterpolation( id == 0 );

  if( id == 0 )
  {
    mitk::DataNode::Pointer workingData =   m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetWorkingData(0);
    if( workingData.IsNotNull() )
    {
      m_Controls->lblSegmentation->setText( workingData->GetName().c_str() );
      m_Controls->lblSegImage->show();
      m_Controls->lblSegmentation->show();
    }
  }
  else
  {
    m_Controls->lblSegImage->hide();
    m_Controls->lblSegmentation->hide();
  }

  // this is just a workaround, should be removed when all tools support 3D+t
  if (id==2) // lesions
  {
    mitk::DataNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
    if (node.IsNotNull())
    {
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );
      if (image.IsNotNull())
      {
        if (image->GetDimension()>3)
        {
          m_Controls->widgetStack->setCurrentIndex(0);
          QMessageBox::information(NULL,"Segmentation","Lesion segmentation is currently not supported for 4D images");
        }
      }
    }
  }
}

// protected

void QmitkSegmentationView::OnComboBoxSelectionChanged( const mitk::DataNode* node )
{
  mitk::DataNode* selectedNode = const_cast<mitk::DataNode*>(node);
 if( selectedNode != NULL )
  {
    m_Controls->refImageSelector->show();
    m_Controls->lblReferenceImageSelectionWarning->hide();
    this->OnSelectionChanged( const_cast<mitk::DataNode*>(node) );
  }
  else
  {
    m_Controls->refImageSelector->hide();
    m_Controls->lblReferenceImageSelectionWarning->show();
  }
}

//to remember the contour positions
void QmitkSegmentationView::CheckboxRememberContourPositionsStateChanged (int state)
{
  mitk::SegTool2D::Pointer manualSegmentationTool;

  unsigned int numberOfExistingTools = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetTools().size();

  for(unsigned int i = 0; i < numberOfExistingTools; i++)
  {
    manualSegmentationTool = dynamic_cast<mitk::SegTool2D*>(m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetToolById(i));

    if (manualSegmentationTool)
    {
      if(state == Qt::Checked)
      {
        manualSegmentationTool->SetRememberContourPositions( true );
      }
      else
      {
        manualSegmentationTool->SetRememberContourPositions( false );
      }
    }
  }
}

void QmitkSegmentationView::OnSelectionChanged(mitk::DataNode* node)
{
  std::vector<mitk::DataNode*> nodes;
  nodes.push_back( node );
  this->OnSelectionChanged( nodes );
}



void QmitkSegmentationView::OnSurfaceSelectionChanged()
{
  // if Image and Surface are selected, enable button
  if ( (m_Controls->refImageSelector->GetSelectedNode().IsNull()) ||
       (m_Controls->MaskSurfaces->GetSelectedNode().IsNull()))
    m_Controls->CreateSegmentationFromSurface->setEnabled(false);
  else
    m_Controls->CreateSegmentationFromSurface->setEnabled(true);
}

void QmitkSegmentationView::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{
    // if the selected node is a contourmarker
    if ( !nodes.empty() )
    {
        std::string markerName = "Contourmarker";
        unsigned int numberOfNodes = nodes.size();
        std::string nodeName = nodes.at( 0 )->GetName();
        if ( ( numberOfNodes == 1 ) && ( nodeName.find( markerName ) == 0) )
        {
            this->OnContourMarkerSelected( nodes.at( 0 ) );
        }
    }
  // if Image and Surface are selected, enable button
  if ( (m_Controls->refImageSelector->GetSelectedNode().IsNull()) ||
       (m_Controls->MaskSurfaces->GetSelectedNode().IsNull()))
    m_Controls->CreateSegmentationFromSurface->setEnabled(false);
  else
    m_Controls->CreateSegmentationFromSurface->setEnabled(true);

  if (!m_Parent || !m_Parent->isVisible()) return;

  // reaction to BlueBerry selection events
  //   this method will try to figure out if a relevant segmentation and its corresponding original image were selected
  //   a warning is issued if the selection is invalid
  //   appropriate reactions are triggered otherwise

  mitk::DataNode::Pointer referenceData = FindFirstRegularImage( nodes ); //m_Controls->refImageSelector->GetSelectedNode(); //FindFirstRegularImage( nodes );
  mitk::DataNode::Pointer workingData =   FindFirstSegmentation( nodes );

  bool invalidSelection( !nodes.empty() &&
                         (
                           nodes.size() > 2 ||    // maximum 2 selected nodes
                           (nodes.size() == 2 && (workingData.IsNull() || referenceData.IsNull()) ) || // with two nodes, one must be the original image, one the segmentation
                           ( workingData.GetPointer() == referenceData.GetPointer() ) //one node is selected as reference and working image
                           // one item is always ok (might be working or reference or nothing
                         )
                       );

  if (invalidSelection)
  {
    // TODO visible warning when two images are selected
    MITK_ERROR << "WARNING: No image, too many (>2) or two equal images were selected.";
    workingData = NULL;

    if( m_Controls->refImageSelector->GetSelectedNode().IsNull() )
      referenceData = NULL;
  }

  if ( workingData.IsNotNull() && referenceData.IsNull() )
  {
    // find the DataStorage parent of workingData
    // try to find a "normal image" parent, select this as reference image
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
    mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateNot::Pointer isNotBinary = mitk::NodePredicateNot::New( isBinary );
    mitk::NodePredicateAnd::Pointer isNormalImage = mitk::NodePredicateAnd::New( isImage, isNotBinary );

    mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = this->GetDefaultDataStorage()->GetSources( workingData, isNormalImage );

    if (possibleParents->size() > 0)
    {
      if (possibleParents->size() > 1)
      {
        // TODO visible warning for this rare case
        MITK_ERROR << "Selected binary image has multiple parents. Using arbitrary first one for segmentation.";
      }

      referenceData = (*possibleParents)[0];
    }
  }

  //set comboBox to reference image
  disconnect( m_Controls->refImageSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
           this, SLOT( OnComboBoxSelectionChanged( const mitk::DataNode* ) ) );

  m_Controls->refImageSelector->setCurrentIndex( m_Controls->refImageSelector->Find(referenceData) );

  connect( m_Controls->refImageSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
    this, SLOT( OnComboBoxSelectionChanged( const mitk::DataNode* ) ) );


  // if Image and Surface are selected, enable button
  if ( (m_Controls->refImageSelector->GetSelectedNode().IsNull()) ||
       (m_Controls->MaskSurfaces->GetSelectedNode().IsNull()) ||
       (!referenceData))
    m_Controls->CreateSegmentationFromSurface->setEnabled(false);
  else
    m_Controls->CreateSegmentationFromSurface->setEnabled(true);

  SetToolManagerSelection(referenceData, workingData);
  ForceDisplayPreferencesUponAllImages();
}

//New since rotated contour drawing is allowed. Effects a reorientation of the plane of the affected widget to the marker`s position
void QmitkSegmentationView::OnContourMarkerSelected(const mitk::DataNode *node)
{

    const mitk::PlaneGeometry* markerGeometry =
            dynamic_cast<const mitk::PlaneGeometry*> ( node->GetData()->GetGeometry() );

    QmitkRenderWindow* selectedRenderWindow = 0;
    QmitkRenderWindow* RenderWindow1 =
        this->GetActiveStdMultiWidget()->GetRenderWindow1();
    QmitkRenderWindow* RenderWindow2 =
        this->GetActiveStdMultiWidget()->GetRenderWindow2();
    QmitkRenderWindow* RenderWindow3 =
        this->GetActiveStdMultiWidget()->GetRenderWindow3();
    QmitkRenderWindow* RenderWindow4 =
        this->GetActiveStdMultiWidget()->GetRenderWindow4();
    bool PlanarFigureInitializedWindow = false;

    // find initialized renderwindow
    if (node->GetBoolProperty("PlanarFigureInitializedWindow",
        PlanarFigureInitializedWindow, RenderWindow1->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow1;
    }
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow2->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow2;
    }
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow3->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow3;
    }
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow4->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow4;
    }

    // make node visible
    if (selectedRenderWindow)
    {
      mitk::Point3D centerP = markerGeometry->GetOrigin();
       selectedRenderWindow->GetSliceNavigationController()->SelectSliceByPoint(
          centerP);
      selectedRenderWindow->GetSliceNavigationController()->ReorientSlices(
          centerP, markerGeometry->GetNormal());

    }
}


mitk::DataNode::Pointer QmitkSegmentationView::FindFirstRegularImage( std::vector<mitk::DataNode*> nodes )
{
  if (nodes.empty()) return NULL;

  for(unsigned int i = 0; i < nodes.size(); ++i)
  {
      //mitk::DataNode::Pointer node = i.value()
      bool isImage(false);
      if (nodes.at(i)->GetData())
      {
        isImage = dynamic_cast<mitk::Image*>(nodes.at(i)->GetData()) != NULL;
      }

      // make sure this is not a binary image
      bool isSegmentation(false);
      nodes.at(i)->GetBoolProperty("binary", isSegmentation);

      // return first proper mitk::Image
      if (isImage && !isSegmentation) return nodes.at(i);
  }

  return NULL;
}


mitk::DataNode::Pointer QmitkSegmentationView::FindFirstSegmentation( std::vector<mitk::DataNode*> nodes )
{
  if (nodes.empty()) return NULL;


  for(unsigned int i = 0; i < nodes.size(); ++i)
  {
      bool isImage(false);
      if (nodes.at(i)->GetData())
      {
        isImage = dynamic_cast<mitk::Image*>(nodes.at(i)->GetData()) != NULL;
      }

      bool isSegmentation(false);
      nodes.at(i)->GetBoolProperty("binary", isSegmentation);

      // return first proper binary mitk::Image
      if (isImage && isSegmentation)
      {
        return nodes.at(i);
      }
  }

  return NULL;
}

void QmitkSegmentationView::SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData)
{
  // called as a result of new BlueBerry selections
  //   tells the ToolManager for manual segmentation about new selections
  //   updates GUI information about what the user should select
  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
  toolManager->SetWorkingData(  const_cast<mitk::DataNode*>(workingData));


  // check original image
  m_Controls->btnNewSegmentation->setEnabled(referenceData != NULL);
  if (referenceData)
  {
    m_Controls->lblReferenceImageSelectionWarning->hide();
  }
  else
  {
    m_Controls->lblReferenceImageSelectionWarning->show();
    m_Controls->lblWorkingImageSelectionWarning->hide();
    m_Controls->lblSegImage->hide();
    m_Controls->lblSegmentation->hide();
  }

  // check, wheter reference image is aligned like render windows. Otherwise display a visible warning (because 2D tools will probably not work)
  CheckImageAlignment();

  // check segmentation
  if (referenceData)
  {
    if (!workingData)
    {
      m_Controls->lblWorkingImageSelectionWarning->show();

      if( m_Controls->widgetStack->currentIndex() == 0 )
      {
        m_Controls->lblSegImage->hide();
        m_Controls->lblSegmentation->hide();
      }
    }
    else
    {
      m_Controls->lblWorkingImageSelectionWarning->hide();

      if( m_Controls->widgetStack->currentIndex() == 0 )
      {
        m_Controls->lblSegmentation->setText( workingData->GetName().c_str() );
        m_Controls->lblSegmentation->show();
        m_Controls->lblSegImage->show();
      }
    }
  }
}

void QmitkSegmentationView::CheckImageAlignment()
{
  bool wrongAlignment(true);

  mitk::DataNode::Pointer node = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( node->GetData() );

    if (image.IsNotNull() && m_MultiWidget)
    {

      wrongAlignment = !(    IsRenderWindowAligned(m_MultiWidget->GetRenderWindow1(), image )
                          && IsRenderWindowAligned(m_MultiWidget->GetRenderWindow2(), image )
                          && IsRenderWindowAligned(m_MultiWidget->GetRenderWindow3(), image )
                        );
    }

    if (wrongAlignment)
    {
      m_Controls->lblAlignmentWarning->show();
    }
  }
}

bool QmitkSegmentationView::IsRenderWindowAligned(QmitkRenderWindow* renderWindow, mitk::Image* image)
{
  if (!renderWindow) return false;

  // for all 2D renderwindows of m_MultiWidget check alignment
  mitk::PlaneGeometry::ConstPointer displayPlane = dynamic_cast<const mitk::PlaneGeometry*>( renderWindow->GetRenderer()->GetCurrentWorldGeometry2D() );
  if (displayPlane.IsNull()) return false;

  int affectedDimension(-1);
  int affectedSlice(-1);
  return mitk::SegTool2D::DetermineAffectedImageSlice( image, displayPlane, affectedDimension, affectedSlice );
}

void QmitkSegmentationView::ForceDisplayPreferencesUponAllImages()
{
  if (!m_Parent || !m_Parent->isVisible()) return;

  // check all images and segmentations in DataStorage:
  // (items in brackets are implicitly done by previous steps)
  // 1.
  //   if  a reference image is selected,
  //     show the reference image
  //     and hide all other images (orignal and segmentation),
  //     (and hide all segmentations of the other original images)
  //     and show all the reference's segmentations
  //   if no reference image is selected, do do nothing
  //
  // 2.
  //   if  a segmentation is selected,
  //     show it
  //     (and hide all all its siblings (childs of the same parent, incl, NULL parent))
  //   if no segmentation is selected, do nothing

  if (!m_Controls) return; // might happen on initialization (preferences loaded)
  mitk::DataNode::Pointer referenceData = m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetReferenceData(0);
  mitk::DataNode::Pointer workingData =   m_Controls->m_ManualToolSelectionBox->GetToolManager()->GetWorkingData(0);

  // 1.
  if (referenceData.IsNotNull())
  {
    // iterate all images
    mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();

    mitk::DataStorage::SetOfObjects::ConstPointer allImages = this->GetDefaultDataStorage()->GetSubset( isImage );
    for ( mitk::DataStorage::SetOfObjects::const_iterator iter = allImages->begin();
          iter != allImages->end();
          ++iter)

    {
      mitk::DataNode* node = *iter;
      // apply display preferences
      ApplyDisplayOptions(node);

      // set visibility
      node->SetVisibility(node == referenceData);
    }
  }

  // 2.
  if (workingData.IsNotNull())
  {
    workingData->SetVisibility(true);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSegmentationView::ApplyDisplayOptions(mitk::DataNode* node)
{
  if (!node) return;

  bool isBinary(false);
  node->GetPropertyValue("binary", isBinary);

  if (isBinary)
  {
    node->SetProperty( "outline binary", mitk::BoolProperty::New( this->GetPreferences()->GetBool("draw outline", true)) );
    node->SetProperty( "outline width", mitk::FloatProperty::New( 2.0 ) );
    node->SetProperty( "opacity", mitk::FloatProperty::New( this->GetPreferences()->GetBool("draw outline", true) ? 1.0 : 0.3 ) );
    node->SetProperty( "volumerendering", mitk::BoolProperty::New( this->GetPreferences()->GetBool("volume rendering", false) ) );
  }
}

void QmitkSegmentationView::CreateQtPartControl(QWidget* parent)
{
  // setup the basic GUI of this view
  m_Parent = parent;

  m_Controls = new Ui::QmitkSegmentationControls;
  m_Controls->setupUi(parent);
  m_Controls->lblWorkingImageSelectionWarning->hide();
  m_Controls->lblAlignmentWarning->hide();
  m_Controls->lblSegImage->hide();
  m_Controls->lblSegmentation->hide();

  m_Controls->refImageSelector->SetDataStorage(this->GetDefaultDataStorage());
  m_Controls->refImageSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));

  if( m_Controls->refImageSelector->GetSelectedNode().IsNotNull() )
    m_Controls->lblReferenceImageSelectionWarning->hide();
  else
    m_Controls->refImageSelector->hide();


  mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
  toolManager->SetDataStorage( *(this->GetDefaultDataStorage()) );
  assert ( toolManager );

  // all part of open source MITK
  m_Controls->m_ManualToolSelectionBox->SetGenerateAccelerators(true);
  m_Controls->m_ManualToolSelectionBox->SetToolGUIArea( m_Controls->m_ManualToolGUIContainer );
  m_Controls->m_ManualToolSelectionBox->SetDisplayedToolGroups("Add Subtract Paint Wipe 'Region Growing' Correction Fill Erase");
  m_Controls->m_ManualToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );

  // available only in the 3M application
  if ( !m_Controls->m_OrganToolSelectionBox->children().count() )
  {
    m_Controls->widgetStack->setItemEnabled( 1, false );
  }
  m_Controls->m_OrganToolSelectionBox->SetToolManager( *toolManager );
  m_Controls->m_OrganToolSelectionBox->SetToolGUIArea( m_Controls->m_OrganToolGUIContainer );
  m_Controls->m_OrganToolSelectionBox->SetDisplayedToolGroups("'Hippocampus left' 'Hippocampus right' 'Lung left' 'Lung right' 'Liver' 'Heart LV' 'Endocard LV' 'Epicard LV' 'Prostate'");
  m_Controls->m_OrganToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );

  // available only in the 3M application
  if ( !m_Controls->m_LesionToolSelectionBox->children().count() )
  {
    m_Controls->widgetStack->setItemEnabled( 2, false );
  }
  m_Controls->m_LesionToolSelectionBox->SetToolManager( *toolManager );
  m_Controls->m_LesionToolSelectionBox->SetToolGUIArea( m_Controls->m_LesionToolGUIContainer );
  m_Controls->m_LesionToolSelectionBox->SetDisplayedToolGroups("'Lymph Node'");
  m_Controls->m_LesionToolSelectionBox->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceData );

  toolManager->NewNodesGenerated +=
    mitk::MessageDelegate<QmitkSegmentationView>( this, &QmitkSegmentationView::NewNodesGenerated );          // update the list of segmentations
  toolManager->NewNodeObjectsGenerated +=
    mitk::MessageDelegate1<QmitkSegmentationView, mitk::ToolManager::DataVectorType*>( this, &QmitkSegmentationView::NewNodeObjectsGenerated );          // update the list of segmentations

  // create signal/slot connections
  connect( m_Controls->refImageSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
           this, SLOT( OnComboBoxSelectionChanged( const mitk::DataNode* ) ) );
  connect( m_Controls->btnNewSegmentation, SIGNAL(clicked()), this, SLOT(CreateNewSegmentation()) );
  connect( m_Controls->CreateSegmentationFromSurface, SIGNAL(clicked()), this, SLOT(CreateSegmentationFromSurface()) );
  connect( m_Controls->m_ManualToolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(ManualToolSelected(int)) );
  connect( m_Controls->widgetStack, SIGNAL(currentChanged(int)), this, SLOT(ToolboxStackPageChanged(int)) );
  //To remember the position of each contour
  connect( m_Controls->cbRememberContourPositions, SIGNAL(stateChanged(int)), this, SLOT(CheckboxRememberContourPositionsStateChanged(int)));


  connect(m_Controls->MaskSurfaces,  SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
           this, SLOT( OnSurfaceSelectionChanged( ) ) );

  m_Controls->MaskSurfaces->SetDataStorage(this->GetDefaultDataStorage());
  m_Controls->MaskSurfaces->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  //// create helper class to provide context menus for segmentations in data manager
  // m_PostProcessing = new QmitkSegmentationPostProcessing(this->GetDefaultDataStorage(), this, m_Parent);

}

//void QmitkSegmentationView::OnPlaneModeChanged(int i)
//{
//  //if plane mode changes, disable all tools
//  if (m_MultiWidget)
//  {
//    mitk::ToolManager* toolManager = m_Controls->m_ManualToolSelectionBox->GetToolManager();
//
//    if (toolManager)
//    {
//      if (toolManager->GetActiveToolID() >= 0)
//      {
//        toolManager->ActivateTool(-1);
//      }
//      else
//      {
//        m_MultiWidget->EnableNavigationControllerEventListening();
//      }
//    }
//  }
//}


// ATTENTION some methods for handling the known list of (organ names, colors) are defined in QmitkSegmentationOrganNamesHandling.cpp

