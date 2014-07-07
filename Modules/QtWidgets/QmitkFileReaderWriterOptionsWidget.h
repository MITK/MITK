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

#ifndef QMITKFILEREADERWRITEROPTIONSWIDGET_H
#define QMITKFILEREADERWRITEROPTIONSWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>

#include <usAny.h>

struct QmitkAnyAdapter
{
  QmitkAnyAdapter(const std::string& name)
    : m_Name(name)
  {}

  virtual ~QmitkAnyAdapter() {}
  virtual us::Any GetAny() const = 0;
  std::string GetName() const
  {
    return m_Name;
  }

private:

  std::string m_Name;
};

class QmitkAnyStringWidget : public QLineEdit, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyStringWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyVectorWidget : public QComboBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyVectorWidget(const std::string& name, const us::Any& any, const QString& defaultValue, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyBoolWidget : public QCheckBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyBoolWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyShortWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyShortWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyUShortWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyUShortWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyIntWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyIntWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyUIntWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyUIntWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyFloatWidget : public QDoubleSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyFloatWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkAnyDoubleWidget : public QDoubleSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkAnyDoubleWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};

class QmitkInvalidAnyWidget : public QLabel, public QmitkAnyAdapter
{
  Q_OBJECT

public:

  QmitkInvalidAnyWidget(const std::string& name, const us::Any& any, QWidget* parent = 0);
  us::Any GetAny() const;
};


class QmitkFileReaderWriterOptionsWidget : public QWidget
{
  Q_OBJECT

public:

  typedef std::map<std::string, us::Any> Options;

  QmitkFileReaderWriterOptionsWidget(const Options& options, QWidget* parent = NULL);

  Options GetOptions() const;

private:

  Q_DISABLE_COPY(QmitkFileReaderWriterOptionsWidget)

};

#endif // QMITKFILEREADERWRITEROPTIONSWIDGET_H
