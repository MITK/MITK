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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryIPerspectiveRegistry.h>
#include <berryPlatformUI.h>

// Qmitk
#include "ViewBrowserView.h"

// Qt
#include <QMessageBox>

//mitk image
#include <mitkImage.h>

const std::string ViewBrowserView::VIEW_ID = "org.mitk.views.viewbrowser";

void ViewBrowserView::SetFocus()
{
    m_Controls.buttonPerformImageProcessing->setFocus();
}

void ViewBrowserView::CreateQtPartControl( QWidget *parent )
{
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls.setupUi( parent );
    connect( m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoSomething()) );
}

void ViewBrowserView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
    // iterate all selected objects, adjust warning visibility
    foreach( mitk::DataNode::Pointer node, nodes )
    {
        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
            return;
        }
    }
}


void ViewBrowserView::DoSomething()
{
    berry::IPerspectiveRegistry* perspRegistry = berry::PlatformUI::GetWorkbench()->GetPerspectiveRegistry();
//    QActionGroup* perspGroup = new QActionGroup(menuBar);

    std::vector<berry::IPerspectiveDescriptor::Pointer> perspectives(perspRegistry->GetPerspectives());
    bool skip = false;

    MITK_INFO << "PERSPECTIVES";
    for (unsigned int i=0; i<perspectives.size(); i++)
    {
        berry::IPerspectiveDescriptor::Pointer p = perspectives.at(i);
        MITK_INFO << p->GetId();
        // if perspectiveExcludeList is set, it contains the id-strings of perspectives, which
        // should not appear as an menu-entry in the perspective menu
//        if (perspectiveExcludeList.size() > 0)
//        {
//            for (unsigned int i=0; i<perspectiveExcludeList.size(); i++)
//            {
//                if (perspectiveExcludeList.at(i) == (*perspIt)->GetId())
//                {
//                    skip = true;
//                    break;
//                }
//            }
//            if (skip)
//            {
//                skip = false;
//                continue;
//            }
//        }

//        QAction* perspAction = new berry::QtOpenPerspectiveAction(window,
//                                                                  *perspIt, perspGroup);
//        mapPerspIdToAction.insert(std::make_pair((*perspIt)->GetId(), perspAction));
    }

    MITK_INFO << "VIEWS";
    berry::IViewRegistry* viewRegistry = berry::PlatformUI::GetWorkbench()->GetViewRegistry();
    std::vector<berry::IViewDescriptor::Pointer> views(viewRegistry->GetViews());
    for (unsigned int i=0; i<views.size(); i++)
    {
        berry::IViewDescriptor::Pointer w = views.at(i);
        MITK_INFO << w->GetId();
    }
}
