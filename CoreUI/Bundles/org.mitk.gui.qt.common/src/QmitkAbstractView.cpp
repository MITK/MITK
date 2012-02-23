/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkAbstractView.h"
#include "QmitkDataNodeSelectionProvider.h"
#include "internal/QmitkCommonActivator.h"
#include "internal/QmitkDataNodeItemModel.h"

// mitk Includes
#include <mitkLogMacros.h>
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>
#include <mitkWorkbenchUtil.h>
#include <mitkDataNodeObject.h>
#include <mitkIRenderingManager.h>

// berry Includes
#include <berryIWorkbenchPage.h>
#include <berryIBerryPreferences.h>
#include <berryIEditorPart.h>

// CTK Includes
#include <ctkServiceTracker.h>

// Qt Includes
#include <QItemSelectionModel>
#include <QApplication>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

class QmitkAbstractViewPrivate
{
public:

  QmitkAbstractViewPrivate(QmitkAbstractView* qq)
    : q(qq)
    , m_PrefServiceTracker(QmitkCommonActivator::GetContext())
    , m_DataStorageServiceTracker(QmitkCommonActivator::GetContext())
    , m_Parent(0)
    , m_DataNodeItemModel(new QmitkDataNodeItemModel)
    , m_DataNodeSelectionModel(new QItemSelectionModel(m_DataNodeItemModel))
    , m_InDataStorageChanged(false)
  {
    m_PrefServiceTracker.open();
    m_DataStorageServiceTracker.open();
  }

  ~QmitkAbstractViewPrivate()
  {
    delete m_DataNodeSelectionModel;
    delete m_DataNodeItemModel;

    m_PrefServiceTracker.close();
    m_DataStorageServiceTracker.close();
  }

  /**
   * Called when a DataStorage Add Event was thrown. Sets
   * m_InDataStorageChanged to true and calls NodeAdded afterwards.
   * \see m_InDataStorageChanged
   */
  void NodeAddedProxy(const mitk::DataNode* node)
  {
    // garantuee no recursions when a new node event is thrown in NodeAdded()
    if(!m_InDataStorageChanged)
    {
      m_InDataStorageChanged = true;
      q->NodeAdded(node);
      q->DataStorageModified();
      m_InDataStorageChanged = false;
    }
  }

  /**
   * Called when a DataStorage remove event was thrown. Sets
   * m_InDataStorageChanged to true and calls NodeRemoved afterwards.
   * \see m_InDataStorageChanged
   */
  void NodeRemovedProxy(const mitk::DataNode* node)
  {
    // garantuee no recursions when a new node event is thrown in NodeAdded()
    if(!m_InDataStorageChanged)
    {
      m_InDataStorageChanged = true;
      q->NodeRemoved(node);
      q->DataStorageModified();
      m_InDataStorageChanged = false;
    }
  }

  /**
   * Called when a DataStorage changed event was thrown. Sets
   * m_InDataStorageChanged to true and calls NodeChanged afterwards.
   * \see m_InDataStorageChanged
   */
  void NodeChangedProxy(const mitk::DataNode* node)
  {
    // garantuee no recursions when a new node event is thrown in NodeAdded()
    if(!m_InDataStorageChanged)
    {
      m_InDataStorageChanged = true;
      q->NodeChanged(node);
      q->DataStorageModified();
      m_InDataStorageChanged = false;
    }
  }

  /**
   * reactions to selection events from data manager (and potential other senders)
   */
  void BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
  {
    if(sourcepart.IsNull() || sourcepart->GetSite()->GetId() != "org.mitk.views.datamanager")
      return;

    mitk::DataNodeSelection::ConstPointer _DataNodeSelection
      = selection.Cast<const mitk::DataNodeSelection>();
    q->OnSelectionChanged(sourcepart, this->DataNodeSelectionToQList(_DataNodeSelection));
  }

  /**
   * Converts a mitk::DataNodeSelection to a QList<mitk::DataNode::Pointer> (possibly empty)
   */
  QList<mitk::DataNode::Pointer> DataNodeSelectionToQList(mitk::DataNodeSelection::ConstPointer currentSelection) const;

  QmitkAbstractView* const q;

  ctkServiceTracker<berry::IPreferencesService*> m_PrefServiceTracker;

  ctkServiceTracker<mitk::IDataStorageService*> m_DataStorageServiceTracker;

  /**
   * Saves the parent of this view (this is the scrollarea created in CreatePartControl(void*)
   * \see CreatePartControl(void*)
   */
  QWidget* m_Parent;

  /**
   * Holds the current selection (selection made by this View !!!)
   */
  QmitkDataNodeSelectionProvider::Pointer m_SelectionProvider;

  /**
   * Holds a helper model for firing selection events.
   */
  QmitkDataNodeItemModel* m_DataNodeItemModel;

  /**
   * The selection model for the QmitkDataNodeItemModel;
   */
  QItemSelectionModel* m_DataNodeSelectionModel;

  /**
   * object to observe BlueBerry selections
   */
  berry::ISelectionListener::Pointer m_BlueBerrySelectionListener;

  /**
   * Saves if this class is currently working on DataStorage changes.
   * This is a protector variable to avoid recursive calls on event listener functions.
   */
  bool m_InDataStorageChanged;

};

QmitkAbstractView::QmitkAbstractView()
  : d(new QmitkAbstractViewPrivate(this))
{
}

void QmitkAbstractView::CreatePartControl(void* parent)
{

  // scrollArea
  QScrollArea* scrollArea = new QScrollArea;  
  //QVBoxLayout* scrollAreaLayout = new QVBoxLayout(scrollArea);
  scrollArea->setFrameShadow(QFrame::Plain);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // m_Parent
  d->m_Parent = new QWidget;
  //m_Parent->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
  this->CreateQtPartControl(d->m_Parent);

  //scrollAreaLayout->addWidget(m_Parent);
  //scrollArea->setLayout(scrollAreaLayout);

  // set the widget now
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(d->m_Parent);

  // add the scroll area to the real parent (the view tabbar)
  QWidget* parentQWidget = static_cast<QWidget*>(parent);
  QVBoxLayout* parentLayout = new QVBoxLayout(parentQWidget);
  parentLayout->setMargin(0);
  parentLayout->setSpacing(0);
  parentLayout->addWidget(scrollArea);

  // finally set the layout containing the scroll area to the parent widget (= show it)
  parentQWidget->setLayout(parentLayout);

  this->AfterCreateQtPartControl();
}

void QmitkAbstractView::AfterCreateQtPartControl()
{
  this->SetSelectionProvider();

  // REGISTER DATASTORAGE LISTENER
  this->GetDataStorage()->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkAbstractViewPrivate, const mitk::DataNode*>
                                                    ( d.data(), &QmitkAbstractViewPrivate::NodeAddedProxy ) );
  this->GetDataStorage()->ChangedNodeEvent.AddListener( mitk::MessageDelegate1<QmitkAbstractViewPrivate, const mitk::DataNode*>
                                                        ( d.data(), &QmitkAbstractViewPrivate::NodeChangedProxy ) );
  this->GetDataStorage()->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkAbstractViewPrivate, const mitk::DataNode*>
                                                       ( d.data(), &QmitkAbstractViewPrivate::NodeRemovedProxy ) );

  // REGISTER PREFERENCES LISTENER
  berry::IBerryPreferences::Pointer prefs = this->GetPreferences().Cast<berry::IBerryPreferences>();
  if(prefs.IsNotNull())
    prefs->OnChanged.AddListener(
          berry::MessageDelegate1<QmitkAbstractView, const berry::IBerryPreferences*>(this,
                                                                              &QmitkAbstractView::OnPreferencesChanged));

  // REGISTER FOR WORKBENCH SELECTION EVENTS
  d->m_BlueBerrySelectionListener = berry::ISelectionListener::Pointer(
        new berry::SelectionChangedAdapter<QmitkAbstractViewPrivate>(d.data(),
                                                             &QmitkAbstractViewPrivate::BlueBerrySelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ d->m_BlueBerrySelectionListener);
  
  // EMULATE INITIAL SELECTION EVENTS

  // send the current selection
  berry::IWorkbenchPart::Pointer activePart = this->GetSite()->GetPage()->GetActivePart();
  if (activePart.IsNotNull())
  {
    this->OnSelectionChanged(activePart, this->GetCurrentSelection());
  }

  // send preferences changed event
  this->OnPreferencesChanged(this->GetPreferences().Cast<berry::IBerryPreferences>().GetPointer());
}

QmitkAbstractView::~QmitkAbstractView()
{
  this->Register();

  this->GetDataStorage()->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkAbstractViewPrivate, const mitk::DataNode*>
                                                       ( d.data(), &QmitkAbstractViewPrivate::NodeAddedProxy ) );
  this->GetDataStorage()->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkAbstractViewPrivate, const mitk::DataNode*>
                                                          ( d.data(), &QmitkAbstractViewPrivate::NodeRemovedProxy) );
  this->GetDataStorage()->ChangedNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkAbstractViewPrivate, const mitk::DataNode*>
                                                           ( d.data(), &QmitkAbstractViewPrivate::NodeChangedProxy ) );

  berry::IBerryPreferences::Pointer prefs = this->GetPreferences().Cast<berry::IBerryPreferences>();
  if(prefs.IsNotNull())
  {
    prefs->OnChanged.RemoveListener(
          berry::MessageDelegate1<QmitkAbstractView, const berry::IBerryPreferences*>(this,
                                                                              &QmitkAbstractView::OnPreferencesChanged));
    // flush the preferences here (disabled, everyone should flush them by themselves at the right moment)
    // prefs->Flush();
  }

  // REMOVE SELECTION PROVIDER
  this->GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(NULL));

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
  {
    s->RemovePostSelectionListener(d->m_BlueBerrySelectionListener);
  }

  this->UnRegister(false);
}

void QmitkAbstractView::SetSelectionProvider()
{
  // REGISTER A SELECTION PROVIDER
  d->m_SelectionProvider = QmitkDataNodeSelectionProvider::Pointer(new QmitkDataNodeSelectionProvider);
  d->m_SelectionProvider->SetItemSelectionModel(GetDataNodeSelectionModel());
  this->GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(d->m_SelectionProvider));
}

QItemSelectionModel *QmitkAbstractView::GetDataNodeSelectionModel() const
{
  return d->m_DataNodeSelectionModel;
}

void QmitkAbstractView::OnPreferencesChanged( const berry::IBerryPreferences* )
{
}

void QmitkAbstractView::DataStorageModified()
{
}

void QmitkAbstractView::DataStorageChanged(mitk::IDataStorageReference::Pointer /*dsRef*/)
{
}

mitk::IRenderWindowPart* QmitkAbstractView::GetRenderWindowPart( IRenderWindowPartStrategies strategies ) const
{
  berry::IWorkbenchPage::Pointer page = this->GetSite()->GetPage();

  // Return the active editor if it implements mitk::IRenderWindowPart
  mitk::IRenderWindowPart* renderPart =
      dynamic_cast<mitk::IRenderWindowPart*>(page->GetActiveEditor().GetPointer());
  if (renderPart) return renderPart;

  // No suitable active editor found, check visible editors
  std::list<berry::IEditorReference::Pointer> editors = page->GetEditorReferences();
  for (std::list<berry::IEditorReference::Pointer>::iterator i = editors.begin();
       i != editors.end(); ++i)
  {
    berry::IWorkbenchPart::Pointer part = (*i)->GetPart(false);
    if (page->IsPartVisible(part))
    {
      renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
      if (renderPart) return renderPart;
    }
  }

  // No suitable visible editor found, check visible views
  std::vector<berry::IViewReference::Pointer> views = page->GetViewReferences();
  for(std::vector<berry::IViewReference::Pointer>::iterator i = views.begin();
      i != views.end(); ++i)
  {
    berry::IWorkbenchPart::Pointer part = (*i)->GetPart(false);
    if (page->IsPartVisible(part))
    {
      renderPart = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
      if (renderPart) return renderPart;
    }
  }

  // No strategies given
  if (strategies == NONE) return 0;

  mitk::DataStorageEditorInput::Pointer input(new mitk::DataStorageEditorInput(GetDataStorageReference()));

  bool activate = false;
  if(strategies & ACTIVATE)
  {
    activate = true;
  }

  berry::IEditorPart::Pointer editorPart;

  if(strategies & OPEN)
  {
    // This will create a default editor for the given input. If an editor
    // with that input is already open, the editor is brought to the front.
    editorPart = mitk::WorkbenchUtil::OpenEditor(page, input, activate);
  }
  else if (activate || (strategies & BRING_TO_FRONT))
  {
    // check if a suitable editor is already opened
    editorPart = page->FindEditor(input);
    if (activate)
    {
      page->Activate(editorPart);
    }
    else
    {
      page->BringToTop(editorPart);
    }
  }

  return dynamic_cast<mitk::IRenderWindowPart*>(editorPart.GetPointer());
}

void QmitkAbstractView::RequestRenderWindowUpdate(mitk::RenderingManager::RequestType requestType)
{
  mitk::IRenderWindowPart* renderPart = this->GetRenderWindowPart();
  if (renderPart == 0) return;

  if (mitk::IRenderingManager* renderingManager = renderPart->GetRenderingManager())
  {
    renderingManager->RequestUpdateAll(requestType);
  }
  else
  {
    renderPart->RequestUpdate(requestType);
  }
}

void QmitkAbstractView::HandleException( const char* str, QWidget* parent, bool showDialog ) const
{
  //itkGenericOutputMacro( << "Exception caught: " << str );
  MITK_ERROR << str;
  if ( showDialog )
  {
    QMessageBox::critical ( parent, "Exception caught!", str );
  }
}

void QmitkAbstractView::HandleException( std::exception& e, QWidget* parent, bool showDialog ) const
{
  HandleException( e.what(), parent, showDialog );
}

void QmitkAbstractView::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkAbstractView::BusyCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
}

void QmitkAbstractView::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkAbstractView::BusyCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkAbstractView::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

berry::IPreferences::Pointer QmitkAbstractView::GetPreferences() const
{
  berry::IPreferencesService* prefService = d->m_PrefServiceTracker.getService();
  // const_cast workaround for bad programming: const uncorrectness this->GetViewSite() should be const
  std::string id = "/" + (const_cast<QmitkAbstractView*>(this))->GetViewSite()->GetId();
  return prefService ? prefService->GetSystemPreferences()->Node(id): berry::IPreferences::Pointer(0);
}

mitk::DataStorage::Pointer
QmitkAbstractView::GetDataStorage() const
{
  mitk::IDataStorageService* dsService = d->m_DataStorageServiceTracker.getService();

  if (dsService != 0)
  {
    return dsService->GetDataStorage()->GetDataStorage();
  }

  return 0;
}

mitk::IDataStorageReference::Pointer QmitkAbstractView::GetDataStorageReference() const
{
  mitk::IDataStorageService* dsService = d->m_DataStorageServiceTracker.getService();

  if (dsService != 0)
  {
    return dsService->GetDataStorage();
  }

  return mitk::IDataStorageReference::Pointer(0);
}

QList<mitk::DataNode::Pointer> QmitkAbstractView::GetCurrentSelection() const
{
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  // buffer for the data manager selection
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
  return d->DataNodeSelectionToQList(currentSelection);
}

QList<mitk::DataNode::Pointer> QmitkAbstractView::GetDataManagerSelection() const
{
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    // buffer for the data manager selection
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
  return d->DataNodeSelectionToQList(currentSelection);
}

void QmitkAbstractView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/,
                                           const QList<mitk::DataNode::Pointer>& /*nodes*/)
{
}

QList<mitk::DataNode::Pointer> QmitkAbstractViewPrivate::DataNodeSelectionToQList(mitk::DataNodeSelection::ConstPointer currentSelection) const
{
  QList<mitk::DataNode::Pointer> selectedNodes;
  if(currentSelection.IsNull())
    return selectedNodes;

  mitk::DataNodeObject::Pointer _DataNodeObject;
  mitk::DataNode::Pointer _DataNode;

  for(mitk::DataNodeSelection::iterator it = currentSelection->Begin();
    it != currentSelection->End(); ++it)
  {
    _DataNodeObject = it->Cast<mitk::DataNodeObject>();
    if(_DataNodeObject.IsNotNull())
    {
      _DataNode = _DataNodeObject->GetDataNode();
      if(_DataNode.IsNotNull())
        selectedNodes << _DataNode;
    }
  }

  return selectedNodes;
}


void QmitkAbstractView::NodeAdded( const mitk::DataNode*  /*node*/ )
{
}

void QmitkAbstractView::NodeRemoved( const mitk::DataNode*  /*node*/ )
{
}

void QmitkAbstractView::NodeChanged( const mitk::DataNode* /*node*/ )
{
}

void QmitkAbstractView::FireNodeSelected( mitk::DataNode::Pointer node )
{
  QList<mitk::DataNode::Pointer> nodes;
  nodes << node;
  this->FireNodesSelected(nodes);
}

void QmitkAbstractView::FireNodesSelected( const QList<mitk::DataNode::Pointer>& nodes )
{
  if (nodes.empty())
  {
    d->m_DataNodeSelectionModel->clearSelection();
    d->m_DataNodeItemModel->clear();
    return;
  }

  // The helper data node model is just used for sending selection events.
  // We add the to be selected nodes and set the selection range to everything.

  d->m_DataNodeItemModel->clear();
  foreach(mitk::DataNode::Pointer node, nodes)
  {
    d->m_DataNodeItemModel->AddDataNode(node);
  }
  d->m_DataNodeSelectionModel->select(QItemSelection(d->m_DataNodeItemModel->index(0,0), d->m_DataNodeItemModel->index(nodes.size(), 0)),
                                      QItemSelectionModel::ClearAndSelect);
}
