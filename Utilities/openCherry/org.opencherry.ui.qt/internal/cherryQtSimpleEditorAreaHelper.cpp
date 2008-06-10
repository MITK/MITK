/*=========================================================================
 
Program:   openCherry Platform
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

#include "cherryQtSimpleEditorAreaHelper.h"

#include <org.opencherry.ui/src/cherryIEditorSite.h>
#include <org.opencherry.ui/src/internal/cherryPartSite.h>
#include <org.opencherry.ui/src/internal/cherryEditorReference.h>
#include <org.opencherry.ui/src/internal/cherryWorkbenchPage.h>

#include "../cherryQtSimpleWorkbenchWindow.h"

namespace cherry
{

QtSimpleEditorAreaHelper::QtSimpleEditorAreaHelper(IWorkbenchPage::Pointer page)
 { 
   editorArea = new QTabWidget(static_cast<QWidget*>(page.Cast<WorkbenchPage>()->GetClientComposite()));
   QtWorkbenchWindow::Pointer qtWindow = page->GetWorkbenchWindow().Cast<QtWorkbenchWindow>();
   qtWindow->setCentralWidget(editorArea);
 }
 
 QTabWidget* QtSimpleEditorAreaHelper::GetControl()
 {
   return editorArea;
 }

 void QtSimpleEditorAreaHelper::CloseEditor(IEditorReference::Pointer ref)
 {
   PartPane::Pointer pane = ref.Cast<WorkbenchPartReference>()->GetPane();
   this->CloseEditor(pane);
 }

 void QtSimpleEditorAreaHelper::CloseEditor(IEditorPart::Pointer part)
 {
   PartPane::Pointer pane = part->GetEditorSite().Cast<PartSite>()->GetPane();
   this->CloseEditor(pane);
 }

 LayoutPart::Pointer QtSimpleEditorAreaHelper::GetLayoutPart()
 {
   //return editorArea;
   return 0;
 }

 IEditorReference::Pointer QtSimpleEditorAreaHelper::GetVisibleEditor()
 {
   int index = editorArea->currentIndex();
   std::list<PartPane::Pointer>::iterator pane = editorList.begin();
   std::advance(pane, index);
   if (pane->IsNotNull())
   {
     IEditorReference::Pointer result = (*pane)->GetPartReference().Cast<IEditorReference>();
     return result;
   }
   return 0;
 }

 void QtSimpleEditorAreaHelper::AddEditor(IEditorReference::Pointer ref)
 {
   for (std::list<PartPane::Pointer>::iterator pane = editorList.begin();
        pane != editorList.end(); ++pane)
   {
     if ((*pane)->GetPartReference() == ref)
     {
       return;
     }
   }

   this->AddToLayout(ref.Cast<EditorReference>()->GetPane());

   //editorArea.getPage().partAdded(ref);
 }

 bool QtSimpleEditorAreaHelper::SetVisibleEditor(IEditorReference::Pointer ref, bool setFocus)
 {
//    IEditorReference visibleEditor = getVisibleEditor();
//    if (ref != visibleEditor)
//    {
//      IEditorPart part = (IEditorPart) ref.getPart(true);
//      EditorPane pane = null;
//      if (part != null)
//      {
//        pane = (EditorPane) ((PartSite) part.getEditorSite()).getPane();
//      }
//      if (pane != null)
//      {
//
//        pane.getWorkbook().setSelection(pane);
//
//        if (setFocus)
//        {
//          part.setFocus();
//        }
//        return true;
//      }
//    }
//    return false;
   return true;
 }

 std::vector<IEditorReference::Pointer> QtSimpleEditorAreaHelper::GetEditors()
 {
   //            List result = new ArrayList();
   //            List workbooks = editorArea.getEditorWorkbooks();
   //            
   //            for (Iterator iter = workbooks.iterator(); iter.hasNext();) {
   //                PartStack stack = (PartStack) iter.next();
   //                
   //                LayoutPart[] children = stack.getChildren();
   //                
   //                for (int i = 0; i < children.length; i++) {
   //                    LayoutPart part = children[i];
   //                    
   //                    result.add(((PartPane)part).getPartReference());
   //                }
   //            }
   //            
   //            return (IEditorReference[]) result.toArray(new IEditorReference[result.size()]);
   
   std::vector<IEditorReference::Pointer> result;
   for (std::list<PartPane::Pointer>::iterator pane = editorList.begin();
            pane != editorList.end(); ++pane)
   {
     result.push_back((*pane)->GetPartReference().Cast<IEditorReference>());
   }
   return result;
 }

 void QtSimpleEditorAreaHelper::CloseEditor(PartPane::Pointer pane)
 {
   if (pane.IsNotNull())
   {
     int index = -1;
     int i = 0;
     for (std::list<PartPane::Pointer>::iterator iter = editorList.begin();
              iter != editorList.end(); ++iter, ++i)
     {
       if (pane == *iter)
       {
         index = i;
         break;
       }
     }
     
     if (index > -1)
     {
       editorArea->removeTab(i);
       editorList.remove(pane);
     }
   }
 }

 void QtSimpleEditorAreaHelper::AddToLayout(PartPane::Pointer pane)
 {
   pane->CreateControl(0);
   QWidget* tabWidget = static_cast<QWidget*>(pane->GetControl());
   editorArea->addTab(tabWidget, QString(pane->GetPartReference()->GetPartName().c_str()));
 }

}
