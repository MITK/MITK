/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkScreenshotQuestionWidget.h>
#include "QmitkScreenshotWidget.h"

#include <mitkBaseRenderer.h>
#include <mitkExceptionMacro.h>

#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkRenderLargeImage.h>

#include <QDir>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QTemporaryFile>
#include <QVBoxLayout>

using namespace mitk::Forms;
using Self = QmitkScreenshotQuestionWidget;

namespace
{
  mitk::BaseRenderer* GetRenderer(const QString& action)
  {
    std::string rendererName = "stdmulti.widget";

    if (action == "Axial")
    {
      rendererName += "0";
    }
    else if (action == "Sagittal")
    {
      rendererName += "1";
    }
    else if (action == "Coronal")
    {
      rendererName += "2";
    }
    else
    {
      return nullptr;
    }

    return mitk::BaseRenderer::GetByName(rendererName);
  }

  void TakeScreenshot(vtkRenderer* renderer, const fs::path& fileName)
  {
    vtkNew<vtkRenderLargeImage> renderLargeImage;
    renderLargeImage->SetInput(renderer);
    renderLargeImage->SetMagnification(1);

    vtkNew<vtkPNGWriter> pngWriter;
    pngWriter->SetFileName(fileName.string().c_str());
    pngWriter->SetInputConnection(renderLargeImage->GetOutputPort());
    pngWriter->Write();
  }
}

QmitkScreenshotQuestionWidget::QmitkScreenshotQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QVBoxLayout),
    m_TakeScreenshotButton(new QPushButton("Take Screenshot")),
    m_PopupMenu(new QMenu(this))
{
  m_Layout->addWidget(m_TakeScreenshotButton);

  m_PopupMenu->addAction("Axial");
  m_PopupMenu->addAction("Sagittal");
  m_PopupMenu->addAction("Coronal");

  this->InsertLayout(m_Layout);

  connect(m_TakeScreenshotButton, &QPushButton::clicked, this, &Self::OnTakeScreenshotButtonClicked);
  connect(m_PopupMenu, &QMenu::triggered, [this](QAction* action) { this->OnPopupMenuTriggered(action->text()); });
}

QmitkScreenshotQuestionWidget::~QmitkScreenshotQuestionWidget()
{
}

QmitkQuestionWidget* QmitkScreenshotQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkScreenshotQuestionWidget(parent);
}

Question* QmitkScreenshotQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkScreenshotQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto screenshotQuestion = dynamic_cast<ScreenshotQuestion*>(question);

  if (screenshotQuestion == nullptr)
    mitkThrow() << "QmitkScreenshotQuestionWidget only accepts ScreenshotQuestion as question type!";

  m_Question = screenshotQuestion;
}

void QmitkScreenshotQuestionWidget::Reset()
{
  if (m_Question)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkScreenshotQuestionWidget::OnTakeScreenshotButtonClicked()
{
  m_PopupMenu->popup(QCursor::pos());
}

void QmitkScreenshotQuestionWidget::OnPopupMenuTriggered(const QString& action)
{
  QTemporaryFile file(QString("%1/screenshot_XXXXXX.png").arg(QDir::tempPath()));
  file.setAutoRemove(false);

  if (!file.open())
    return;

  fs::path fileName = file.fileName().toStdString();

  auto renderer = GetRenderer(action);

  if (renderer == nullptr)
    return;

  auto renderWindow = renderer->GetRenderWindow();

  bool doubleBuffer = renderWindow->GetDoubleBuffer();
  renderWindow->DoubleBufferOff();

  TakeScreenshot(renderer->GetVtkRenderer(), fileName);

  renderWindow->SetDoubleBuffer(doubleBuffer);

  m_Question->AddScreenshot(fileName);
  this->AddScreenshotWidget(file.fileName());
}

void QmitkScreenshotQuestionWidget::AddScreenshotWidget(const QString& screenshotFileName)
{
  auto screenshotWidget = new QmitkScreenshotWidget(screenshotFileName);
  connect(screenshotWidget, &QmitkScreenshotWidget::Remove, this, &Self::OnRemoveScreenshot);
  m_Layout->insertWidget(m_Layout->indexOf(m_TakeScreenshotButton), screenshotWidget);
}

void QmitkScreenshotQuestionWidget::OnRemoveScreenshot(QmitkScreenshotWidget* sender)
{
  m_Layout->removeWidget(sender);
  sender->deleteLater();

  m_Question->RemoveScreenshot(sender->GetScreenshotPath().toStdString());
}
