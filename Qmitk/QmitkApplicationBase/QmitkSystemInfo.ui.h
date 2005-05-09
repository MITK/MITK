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

#include <mitkRenderWindow.h>
#include <mitkBaseRenderer.h>

#include <mitkGlobalInteraction.h>
#include <mitkFocusManager.h>

#include <qapplication.h>
#include <qclipboard.h>

void QmitkSystemInfo::init()
{
  m_UpdateRendererListCommand = itk::SimpleMemberCommand<QmitkSystemInfo>::New();
  m_UpdateRendererListCommand->SetCallbackFunction(this, &QmitkSystemInfo::UpdateRendererList);

  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetGlobalInteraction();
  if (globalInteraction)
  {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    mitk::FocusEvent fe;
    fm->AddObserver(fe, m_UpdateRendererListCommand);
  }
  UpdateRendererList();
}

void QmitkSystemInfo::UpdateRendererList()
{
  mitk::RenderWindow* focusedRenderWindow = NULL;

  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetGlobalInteraction();
  if (globalInteraction)
  {
    mitk::FocusManager* fm = globalInteraction->GetFocusManager();
    mitk::BaseRenderer::ConstPointer br = fm->GetFocused();
    if (br.IsNotNull())
    {
      focusedRenderWindow = br->GetRenderWindow();
    }
  }

  int selectedItem = -1;
  int itemNumber = 0;
  m_lbRenderer->clear();
  const mitk::RenderWindow::RenderWindowSet rws = mitk::RenderWindow::GetInstances();
  for (mitk::RenderWindow::RenderWindowSet::const_iterator iter = rws.begin();iter != rws.end();++iter,++itemNumber)
  {
    if ((*iter)->GetName())
    {
      m_lbRenderer->insertItem(QString((*iter)->GetName()));
      if(focusedRenderWindow==(*iter))
        selectedItem = itemNumber;
    }

  }
  if (selectedItem>=0)
  {
    m_lbRenderer->setCurrentItem(selectedItem);
  }
  else
  {
    m_lbRenderer->clearSelection();
  }
}

void QmitkSystemInfo::AddRendererInfo()
{
  int selectedItem = m_lbRenderer->currentItem() ;
  int itemNumber = -1;
  const mitk::RenderWindow::RenderWindowSet rws = mitk::RenderWindow::GetInstances();
  mitk::RenderWindow::RenderWindowSet::const_iterator iter;
  for (iter = rws.begin();iter != rws.end();++iter)
  {
    ++itemNumber;
    if(itemNumber==selectedItem)
      break;
  }
  if(itemNumber==selectedItem)
  {
    ::itk::OStringStream itkOutput;
    (*iter)->GetRenderer()->Print(itkOutput);
    m_teOutputPane->append("------------------RendererInfo------------------");
    m_teOutputPane->append((*iter)->GetName());
    m_teOutputPane->append(itkOutput.str().c_str());
    m_teOutputPane->append((*iter)->GetName());
    m_teOutputPane->append("--------------End of RendererInfo---------------");
  }
}

void QmitkSystemInfo::CopyToClipboard()
{
  QApplication::clipboard()->setText(m_teOutputPane->text(), QClipboard::Clipboard);
}

void QmitkSystemInfo::Clear()
{
  m_teOutputPane->clear();
}
