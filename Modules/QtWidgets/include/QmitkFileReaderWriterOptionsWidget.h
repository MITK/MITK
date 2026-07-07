/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFileReaderWriterOptionsWidget_h
#define QmitkFileReaderWriterOptionsWidget_h

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>

#include <usAny.h>

/**
 * \brief Abstract adapter interface for converting Qt widget values to us::Any.
 *
 * Each adapter associates a named option with a Qt widget and provides
 * a GetAny() method that returns the widget's current value as a us::Any.
 *
 * \sa QmitkFileReaderWriterOptionsWidget
 */
struct QmitkAnyAdapter
{
  /**
   * \brief Constructs the adapter with the given option name.
   * \param[in] name The option name.
   */
  QmitkAnyAdapter(const std::string &name) : m_Name(name) {}
  virtual ~QmitkAnyAdapter() {}

  /**
   * \brief Returns the current widget value as a us::Any.
   * \return The value wrapped in us::Any.
   */
  virtual us::Any GetAny() const = 0;

  /**
   * \brief Returns the option name associated with this adapter.
   * \return The option name.
   */
  std::string GetName() const { return m_Name; }
private:
  std::string m_Name;
};

/**
 * \brief A QLineEdit-based adapter for string-valued options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyStringWidget : public QLineEdit, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a string option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to std::string).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyStringWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current text as a us::Any. */
  us::Any GetAny() const override;
};

/**
 * \brief A QComboBox-based adapter for vector-of-string options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyVectorWidget : public QComboBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a vector option widget with a combo box.
   * \param[in] name         The option name.
   * \param[in] any          The initial vector value.
   * \param[in] defaultValue The default item text to select.
   * \param[in] parent       Optional parent widget.
   */
  QmitkAnyVectorWidget(const std::string &name,
                       const us::Any &any,
                       const QString &defaultValue,
                       QWidget *parent = nullptr);

  /** \brief Returns the currently selected item as a us::Any. */
  us::Any GetAny() const override;
};

/**
 * \brief A QCheckBox-based adapter for boolean options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyBoolWidget : public QCheckBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a boolean option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to bool).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyBoolWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current check state as a us::Any. */
  us::Any GetAny() const override;
};

/**
 * \brief A QSpinBox-based adapter for short integer options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyShortWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a short integer option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to short).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyShortWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current value as a us::Any wrapping a short. */
  us::Any GetAny() const override;
};

/**
 * \brief A QSpinBox-based adapter for unsigned short integer options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyUShortWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs an unsigned short integer option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to unsigned short).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyUShortWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current value as a us::Any wrapping an unsigned short. */
  us::Any GetAny() const override;
};

/**
 * \brief A QSpinBox-based adapter for signed integer options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyIntWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a signed integer option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to int).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyIntWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current value as a us::Any wrapping an int. */
  us::Any GetAny() const override;
};

/**
 * \brief A QSpinBox-based adapter for unsigned integer options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyUIntWidget : public QSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs an unsigned integer option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to unsigned int).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyUIntWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current value as a us::Any wrapping an unsigned int. */
  us::Any GetAny() const override;
};

/**
 * \brief A QDoubleSpinBox-based adapter for float options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyFloatWidget : public QDoubleSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a float option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to float).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyFloatWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current value as a us::Any wrapping a float. */
  us::Any GetAny() const override;
};

/**
 * \brief A QDoubleSpinBox-based adapter for double-precision options.
 * \sa QmitkAnyAdapter
 */
class QmitkAnyDoubleWidget : public QDoubleSpinBox, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a double-precision option widget.
   * \param[in] name   The option name.
   * \param[in] any    The initial value (must be convertible to double).
   * \param[in] parent Optional parent widget.
   */
  QmitkAnyDoubleWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the current value as a us::Any wrapping a double. */
  us::Any GetAny() const override;
};

/**
 * \brief A QLabel-based adapter for options of unsupported types.
 *
 * Displays the string representation of the us::Any value as a read-only label.
 * GetAny() returns the original value unchanged.
 *
 * \sa QmitkAnyAdapter
 */
class QmitkInvalidAnyWidget : public QLabel, public QmitkAnyAdapter
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a read-only label for an unsupported option type.
   * \param[in] name   The option name.
   * \param[in] any    The option value.
   * \param[in] parent Optional parent widget.
   */
  QmitkInvalidAnyWidget(const std::string &name, const us::Any &any, QWidget *parent = nullptr);

  /** \brief Returns the original us::Any value unchanged. */
  us::Any GetAny() const override;
};

/**
 * \brief A widget that dynamically creates typed editor widgets for file reader/writer options.
 *
 * Given a map of option name to us::Any value, this widget creates an appropriate
 * editor widget for each option (text field, checkbox, spin box, combo box, etc.)
 * and lays them out vertically. The current values can be retrieved as a map via
 * GetOptions().
 *
 * \sa QmitkAnyAdapter
 * \sa QmitkFileReaderOptionsDialog
 * \sa QmitkFileWriterOptionsDialog
 */
class QmitkFileReaderWriterOptionsWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief Type alias for the options map (option name to us::Any). */
  typedef std::map<std::string, us::Any> Options;

  /**
   * \brief Constructs the widget, creating editor sub-widgets for each option.
   * \param[in] options The options to present.
   * \param[in] parent  Optional parent widget.
   */
  QmitkFileReaderWriterOptionsWidget(const Options &options, QWidget *parent = nullptr);

  /**
   * \brief Returns the current option values from all editor widgets.
   * \return A map of option name to us::Any containing the current values.
   */
  Options GetOptions() const;

private:
  Q_DISABLE_COPY(QmitkFileReaderWriterOptionsWidget)
};

#endif
