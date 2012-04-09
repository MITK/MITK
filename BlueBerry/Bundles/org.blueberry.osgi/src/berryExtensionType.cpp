/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "berryExtensionType.h"

#include <QVector>
#include <QReadWriteLock>

#include <cstring>

namespace berry {

class CustomTypeInfo
{
public:
  CustomTypeInfo() : typeName(), constr(0), destr(0)
  {}

  QByteArray typeName;
  ExtensionType::Constructor constr;
  ExtensionType::Destructor destr;

  int alias;
};

//Q_DECLARE_TYPEINFO(QCustomTypeInfo, Q_MOVABLE_TYPE);
Q_GLOBAL_STATIC(QVector<CustomTypeInfo>, customTypes)
Q_GLOBAL_STATIC(QReadWriteLock, customTypesLock)

const char* ExtensionType::typeName(int type)
{

  const QVector<CustomTypeInfo>* const ct = customTypes();
  QReadLocker locker(customTypesLock());
  return ct && ct->count() > type && !ct->at(type).typeName.isEmpty()
      ? ct->at(type).typeName.constData()
      : static_cast<const char *>(0);
}


/*! \internal
    Similar to ExtensionType::type(), but doesn't lock the mutex.
*/
static int extensionTypeCustomType_unlocked(const char *typeName, int length)
{
  const QVector<CustomTypeInfo>* const ct = customTypes();
  if (!ct) return 0;

  for (int v = 0; v < ct->count(); ++v)
  {
    const CustomTypeInfo& customInfo = ct->at(v);
    if ((length == customInfo.typeName.size())
        && !std::strcmp(typeName, customInfo.typeName.constData()))
    {
      if (customInfo.alias >= 0)
        return customInfo.alias;
      return v+1;
    }
  }
  return 0;
}

int ExtensionType::registerType(const char *typeName, Destructor destructor,
                            Constructor constructor)
{
  QVector<CustomTypeInfo>* ct = customTypes();
  if (!ct || !typeName || !destructor || !constructor)
    return -1;

  QByteArray normalizedTypeName = QMetaObject::normalizedType(typeName);

  QWriteLocker locker(customTypesLock());
  int idx = extensionTypeCustomType_unlocked(normalizedTypeName.constData(),
                                             normalizedTypeName.size());
  if (!idx)
  {
    CustomTypeInfo inf;
    inf.typeName = normalizedTypeName;
    inf.constr = constructor;
    inf.destr = destructor;
    inf.alias = -1;
    idx = ct->size();
    ct->append(inf);
  }

  return idx;
}

int ExtensionType::registerTypedef(const char* typeName, int aliasId)
{
  QVector<CustomTypeInfo>* ct = customTypes();
  if (!ct || !typeName) return -1;

  QByteArray normalizedTypeName = QMetaObject::normalizedType(typeName);

  QWriteLocker locker(customTypesLock());
  int idx = extensionTypeCustomType_unlocked(normalizedTypeName.constData(),
                                         normalizedTypeName.size());

  if (idx) return idx;

  CustomTypeInfo inf;
  inf.typeName = normalizedTypeName;
  inf.alias = aliasId;
  inf.constr = 0;
  inf.destr = 0;
  ct->append(inf);
  return aliasId;
}

void ExtensionType::unregisterType(const char* typeName)
{
  QVector<CustomTypeInfo> *ct = customTypes();
  if (!ct || !typeName) return;

  QByteArray normalizedTypeName = QMetaObject::normalizedType(typeName);

  QWriteLocker locker(customTypesLock());
  for (int v = 0; v < ct->count(); ++v)
  {
    if (ct->at(v).typeName == typeName)
    {
      CustomTypeInfo &inf = (*ct)[v];
      inf.typeName.clear();
      inf.constr = 0;
      inf.destr = 0;
      inf.alias = -1;
    }
  }
}

bool ExtensionType::isRegistered(int type)
{
  QReadLocker locker(customTypesLock());
  const QVector<CustomTypeInfo>* const ct = customTypes();
  return ((type > 0) && (ct && ct->count() > type - 1) &&
          !ct->at(type - 1).typeName.isEmpty());
}

int ExtensionType::type(const char *typeName)
{
  int length = static_cast<int>(std::strlen(typeName));
  if (!length) return 0;

  QReadLocker locker(customTypesLock());
  int type = extensionTypeCustomType_unlocked(typeName, length);
  if (!type)
  {
    const QByteArray normalizedTypeName = QMetaObject::normalizedType(typeName);
    type = extensionTypeCustomType_unlocked(normalizedTypeName.constData(),
                                        normalizedTypeName.size());
  }

  return type;
}

QObject* ExtensionType::construct(int type)
{
  const QVector<CustomTypeInfo> * const ct = customTypes();

  Constructor constr = 0;

  QReadLocker locker(customTypesLock());
  if (!ct || ct->count() <= type - 1)
    return 0;
  if (ct->at(type - 1).typeName.isEmpty())
    return 0;
  constr = ct->at(type - 1).constr;
  return constr();
}

void ExtensionType::destroy(int type, QObject* data)
{
  if (!data) return;

  const QVector<CustomTypeInfo> * const ct = customTypes();
  if (!ct || ct->count() <= type - 1) return;

  Destructor destr = 0;

  QReadLocker locker(customTypesLock());
  if (ct->at(type - 1).typeName.isEmpty())
    return;
  destr = ct->at(type - 1).destr;
  destr(data);
}

} // end namespace berry
