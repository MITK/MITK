/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkForm.h>
#include <ui_QmitkForm.h>

#include <mitkExceptionMacro.h>
#include <mitkIQuestionWidgetFactory.h>
#include <mitkQuestion.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>

using namespace mitk::Forms;
using Self = QmitkForm;

QmitkForm::QmitkForm(QWidget* parent)
  : QWidget(parent),
    m_Ui(new Ui::QmitkForm),
    m_Form(nullptr),
    m_HasBeenSubmitted(false)
{
  this->setStyleSheet(
    "QFrame[frameShape=\"1\"] { border-radius: 6px; }"
    "QComboBox, QComboBox::drop-down, QLineEdit, QPushButton { border-radius: 4px; }"
  );

  m_Ui->setupUi(this);

  connect(m_Ui->sectionWidget, &QStackedWidget::currentChanged, this, [this](int) { this->Update(); });

  connect(m_Ui->backButton, &QPushButton::clicked, this, &Self::OnBackButtonClicked);
  connect(m_Ui->nextButton, &QPushButton::clicked, this, &Self::OnNextButtonClicked);
  connect(m_Ui->submitButton, &QPushButton::clicked, this, &Self::OnSubmitButtonClicked);
  connect(m_Ui->clearButton, &QPushButton::clicked, this, &Self::OnClearButtonClicked);
  connect(m_Ui->submitAnotherButton, &QPushButton::clicked, this, &Self::OnSubmitAnotherButtonClicked);
}

QmitkForm::QmitkForm(Form* form, QWidget* parent)
  : QmitkForm(parent)
{
  this->SetForm(form);
}

QmitkForm::~QmitkForm()
{
}

mitk::Forms::Form* QmitkForm::GetForm() const
{
  return m_Form;
}

void QmitkForm::SetForm(mitk::Forms::Form* form)
{
  if (form == nullptr)
    mitkThrow() << "Parameter \"form\" must point to a valid form!";

  m_Form = form;

  this->CreateQuestionWidgets();
  this->Reset();
}

fs::path QmitkForm::GetResponsesPath() const
{
  return m_ResponsesPath;
}

void QmitkForm::SetResponsesPath(const fs::path& csvPath)
{
  m_ResponsesPath = csvPath;
}

void QmitkForm::CreateQuestionWidgets()
{
  while (m_Ui->sectionWidget->count() > 0)
  {
    auto sectionWidget = m_Ui->sectionWidget->widget(0);
    m_Ui->sectionWidget->removeWidget(sectionWidget);
    sectionWidget->deleteLater();
  }

  const int numberOfSections = m_Form->GetNumberOfSections();

  for (int sectionIndex = 0; sectionIndex < numberOfSections; ++sectionIndex)
  {
    const auto& section = m_Form->GetSection(sectionIndex);
    auto questions = section.GetQuestions();

    auto sectionWidget = new QWidget;
    m_Ui->sectionWidget->addWidget(sectionWidget);

    auto sectionLayout = new QVBoxLayout(sectionWidget);
    sectionLayout->setContentsMargins(0, 0, 0, 0);
    sectionLayout->setSpacing(8);

    for (auto question : questions)
    {
      auto questionWidget = UI::IQuestionWidgetFactory::GetInstance()->Create(question);
      sectionLayout->addWidget(questionWidget);
    }
  }
}

bool QmitkForm::ValidateCurrentSection()
{
  auto sectionWidget = m_Ui->sectionWidget->currentWidget();
  auto questionWidgets = sectionWidget->findChildren<QmitkQuestionWidget*>();

  bool isComplete = true;

  for (auto questionWidget : questionWidgets)
  {
    auto question = questionWidget->GetQuestion();

    if (question->IsRequired() && !question->IsComplete())
    {
      questionWidget->ShowRequirement();
      isComplete = false;
    }
  }

  return isComplete;
}

void QmitkForm::Reset()
{
  int numberOfSections = m_Ui->sectionWidget->count();

  for (int sectionIndex = 0; sectionIndex < numberOfSections; ++sectionIndex)
  {
    auto sectionWidget = m_Ui->sectionWidget->widget(sectionIndex);
    auto questionWidgets = sectionWidget->findChildren<QmitkQuestionWidget*>();

    for (auto questionWidget : questionWidgets)
      questionWidget->Reset();
  }

  m_HasBeenSubmitted = false;
  m_Ui->sectionWidget->setCurrentIndex(0);

  this->Update();
}

void QmitkForm::Update()
{
  this->UpdateFormHeader();
  this->UpdateSubmittedHeader();
  this->UpdateSectionHeader();
  this->UpdateQuestionWidgets();
  this->UpdateFormButtons();
}

void QmitkForm::UpdateFormHeader()
{
  if (m_HasBeenSubmitted || m_Ui->sectionWidget->currentIndex() == -1)
  {
    m_Ui->formHeaderFrame->hide();
    return;
  }

  bool showTitle = !m_Form->GetTitle().empty();

  m_Ui->formTitleLabel->setVisible(showTitle);

  if (showTitle)
    m_Ui->formTitleLabel->setText(QString("<h2>%1</h2>").arg(QString::fromStdString(m_Form->GetTitle())));

  int sectionIndex = m_Ui->sectionWidget->currentIndex();
  bool showDescription = sectionIndex == 0 && !m_Form->GetDescription().empty();

  m_Ui->formDescriptionLabel->setVisible(showDescription);

  if (showDescription)
    m_Ui->formDescriptionLabel->setText(QString::fromStdString(m_Form->GetDescription()));

  bool hasRequiredQuestion = false;

  for (auto question : m_Form->GetSection(sectionIndex).GetQuestions())
  {
    if (question->IsRequired())
    {
      hasRequiredQuestion = true;
      break;
    }
  }

  m_Ui->requiredLabel->setVisible(hasRequiredQuestion);

  m_Ui->formHeaderFrame->setVisible(showTitle || showDescription || hasRequiredQuestion);
}

void QmitkForm::UpdateSubmittedHeader()
{
  if (m_HasBeenSubmitted && m_Ui->sectionWidget->currentIndex() != -1)
  {
    bool showTitle = !m_Form->GetTitle().empty();

    m_Ui->submittedTitleLabel->setVisible(showTitle);

    if (showTitle)
      m_Ui->submittedTitleLabel->setText(QString("<h2>%1</h2>").arg(QString::fromStdString(m_Form->GetTitle())));

    m_Ui->submittedFrame->show();
  }
  else
  {
    m_Ui->submittedFrame->hide();
  }
}

void QmitkForm::UpdateSectionHeader()
{
  if (m_HasBeenSubmitted || m_Ui->sectionWidget->currentIndex() == -1)
  {
    m_Ui->sectionHeaderFrame->hide();
    return;
  }

  int sectionIndex = m_Ui->sectionWidget->currentIndex();

  if (sectionIndex == 0)
  {
    m_Ui->sectionHeaderFrame->hide();
    return;
  }

  const auto& section = m_Form->GetSection(sectionIndex);
  bool showTitle = !section.GetTitle().empty();

  m_Ui->sectionTitleLabel->setVisible(showTitle);

  if (showTitle)
    m_Ui->sectionTitleLabel->setText(QString::fromStdString(section.GetTitle()));

  bool showDescription = !section.GetDescription().empty();

  m_Ui->sectionDescriptionLabel->setVisible(showDescription);

  if (showDescription)
    m_Ui->sectionDescriptionLabel->setText(QString::fromStdString(section.GetDescription()));

  m_Ui->sectionHeaderFrame->setVisible(showTitle || showDescription);
}

void QmitkForm::UpdateQuestionWidgets()
{
  if (m_HasBeenSubmitted || m_Ui->sectionWidget->currentIndex() == -1)
  {
    m_Ui->sectionWidget->hide();
    return;
  }
  else
  {
    m_Ui->sectionWidget->show();
  }

  int currentSectionIndex = m_Ui->sectionWidget->currentIndex();
  int numberOfSections = m_Ui->sectionWidget->count();

  for (int sectionIndex = 0; sectionIndex < numberOfSections; ++sectionIndex)
  {
    auto sectionWidget = m_Ui->sectionWidget->widget(sectionIndex);
    auto questionWidgets = sectionWidget->findChildren<QmitkQuestionWidget*>();

    for (auto questionWidget : questionWidgets)
      questionWidget->setVisible(sectionIndex == currentSectionIndex);
  }
}

void QmitkForm::UpdateFormButtons()
{
  int sectionIndex = m_Ui->sectionWidget->currentIndex();

  m_Ui->backButton->setVisible(!m_HasBeenSubmitted && sectionIndex > 0);
  m_Ui->nextButton->setVisible(!m_HasBeenSubmitted && sectionIndex < m_Ui->sectionWidget->count() - 1);
  m_Ui->submitButton->setVisible(!m_HasBeenSubmitted && sectionIndex != -1 && sectionIndex == m_Ui->sectionWidget->count() - 1);
  m_Ui->clearButton->setVisible(!m_HasBeenSubmitted && sectionIndex != -1);
}

void QmitkForm::OnBackButtonClicked()
{
  m_Ui->sectionWidget->setCurrentIndex(m_Ui->sectionWidget->currentIndex() - 1);
}

void QmitkForm::OnNextButtonClicked()
{
  if (this->ValidateCurrentSection())
    m_Ui->sectionWidget->setCurrentIndex(m_Ui->sectionWidget->currentIndex() + 1);
}

void QmitkForm::OnSubmitButtonClicked()
{
  if (this->ValidateCurrentSection())
  {
    emit Submit();

    if (m_ResponsesPath.empty())
    {
      m_ResponsesPath = QFileDialog::getSaveFileName(this,
        "Submit Form",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation).append("/form.csv"),
        "Comma-separated values (*.csv)").toStdString();
    }

    if (!m_ResponsesPath.empty())
    {
      bool retry;

      do
      {
        retry = false;

        try
        {
          mitk::Forms::SubmitToCSV(*m_Form, m_ResponsesPath);
        }
        catch (const mitk::Exception& e)
        {
          QMessageBox messageBox;
          messageBox.setWindowTitle("Submit form");
          messageBox.setIcon(QMessageBox::Warning);
          messageBox.setText(e.GetDescription());
          auto retryButton = messageBox.addButton("Retry", QMessageBox::NoRole);
          messageBox.addButton("Ignore", QMessageBox::YesRole);

          messageBox.exec();

          if (messageBox.clickedButton() == retryButton)
            retry = true;
        }
      } while (retry);

      m_HasBeenSubmitted = true;
      this->Update();
    }
  }
}

void QmitkForm::OnClearButtonClicked()
{
  QMessageBox messageBox;
  messageBox.setWindowTitle("Clear form?");
  messageBox.setIcon(QMessageBox::Question);
  messageBox.setText("This will remove your answers from all\nquestions and cannot be undone.");
  messageBox.addButton("Cancel", QMessageBox::NoRole);
  auto clearFormButton = messageBox.addButton("Clear form", QMessageBox::YesRole);

  messageBox.exec();

  if (messageBox.clickedButton() == clearFormButton)
    this->Reset();
}

void QmitkForm::OnSubmitAnotherButtonClicked()
{
  this->Reset();
}
