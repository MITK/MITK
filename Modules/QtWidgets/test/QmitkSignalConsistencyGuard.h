/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSignalConsistencyGuard_h
#define QmitkSignalConsistencyGuard_h

#include <QAbstractItemModel>
#include <QStack>
#include <QString>

#include <string>
#include <vector>

/**
 * Live validator for a model's structural change signals.
 *
 * QAbstractItemModel promises its views that every rowsInserted /
 * rowsRemoved names the parent under which the change happened and a range
 * that matches the resulting change in that parent's rowCount(). A
 * QTreeView and a QSortFilterProxyModel keep their internal bookkeeping in
 * sync by trusting exactly this. When a model announces the change under
 * the wrong parent, that bookkeeping is corrupted and later index lookups
 * dereference stale items - the mechanism behind the intermittent Data
 * Manager crashes on drag-and-drop.
 *
 * The guard connects to the model and records, per emitted change, whether
 * the announced parent's rowCount() actually moved by the announced amount;
 * for moves, the counts of the source and the destination parent.
 * It also rejects a structurally invalid announced range (negative first row
 * or last < first), which no valid model may emit.
 */
class SignalConsistencyGuard
{
public:
  explicit SignalConsistencyGuard(QAbstractItemModel *model)
    : m_Model(model)
  {
    m_Connections.push_back(QObject::connect(
      model, &QAbstractItemModel::rowsAboutToBeInserted,
      [this](const QModelIndex &parent, int first, int last)
      {
        const int before = m_Model->rowCount(parent);
        if (first < 0 || last < first)
          this->Fail(QStringLiteral("rowsAboutToBeInserted announced an invalid range [%1,%2] under parent(row=%3)")
                       .arg(first).arg(last).arg(parent.row()));
        else if (first > before)
          this->Fail(QStringLiteral("rowsAboutToBeInserted announced first row %1 under parent(row=%2), "
                                    "but that parent only has %3 row(s)")
                       .arg(first).arg(parent.row()).arg(before));
        m_InsertBefore.push(before);
      }));

    m_Connections.push_back(QObject::connect(
      model, &QAbstractItemModel::rowsInserted,
      [this](const QModelIndex &parent, int first, int last)
      {
        const int before = m_InsertBefore.isEmpty() ? -1 : m_InsertBefore.pop();
        const int actual = m_Model->rowCount(parent);
        if (before < 0 || actual != before + (last - first + 1))
        {
          this->Fail(QStringLiteral("rowsInserted announced %1 new row(s) under parent(row=%2), "
                                    "but that parent's rowCount went from %3 to %4")
                       .arg(last - first + 1).arg(parent.row()).arg(before).arg(actual));
        }
      }));

    m_Connections.push_back(QObject::connect(
      model, &QAbstractItemModel::rowsAboutToBeRemoved,
      [this](const QModelIndex &parent, int first, int last)
      {
        if (first < 0 || last < first)
          this->Fail(QStringLiteral("rowsAboutToBeRemoved announced an invalid range [%1,%2] under parent(row=%3)")
                       .arg(first).arg(last).arg(parent.row()));
        m_RemoveBefore.push(m_Model->rowCount(parent));
      }));

    m_Connections.push_back(QObject::connect(
      model, &QAbstractItemModel::rowsRemoved,
      [this](const QModelIndex &parent, int first, int last)
      {
        const int before = m_RemoveBefore.isEmpty() ? -1 : m_RemoveBefore.pop();
        const int actual = m_Model->rowCount(parent);
        if (before < 0 || actual != before - (last - first + 1))
        {
          this->Fail(QStringLiteral("rowsRemoved announced %1 removed row(s) under parent(row=%2), "
                                    "but that parent's rowCount went from %3 to %4")
                       .arg(last - first + 1).arg(parent.row()).arg(before).arg(actual));
        }
      }));

    m_Connections.push_back(QObject::connect(
      model, &QAbstractItemModel::rowsAboutToBeMoved,
      [this](const QModelIndex &sourceParent, int first, int last, const QModelIndex &destinationParent, int)
      {
        if (first < 0 || last < first)
          this->Fail(QStringLiteral("rowsAboutToBeMoved announced an invalid range [%1,%2]").arg(first).arg(last));
        m_MoveBefore.push({m_Model->rowCount(sourceParent), m_Model->rowCount(destinationParent),
                           sourceParent == destinationParent});
      }));

    m_Connections.push_back(QObject::connect(
      model, &QAbstractItemModel::rowsMoved,
      [this](const QModelIndex &sourceParent, int first, int last, const QModelIndex &destinationParent, int)
      {
        if (m_MoveBefore.isEmpty())
        {
          this->Fail(QStringLiteral("rowsMoved without rowsAboutToBeMoved"));
          return;
        }

        const MoveCounts before = m_MoveBefore.pop();
        const int moved = last - first + 1;
        const int sourceAfter = m_Model->rowCount(sourceParent);
        const int destinationAfter = m_Model->rowCount(destinationParent);
        const bool consistent = before.SameParent
          ? sourceAfter == before.Source
          : sourceAfter == before.Source - moved && destinationAfter == before.Destination + moved;
        if (!consistent)
        {
          this->Fail(QStringLiteral("rowsMoved announced %1 moved row(s), but the source went from %2 to %3 "
                                    "rows and the destination from %4 to %5")
                       .arg(moved).arg(before.Source).arg(sourceAfter).arg(before.Destination).arg(destinationAfter));
        }
      }));
  }

  ~SignalConsistencyGuard()
  {
    for (const auto &connection : m_Connections)
      QObject::disconnect(connection);
  }

  SignalConsistencyGuard(const SignalConsistencyGuard &) = delete;
  SignalConsistencyGuard &operator=(const SignalConsistencyGuard &) = delete;

  bool IsConsistent() const { return m_Consistent; }
  std::string FailureText() const { return m_Failure.toStdString(); }

private:
  void Fail(const QString &message)
  {
    if (m_Consistent)
    {
      m_Consistent = false;
      m_Failure = message;
    }
  }

  struct MoveCounts
  {
    int Source;
    int Destination;
    bool SameParent;
  };

  QAbstractItemModel *m_Model;
  std::vector<QMetaObject::Connection> m_Connections;
  QStack<int> m_InsertBefore;
  QStack<int> m_RemoveBefore;
  QStack<MoveCounts> m_MoveBefore;
  bool m_Consistent = true;
  QString m_Failure;
};

#endif
