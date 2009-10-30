/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtGui module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#include "qxtstringvalidator.h"
#include "qxtstringvalidator_p.h"

#include <QDebug>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QFlags>

QxtStringValidatorPrivate::QxtStringValidatorPrivate() : isUserModel(false)
        , model(0)
        , cs(Qt::CaseSensitive)
        , lookupRole(Qt::EditRole)
        , userFlags(Qt::MatchWrap)
        , lookupStartModelIndex(QModelIndex())
{}

QModelIndex QxtStringValidatorPrivate::lookupPartialMatch(const QString &value) const
{
    //a empty string is always invalid
    if (value.isEmpty())
        return QModelIndex();

    Qt::MatchFlags matchFlags = Qt::MatchStartsWith | userFlags;
    if (cs == Qt::CaseSensitive)
        matchFlags |= Qt::MatchCaseSensitive;

    return lookup(value, matchFlags);
}

QModelIndex QxtStringValidatorPrivate::lookupExactMatch(const QString &value) const
{
    Qt::MatchFlags  matchFlags = Qt::MatchFixedString | userFlags;
    if (cs == Qt::CaseSensitive)
        matchFlags |= Qt::MatchCaseSensitive;

    return lookup(value, matchFlags);
}

QModelIndex QxtStringValidatorPrivate::lookup(const QString &value, const Qt::MatchFlags  &matchFlags) const
{
    QModelIndex startIndex =  lookupStartModelIndex.isValid() ? lookupStartModelIndex : model->index(0, 0);

    QModelIndexList list = model->match(startIndex, lookupRole, value, 1, matchFlags);

    if (list.size() > 0)
        return list[0];
    return QModelIndex();
}


/*!
    \class QxtStringValidator QxtStringValidator
    \ingroup QxtGui
    \brief The QxtStringValidator class provides validation on a QStringList or a QAbstractItemModel

    It provides a String based validation in a stringlist or a custom model.
    QxtStringValidator uses QAbstractItemModel::match() to validate the input.
    For a partial match it returns QValidator::Intermediate and for a full match QValidator::Acceptable.

    Example usage:
    \code

    QLineEdit * testLineEdit = new QLineEdit();

    QStringList testList;
    testList << "StringTestString" << "sTrInGCaSe"<< "StringTest"<< "String"<< "Foobar"<< "BarFoo"<< "QxtLib";

    QxtStringValidator *validator = new QxtStringValidator(ui.lineEdit);
    validator->setStringList(testList);

    //change lookup case sensitivity
    validator->setCaseSensitivity(Qt::CaseInsensitive);

    testLineEdit->setValidator(validator);

    \endcode
 */

/*!
    Constructs a validator object with a parent object that accepts any string in the stringlist.
*/
QxtStringValidator::QxtStringValidator(QObject * parent) : QValidator(parent)
{
    QXT_INIT_PRIVATE(QxtStringValidator);
}

QxtStringValidator::~QxtStringValidator(void)
{}

/*!
    Fixes up the string input if there is no exact match in the stringlist/model.
    The default implementation does nothing
*/
void QxtStringValidator::fixup(QString & input) const
{
    qDebug() << "Fixup called";
    /*we can not choose whats the correct fixup, if a user needs a fixup he has to do it himself
      using the first match in the model is just wrong, because thats what QCompleter should do
    */
    QValidator::fixup(input);
}

/*!
    uses stringlist as new validation list
    if a model was set before it is not deleted
*/
void QxtStringValidator::setStringList(const QStringList &stringList)
{
    //delete model only if it is a model created by us
    if (qxt_d().model && !qxt_d().isUserModel)
    {
        delete qxt_d().model;
        qxt_d().model = 0;
    }

    qxt_d().isUserModel = false;
    qxt_d().lookupStartModelIndex = QModelIndex();
    qxt_d().lookupRole = Qt::EditRole;
    qxt_d().model = new QStringListModel(stringList, this);
}

/*!
    Returns Acceptable if the string input matches a item in the stringlist.
    Returns Intermediate if the string input matches a item in the stringlist partial or if input is empty.
    Returns Invalid otherwise.

    Note: A partial match means the beginning of the strings are matching:
        qxtL matches qxtLib but not testqxtLib
*/
QValidator::State QxtStringValidator::validate(QString & input, int & pos) const
{
    Q_UNUSED(pos);

    // no model or a empty model has only Acceptable values (like no validator was set)
    if (!qxt_d().model)
        return QValidator::Acceptable;

    if (qxt_d().model->rowCount() == 0)
        return QValidator::Acceptable;

    if (input.isEmpty())
        return QValidator::Intermediate;

    if (qxt_d().lookupExactMatch(input).isValid())
    {
        qDebug() << input << " is QValidator::Acceptable";
        return QValidator::Acceptable;
    }

    if (qxt_d().lookupPartialMatch(input).isValid())
    {
        qDebug() << input << " is QValidator::Intermediate";
        return QValidator::Intermediate;
    }

    qDebug() << input << " is QValidator::Invalid";
    return QValidator::Invalid;
}


/*!
    Returns the startModelIndex.
    Note: The return value will only we valid if the user has set the model with setLookupModel().
    \sa setStartModelIndex()
*/
QModelIndex QxtStringValidator::startModelIndex() const
{
    if (qxt_d().isUserModel && qxt_d().model)
    {
        if (qxt_d().lookupStartModelIndex.isValid())
            return qxt_d().lookupStartModelIndex;
        else
            return qxt_d().model->index(0, 0);
    }
    return QModelIndex();
}

/*!
    Returns if recursive lookup is enabled.
    \sa setRecursiveLookup()
*/
bool QxtStringValidator::recursiveLookup() const
{
    if (qxt_d().userFlags & Qt::MatchRecursive)
        return true;
    return false;
}

/*!
    Returns if wrapping lookup is enabled.
    \sa setWrappingLookup()
*/
bool QxtStringValidator::wrappingLookup() const
{
    if (qxt_d().userFlags & Qt::MatchWrap)
        return true;
    return false;
}

/*!
    Returns the used model if it was set by the user.
    \sa setLookupModel()
*/
QAbstractItemModel * QxtStringValidator::lookupModel() const
{
    if (qxt_d().isUserModel)
        return qxt_d().model;
    return 0;
}

/*!
    Returns Qt::CaseSensitive if the QxtStringValidator is matched case sensitively; otherwise returns Qt::CaseInsensitive.
    \sa setCaseSensitivity().
*/
Qt::CaseSensitivity QxtStringValidator::caseSensitivity() const
{
    return qxt_d().cs;
}

/*!
    Sets case sensitive matching to cs.
    If cs is Qt::CaseSensitive, inp matches input but not INPUT.
    The default is Qt::CaseSensitive.
    \sa caseSensitivity().
*/
void QxtStringValidator::setCaseSensitivity(Qt::CaseSensitivity caseSensitivity)
{
    qxt_d().cs = caseSensitivity;
}

/*!
    Sets the index the search should start at
    The default is QModelIndex(0,0).
    Note: this is set to default when the model changes.
    Changing the startModelIndex is only possible if the validator uses a userdefined model
       and the modelindex comes from the used model
    \sa startModelIndex()
*/
void QxtStringValidator::setStartModelIndex(const QModelIndex &index)
{
    if (index.model() == qxt_d().model)
        qxt_d().lookupStartModelIndex = index;
    else
        qWarning() << "ModelIndex from different model. Ignoring.";
}

/*!
    If enabled QxtStringValidator searches the entire hierarchy of the model.
    This is disabled by default.
    \sa recursiveLookup()
*/
void QxtStringValidator::setRecursiveLookup(bool enable)
{
    if (enable)
        qxt_d().userFlags |= Qt::MatchRecursive;
    else
        qxt_d().userFlags &= ~Qt::MatchRecursive;

}

/*!
    If enabled QxtStringValidator performs a search that wraps around,
    so that when the search reaches the last item in the model, it begins again at the first item and continues until all items have been examined.
    This is set by default.
    \sa wrappingLookup()
*/
void QxtStringValidator::setWrappingLookup(bool enable)
{
    if (enable)
        qxt_d().userFlags |= Qt::MatchWrap;
    else
        qxt_d().userFlags &= ~Qt::MatchWrap;
}

/*!
    Uses model as new validation model.
    Note: The validator does not take ownership of the model.
    sa\ lookupModel()
*/
void QxtStringValidator::setLookupModel(QAbstractItemModel *model)
{
    if (!qxt_d().isUserModel && qxt_d().model)
    {
        delete qxt_d().model;
        qxt_d().model = 0;
    }

    qxt_d().lookupRole = Qt::EditRole;
    qxt_d().isUserModel = true;
    qxt_d().lookupStartModelIndex = QModelIndex();
    qxt_d().model = QPointer<QAbstractItemModel>(model);
}

/*!
    Sets the item role to be used to query the contents of items for validation.
    Note: this is only possible if the model was set with setLookupModel()
    This is set to default Value Qt::EditRole when the model changes
    \sa lookupRole()
*/
void QxtStringValidator::setLookupRole(const int role)
{
    if (qxt_d().isUserModel)
        qxt_d().lookupRole = role;
}

/*!
    Returns the item role to be used to query the contents of items for validation.
    \sa setLookupRole()
*/
int QxtStringValidator::lookupRole() const
{
    return qxt_d().lookupRole;
}
