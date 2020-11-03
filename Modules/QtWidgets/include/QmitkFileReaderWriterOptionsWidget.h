/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKFILEREADERWRITEROPTIONSWIDGET_H
#define QMITKFILEREADERWRITEROPTIONSWIDGET_H

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

#include <usAny.h>

struct QmitkAnyAdapter
{
  QmitkAnyAdapter(const std::string &name) : m_Name(name) {}
  virtual ~QmitkAnyAdapter() {}
  virtual us::Any GetAny() const = 0;
  std::string GetName() const { return m_Name; }
private:
  std::string m_Name;
};

class QmitkAnyStringWidget : public QLineEdit, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyStringWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyVectorWidget : public QComboBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyVectorWidget(const std::string &name,
                       const us::Any &any,
                       const QString &defaultValue,
                       QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyBoolWidget : public QCheckBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyBoolWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyShortWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyShortWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyUShortWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyUShortWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyIntWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyIntWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyUIntWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyUIntWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyFloatWidget : public QDoubleSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyFloatWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkAnyDoubleWidget : public QDoubleSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkAnyDoubleWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkInvalidAnyWidget : public QLabel, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  QmitkInvalidAnyWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);
  us::Any GetAny() const override;
};

class QmitkFileReaderWriterOptionsWidget : public QWidget
{
  Q_OBJECT

public:
  typedef std::map<std::string, us::Any> Options;

  QmitkFileReaderWriterOptionsWidget(const Options &options, QWidget *parent = nullptr);

  Options GetOptions() const;

private:
  Q_DISABLE_COPY(QmitkFileReaderWriterOptionsWidget)
};

#endif // QMITKFILEREADERWRITEROPTIONSWIDGET_H
