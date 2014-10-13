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

/**
  \file mitkdumpdir.cpp

  \brief Commandline application that calls the MitkDump application on multiple directories.

  Usage:
  \verbatim
    MitkDumpDir [-v] dir1 [... dirN]

    -v      output more details on commandline
    dirN    directory names
  \endverbatim

  The application will traverse each directory for sub-directories.
  For each leaf-directory (i.e. directories without sub-directories),
  the mitkdump application will be called, given all files from the
  leaf directory.

  If the "-v" flag is used, it will be passed to the mitkdump application,
  which can create considerably more output.
*/

#include <QtCore>

bool printEachOutput(false);

bool processLeafNode(const QString& dirname, const QString& executablename)
{
  qDebug() << "Processing " << qPrintable(dirname);

  QFileInfoList fileinfos = QDir( dirname ).entryInfoList( QDir::Files | QDir::Readable );

  QStringList filenames;
  foreach(QFileInfo info, fileinfos)
  {
    filenames << info.absoluteFilePath();
  }

  QProcess mitkdump;
  mitkdump.start(executablename, filenames);

  if (!mitkdump.waitForStarted())
    return false;

  if (!mitkdump.waitForFinished())
    return false;

  if (printEachOutput)
  {
    QString output(mitkdump.readAll());
    qDebug() << output;
  }

  return true;
}

void processLeafNodes(const QString& rootDir, const QString& executablename)
{
  QDirIterator dirIter(rootDir,
                       QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable,
                       QDirIterator::Subdirectories);

  while (dirIter.hasNext())
  {
    dirIter.next();

    if ( QDir( dirIter.fileInfo().absoluteFilePath() ).entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable ).empty() )
    {
      processLeafNodes( dirIter.filePath(), executablename );
    }
  }

  if ( QDir( rootDir ).entryList( QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable ).empty() )
  {
    processLeafNode( rootDir, executablename );
  }
}

int main(int argc, char* argv[])
{
  QCoreApplication a(argc,argv);

  int firstDirIndex = 1;

  // see if we got the '-v' flag to output file details
  if (argc > 1 && std::string(argv[firstDirIndex]) == "-v")
  {
    printEachOutput = true;
    ++firstDirIndex;
  }

  QString executablename( QCoreApplication::applicationDirPath() + QDir::separator() + "MitkDump" );
#ifdef WIN32
  executablename += ".exe";
#endif

  // analyze dirs from argv
  QStringList inputDirs;
  for (int a = firstDirIndex; a < argc; ++a)
  {
    inputDirs << argv[a];
  }

  foreach(QString dirname, inputDirs)
  {
    processLeafNodes(dirname, executablename);
  }

  // if we got so far, everything is fine
  return EXIT_SUCCESS;
}
