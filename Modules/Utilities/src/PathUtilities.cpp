#include "PathUtilities.h"

#include <algorithm>

#include <QDir>
#include <QStringList>
#include <QCoreApplication>

namespace Utilities
{
  std::string preferredPath(const std::string& path)
  {
    std::string temp = path;
    
#ifdef _WIN32
    std::replace(temp.begin(), temp.end(), '/', '\\');
#else
    std::replace(temp.begin(), temp.end(), '\\', '/');
#endif
    
    return temp;
  }
  
  std::string convertToLocalEncoding(QString str)
  {
#ifdef _WIN32
    return str.toLocal8Bit().constData();
#else
    return str.toUtf8().constData();
#endif
  }
  
  std::string absPath(const std::string& relpath)
  {
    // TODO: wstring support?
    
    QString appDirPath = QCoreApplication::applicationDirPath();
    
    // QApplication::applicationDirPath returns a path in incorrect case
    QString dpath = appDirPath.toLower();
    
    static const QStringList CMAKE_BUILD_TYPES = {
      "release",
      "debug",
      "relwithdebinfo",
      "minsizerel"
    };
    
    for (const auto& btype : CMAKE_BUILD_TYPES) {
      // Check the latest directory name from application dir path
      if (QDir(dpath).dirName() == btype) {
        appDirPath = QDir(appDirPath).filePath("..");
        break;
      }
    }
    
    QString absPath = QDir(appDirPath).filePath(QString::fromStdString(relpath));
    return convertToLocalEncoding(absPath);
  }
}
