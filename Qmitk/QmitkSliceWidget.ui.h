/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <mitkSliceNavigationController.h>
#include <QmitkStepperAdapter.h>
#include <QmitkRenderWindow.h>
#include <mitkLevelWindowProperty.h>
#include <vtkRenderer.h>

void QmitkSliceWidget::init()
{
  popUp = new QPopupMenu(this);
  popUp->insertItem("Transversal",0);
  popUp->insertItem("Frontal",1);
  popUp->insertItem("Sagittal",2);

  connect(popUp, SIGNAL(activated(int)), this, SLOT(ChangeView(int)) );
  setPopUpEnabled(false);

  m_DataTreeIterator = NULL;
  m_SlicedGeometry = NULL;
  m_View = mitk::SliceNavigationController::Transversal;

  QHBoxLayout *hlayout;
  hlayout=new QHBoxLayout(container);

  // create Renderer
  m_Renderer=mitk::OpenGLRenderer::New();

  // create widget
  QString composedName("QmitkSliceWidget::");
  if(name()!=NULL)
    composedName+=name();
  else
    composedName+="QmitkGLWidget";
  m_RenderWindow = new QmitkRenderWindow(m_Renderer, container, composedName);
  hlayout->addWidget(m_RenderWindow);

  m_SliceNavigator = new mitk::SliceNavigationController(NULL);
  m_SliceNavigator->SetViewDirection(mitk::SliceNavigationController::Transversal);
  m_SliceNavigator->ConnectGeometrySliceEvent(m_Renderer.GetPointer());
  m_SliceNavigator->ConnectRepaint(m_RenderWindow);
  new QmitkStepperAdapter(m_NavigatorWidget, m_SliceNavigator->GetSlice(), "navigation");  
  
  SetLevelWindowEnabled(true);

}


mitk::OpenGLRenderer* QmitkSliceWidget::GetRenderer()
{
  return m_Renderer;
}


QFrame* QmitkSliceWidget::GetSelectionFrame()
{
  return SelectionFrame;
}


void QmitkSliceWidget::SetData( mitk::DataTreeIteratorBase * it )
{
  SetData(it, m_View);
}

void QmitkSliceWidget::SetData( mitk::DataTreeNode::Pointer node  )
{
  if ( m_DataTree.IsNotNull() )
  {
    m_DataTree = NULL;
  }
  m_DataTree = mitk::DataTree::New();
  mitk::DataTreePreOrderIterator it(m_DataTree);
  it.Add(node);
  SetData(&it, m_View);
}

void QmitkSliceWidget::AddData( mitk::DataTreeNode::Pointer node)
{
  if ( m_DataTree.IsNull() )
  {
    m_DataTree = mitk::DataTree::New();
  }
  mitk::DataTreePreOrderIterator it(m_DataTree);
  it.Add( node );
  SetData(&it, m_View);
}


void QmitkSliceWidget::SetData( mitk::DataTreeIteratorBase* it, mitk::SliceNavigationController::ViewDirection view )
{
  m_DataTreeIterator = it;
  mitk::DataTreeIteratorClone tmpIterator=m_DataTreeIterator.GetPointer();

  while(!tmpIterator->IsAtEnd()) 
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(tmpIterator->Get()->GetData());
    if(image.IsNotNull() && tmpIterator->Get()->IsVisible(GetRenderer()))
    {
      m_SlicedGeometry = image->GetSlicedGeometry();

      mitk::LevelWindow picLevelWindow;
      tmpIterator->Get()->GetLevelWindow(picLevelWindow,NULL);

      levelWindow->setLevelWindow(picLevelWindow);

      GetRenderer()->SetData(tmpIterator.GetPointer());
//      tmpIterator->Get()->SetLevelWindow(levelWindow, NULL );
      break;
    }
 
    ++tmpIterator;
  }
  InitWidget(view);
}

void QmitkSliceWidget::InitWidget( mitk::SliceNavigationController::ViewDirection viewDirection )
{
  m_View = viewDirection;


  if (viewDirection == mitk::SliceNavigationController::Transversal)
  {
    m_SliceNavigator->SetViewDirection(mitk::SliceNavigationController::Transversal);
  }
  else if (viewDirection == mitk::SliceNavigationController::Frontal)
  {  
    m_SliceNavigator->SetViewDirection(mitk::SliceNavigationController::Frontal);
  }
  // init sagittal view
  else 
  {
    m_SliceNavigator->SetViewDirection(mitk::SliceNavigationController::Sagittal);
  }    


  int currentPos = 0;
  if (m_SliceNavigator.IsNotNull())
  {
    currentPos = m_SliceNavigator->GetSlice()->GetPos();
  }

  if (m_DataTreeIterator.IsNull() || m_SlicedGeometry.IsNull() ) 
  {
    return;
  }

  // compute bounding box with respect to first images geometry
  const mitk::BoundingBox::BoundsArrayType imageBounds = m_SlicedGeometry->GetBoundingBox()->GetBounds();

//  mitk::SlicedGeometry3D::Pointer correctGeometry = m_SlicedGeometry.GetPointer();
  mitk::Geometry3D::Pointer geometry = m_SlicedGeometry.GetPointer();

  const mitk::BoundingBox::Pointer boundingbox = mitk::DataTree::ComputeVisibleBoundingBox(m_DataTreeIterator.GetPointer(), NULL, "includeInBoundingBox");
  if(boundingbox->GetPoints()->Size()>0)
  {
////    geometry = mitk::Geometry3D::New();
////    geometry->Initialize();
//    geometry->SetBounds(boundingbox->GetBounds());
//    geometry->SetSpacing(correctGeometry->GetSpacing());

    //lets see if we have data with a limited live-span ...
    mitk::TimeBounds timebounds = mitk::DataTree::ComputeTimeBoundsInMS(m_DataTreeIterator.GetPointer(), NULL, "includeInBoundingBox");
    if(timebounds[1]<mitk::ScalarTypeNumericTraits::max())
    {
      mitk::ScalarType duration = timebounds[1]-timebounds[0];

      mitk::TimeSlicedGeometry::Pointer timegeometry = mitk::TimeSlicedGeometry::New();
      timegeometry->InitializeEvenlyTimed(m_SlicedGeometry.GetPointer(), (unsigned int) duration);
      timegeometry->SetTimeBoundsInMS(timebounds); //@bug really required? FIXME

      timebounds[1] = timebounds[0]+1.0f;
      geometry->SetTimeBoundsInMS(timebounds);

      geometry=timegeometry;
    }

    if(const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2()>=mitk::eps)
    {
      m_SliceNavigator->SetInputWorldGeometry(geometry); 
      m_SliceNavigator->Update();
    }
  }

  GetRenderer()->GetDisplayGeometry()->Fit();
  GetRenderer()->GetRenderWindow()->Repaint();
  GetRenderer()->Update();
  //int w=vtkObject::GetGlobalWarningDisplay();
  //vtkObject::GlobalWarningDisplayOff();  
  //vtkRenderer * vtkrenderer = ((mitk::OpenGLRenderer*)(GetRenderer()))->GetVtkRenderer();
  //if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  //vtkObject::SetGlobalWarningDisplay(w);

  
}

void QmitkSliceWidget::ChangeLevelWindow(mitk::LevelWindow* lw )
{
  if ( m_DataTreeIterator.IsNull() )
  {
    return;
  }

  mitk::DataTreeIteratorClone it = m_DataTreeIterator.GetPointer();
  while(!it->IsAtEnd()) 
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(it->Get()->GetData());
    if( image.IsNotNull() )
    {
      mitk::LevelWindowProperty::Pointer levWinProp = 
        dynamic_cast<mitk::LevelWindowProperty*>(it->Get()->GetProperty("levelwindow", NULL).GetPointer());

      if (levWinProp.IsNotNull()) 
      {
        mitk::LevelWindow levWin = levWinProp->GetLevelWindow();
        levWin.SetMin(lw->GetMin());
        levWin.SetMax(lw->GetMax());
//        levWin.SetRangeMin(lw->GetRangeMin());
//        levWin.SetRangeMax(lw->GetRangeMax());

        levWinProp->SetLevelWindow(levWin);
        it->Get()->SetLevelWindow(levWin, NULL);
      }
    }
    ++it;
  }
  GetRenderer()->GetRenderWindow()->Repaint();
}


void QmitkSliceWidget::UpdateGL()
{
  GetRenderer()->GetDisplayGeometry()->Fit();
  GetRenderer()->GetRenderWindow()->Repaint();
}

void QmitkSliceWidget::mousePressEvent( QMouseEvent * e )
{
  if (e->button() == Qt::RightButton && popUpEnabled) 
  {
    popUp->popup(QCursor::pos());
  }
}

void QmitkSliceWidget::wheelEvent( QWheelEvent * e )
{
  int val = m_NavigatorWidget->GetPos();

  if (e->orientation() * e->delta()  > 0) 
  {
    m_NavigatorWidget->SetPos( val+1 );
  }
  else 
  {
    if (val > 0)
      m_NavigatorWidget->SetPos( val-1 );
  }
}

void QmitkSliceWidget::ChangeView(int val)
{
  if (val == 0)
  {
    InitWidget(mitk::SliceNavigationController::Transversal);
  }
  else if (val == 1)
  {
    InitWidget(mitk::SliceNavigationController::Frontal);
  }
  else if (val == 2)
  {
    InitWidget(mitk::SliceNavigationController::Sagittal);
  }
}


void QmitkSliceWidget::setPopUpEnabled( bool b )
{
  popUpEnabled = b;
}

mitk::DataTreeIteratorBase* QmitkSliceWidget::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

mitk::DataTree::Pointer QmitkSliceWidget::GetDataTree()
{
  return m_DataTree;
}


QmitkSliderNavigator* QmitkSliceWidget::GetNavigatorWidget()
{
  return m_NavigatorWidget;
}


mitk::SliceNavigationController::Pointer QmitkSliceWidget::GetSliceController()
{
  return m_SliceNavigator;
}


void QmitkSliceWidget::SetLevelWindowEnabled( bool enable )
{
    levelWindow->setEnabled(enable);
    if (!enable)
    {
      levelWindow->setMinimumWidth(0);
      levelWindow->setMaximumWidth(0);
    }
    else
    {
     levelWindow->setMinimumWidth(28);
     levelWindow->setMaximumWidth(28);
    }
}


bool QmitkSliceWidget::IsLevelWindowEnabled()
{
  return levelWindow->isEnabled();
}


QmitkRenderWindow* QmitkSliceWidget::GetRenderWindow()
{
    return m_RenderWindow;
}
