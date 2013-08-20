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

#include "QmitkLabelSetWidget.h"

#include <berryPlatform.h>

#include <mitkIDataStorageService.h>
#include <mitkLabelSetImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkToolManagerProvider.h>
#include <mitkLabelSetImageToSurfaceThreadedFilter.h>

#include <QmitkDataStorageComboBox.h>
#include <QmitkNewSegmentationDialog.h>

#include <algorithm>
#include <cassert>
#include <iterator>

#include <QCompleter>
#include <QStringListModel>


QmitkLabelSetWidget::QmitkLabelSetWidget(QWidget* parent)
  : QWidget(parent)
{
  m_Controls.setupUi(this);

  m_Controls.m_LabelSetTableWidget->Init();

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(newLabel()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(renameLabel(int)), this, SLOT(OnRenameLabel(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createSurface(int)), this, SLOT(OnCreateSurface(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SIGNAL(goToLabel(const mitk::Point3D&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateSurface( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createMask(int)), this, SLOT(OnCreateMask(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateMask( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_btNewLabelSet, SIGNAL(clicked()), this, SLOT(OnNewLabelSet()) );
  connect( m_Controls.m_btSaveLabelSet, SIGNAL(clicked()), this, SLOT(OnSaveLabelSet()) );
  connect( m_Controls.m_btLoadLabelSet, SIGNAL(clicked()), this, SLOT(OnLoadLabelSet()) );
  connect( m_Controls.m_btImportLabelSet, SIGNAL(clicked()), this, SLOT(OnImportLabelSet()) );

  m_Controls.m_LabelSearchBox->setAlwaysShowClearIcon(true);
  m_Controls.m_LabelSearchBox->setShowSearchIcon(true);

  QStringList completionList;
  completionList << "";
  m_Completer = new QCompleter(completionList,this);
  m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.m_LabelSearchBox->setCompleter(m_Completer);

  connect( m_Controls.m_LabelSearchBox, SIGNAL(returnPressed()), this, SLOT(OnSearchLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(labelListModified(const QStringList&)), this, SLOT( OnLabelListModified(const QStringList&)) );
//  connect( m_Controls.m_btNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );

  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(m_Controls.m_LabelSetTableWidget->GetLabelList());
}

QmitkLabelSetWidget::~QmitkLabelSetWidget()
{
}

void QmitkLabelSetWidget::SetActiveLabelSetImage(mitk::LabelSetImage* _image)
{
  m_Controls.m_LabelSetTableWidget->SetActiveLabelSetImage(_image);
}

void QmitkLabelSetWidget::SetPreferences( berry::IPreferences::Pointer prefs)
{
  m_Preferences = prefs;
}

void QmitkLabelSetWidget::OnSearchLabel()
{
  m_Controls.m_LabelSetTableWidget->SetActiveLabel(m_Controls.m_LabelSearchBox->text().toStdString());
}

void QmitkLabelSetWidget::OnLabelListModified(const QStringList& list)
{
   QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
   completeModel->setStringList(list);
}

void QmitkLabelSetWidget::OnRenameLabel(int index)
{
    // ask about the name and organ type of the new segmentation
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );

    QString storedList = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
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
        QString storedString = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
        MITK_DEBUG << "storedString: " << storedString.toStdString();
        // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
        QRegExp onePart("(?:[^;]|\\\\;)*");
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
    }

    dialog->SetSuggestionList( organColors );

    int dialogReturnValue = dialog->exec();

    if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

    mitk::DataNode* workingNode = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0);
    if (!workingNode) return;

    mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
    if ( !lsImage ) return;

    lsImage->RenameLabel(index, dialog->GetSegmentationName().toStdString(), dialog->GetColor());
}

void QmitkLabelSetWidget::OnNewLabel()
{
    // ask about the name and organ type of the new segmentation
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );

    QString storedList = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
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
        QString storedString = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
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

    mitk::Color color = dialog->GetColor();

    mitk::DataNode* workingNode = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0);
    if (!workingNode) return;

    mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
    if ( !lsImage ) return;

    lsImage->AddLabel(dialog->GetSegmentationName().toStdString(), color);
}

void QmitkLabelSetWidget::OnCreateSurface(int index)
{
    mitk::DataNode* segNode = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0);
    if (!segNode) return;

    mitk::LabelSetImage* segImage = dynamic_cast<mitk::LabelSetImage*>( segNode->GetData() );
    if (!segImage) return;

    mitk::LabelSetImageToSurfaceThreadedFilter::Pointer filter =
       mitk::LabelSetImageToSurfaceThreadedFilter::New();

    filter->SetPointerParameter("Input", segImage);
    filter->SetParameter("RequestedLabel", index);
//    filter->SetDataStorage( *this->GetDataStorage() );

    filter->StartAlgorithm();
}
