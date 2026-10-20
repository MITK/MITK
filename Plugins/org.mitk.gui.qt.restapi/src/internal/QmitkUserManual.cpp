/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUserManual.h"

#include <ctkPlugin.h>
#include <ctkPluginContext.h>

#include <QDir>
#include <QHelpEngineCore>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextDocument>

#include <algorithm>
#include <set>

namespace
{
  constexpr std::size_t MaxFullSections = 3;
  constexpr std::size_t MaxListedSections = 15;
  constexpr qsizetype MaxSearchSectionLength = 6000;
  constexpr qsizetype MaxReadSectionLength = 20000;
}

QString QmitkUserManual::Load(ctkPluginContext* context)
{
  if (m_Loaded)
    return QString();

  if (context == nullptr)
    return "The REST API plugin is not active.";

  // Start afresh, so that the help of plugins removed since does not linger.
  QDir directory(context->getDataFile("manual").absoluteFilePath());
  directory.removeRecursively();

  if (!directory.mkpath("."))
    return QString("Cannot create %1.").arg(directory.absolutePath());

  QHelpEngineCore engine(directory.filePath("manual.qhc"));
  engine.setReadOnly(false);

  if (!engine.setupData())
    return QString("Cannot open the help collection: %1").arg(engine.error());

  constexpr auto ReadableStates = ctkPlugin::RESOLVED | ctkPlugin::STARTING | ctkPlugin::ACTIVE | ctkPlugin::STOPPING;

  for (const auto& plugin : context->getPlugins())
  {
    // The system plugin and plugins that are not resolved have no archive to
    // read resources from; asking them crashes, like the help view knows.
    if (plugin->getPluginId() == 0 || (plugin->getState() & ReadableStates) == 0)
      continue;

    for (const auto& resource : plugin->findResources("/", "*.qch", true))
    {
      const auto path = directory.filePath(QString("%1_%2").arg(plugin->getPluginId()).arg(resource.section('/', -1)));
      QFile file(path);

      if (!file.open(QIODevice::WriteOnly))
        continue;

      file.write(plugin->getResource(resource));
      file.close();

      engine.registerDocumentation(path);
    }
  }

  for (const auto& namespaceName : engine.registeredDocumentations())
  {
    for (const auto& url : engine.files(namespaceName, QString(), "html"))
      this->AddPage(QString::fromUtf8(engine.fileData(url)));
  }

  m_Loaded = true;
  return QString();
}

bool QmitkUserManual::IsLoaded() const
{
  return m_Loaded;
}

void QmitkUserManual::AddPage(const QString& html)
{
  QTextDocument document;
  document.setHtml(html);

  const auto page = document.metaInformation(QTextDocument::DocumentTitle).trimmed();
  Entry entry = { page, page, QString() };

  auto addEntry = [this, &entry]() {
    if (!entry.Text.trimmed().isEmpty())
      m_Entries.push_back(entry);
  };

  for (auto block = document.begin(); block.isValid(); block = block.next())
  {
    auto text = block.text();
    text.remove(QChar::ObjectReplacementCharacter);
    text = text.trimmed();

    if (block.blockFormat().headingLevel() > 0)
    {
      addEntry();
      entry = { page, text, QString() };
      continue;
    }

    if (!text.isEmpty())
      entry.Text += text + '\n';
  }

  addEntry();
}

QmitkUserManual::Section QmitkUserManual::MakeSection(std::size_t id, qsizetype maxLength) const
{
  const auto& entry = m_Entries[id];

  Section section;
  section.Id = id;
  section.Page = entry.Page;
  section.Title = entry.Title;

  if (maxLength > 0)
  {
    section.IsCut = entry.Text.size() > maxLength;
    section.Text = entry.Text.left(maxLength);
  }

  return section;
}

QmitkUserManual::SearchResult QmitkUserManual::Search(const QString& query) const
{
  static const QRegularExpression separators("[^\\w]+");

  std::set<QString> words;

  for (const auto& word : query.toLower().split(separators, Qt::SkipEmptyParts))
  {
    if (word.size() >= 3)
      words.insert(word);
  }

  std::vector<std::pair<int, std::size_t>> scores;

  for (std::size_t i = 0; i < m_Entries.size() && !words.empty(); ++i)
  {
    const auto title = (m_Entries[i].Page + ' ' + m_Entries[i].Title).toLower();
    const auto text = m_Entries[i].Text.toLower();
    int score = 0;

    for (const auto& word : words)
    {
      const auto inTitle = title.count(word);
      const auto inText = std::min<qsizetype>(text.count(word), 10);

      // Sections that contain more of the words come first.
      if (inTitle + inText > 0)
        score += 20;

      score += static_cast<int>(5 * inTitle + inText);
    }

    if (score > 0)
      scores.emplace_back(score, i);
  }

  std::stable_sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) { return a.first > b.first; });

  SearchResult result;
  result.MatchCount = scores.size();

  for (std::size_t i = 0; i < std::min(MaxFullSections + MaxListedSections, scores.size()); ++i)
  {
    if (i < MaxFullSections)
    {
      result.Sections.push_back(this->MakeSection(scores[i].second, MaxSearchSectionLength));
    }
    else
    {
      result.OtherSections.push_back(this->MakeSection(scores[i].second, 0));
    }
  }

  return result;
}

std::optional<QmitkUserManual::Section> QmitkUserManual::GetSection(std::size_t id) const
{
  if (id >= m_Entries.size())
    return std::nullopt;

  return this->MakeSection(id, MaxReadSectionLength);
}
