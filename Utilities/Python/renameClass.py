#!/usr/bin/python

# mitk c++ class rename script by Marco Nolden and Michael Mueller
#
# There are two ways to use this:
#
# 1. renameClass <dir> <oldClassName> <newClassName>
#
# 2. renameClass <dir> <csvFileOfClassNameReplaces>
#
#
# Always backup your code before using this! It has only been tested on a few cases for a special purpose!
# It does not parse the c++ , but just does a text replace on ClassName, mitkClassName, m_ClassName, GetClassName
# and SetClassName and renames files accordingly. There is some basic mechanism to avoid name clashes but better
# double check the results.
#
# using the commitCommandString and the renameFileCommand you can commit your rename results directly to your
# favourite version control.
#
# some source code was taken from regexplace by Stefano Spinucci
#


import os;
import fileinput;
import re;
import sys;
import stat;
import os.path;
import csv;

commitCommandString = None
renameFileCommand = None
#
# uncomment and adapt this to commit after each rename. Parameters will be oldname, newname
# commitCommandString = "git commit -a -m \"CHG (#3669): renamed %s to %s\""

# uncomment and adapt this for  renaming files. If undefined, a normal file rename will we performed
# using python commands
# renameFileCommand = "git mv %s %s "

class FileList:
  def __init__(self,dir):
    self.filelist = [];
    for root,dirs,files in os.walk(dir):
      if ".svn" in dirs:
          dirs.remove(".svn")
      if ".git" in dirs:
          dirs.remove(".git")

      for name in files:
          self.filelist.append((root,name))

  def contains(self,filename):
    for root,xfilename in self.filelist:
      if (xfilename == filename):
        return (root,filename)

    return None

  def rename_file(self,source,dest):
    self.filelist.remove(source)
    xroot,xfile = source
    self.filelist.append((xroot,dest))
    if renameFileCommand:
      os.system(renameFileCommand % (os.path.join(xroot,xfile),os.path.join(xroot,dest) ) )
    else:
      os.rename(os.path.join(xroot,xfile),os.path.join(xroot,dest))

  def exists_somewhere(self,stringlist):
    exists = False
    regexString = str(stringlist.pop())
    for string in stringlist:
      regexString = regexString  + "|" + string

    regexString = "\\b(" + regexString + ")\\b"
    regex = re.compile(regexString)

    for root,filename in self.filelist:
        xfile = os.path.join(root,filename)

        # open file for read
        readlines=open(xfile,'r').readlines()

        # search and replace in current file printing to the user changed lines
        for currentline in readlines:
            if regex.search(currentline):
              print "warning: %s found in %s"  % (string,xfile)
              exists = True
    return exists



def find_all(dir):
    filelist = [];
    for root,dirs,files in os.walk(dir):
      if ".svn" in dirs:
          dirs.remove(".svn")
      if ".git" in dirs:
          dirs.remove(".git")

      for name in files:
          filelist.append((root,name))

    return filelist

# in all files in 'fileslist' search the regexp 'searchregx' and replace
# with 'replacestring'; real substitution in files only if 'simulation' = 0;
# real substitution may also be step by step (if 'stepbystep' = 1)
def replace_in_files(fileslist, searchregx, replacestring, simulation, stepbystep):
    # compile regexp
    cregex=re.compile(searchregx)

    # print message to the user
    if simulation == 1:
        print '\nReplaced (simulation):\n'
    else:
        print '\nReplaced:\n'

    # loop on all files
    for root,filename in fileslist:
        xfile = os.path.join(root,filename)

        # initialize the replace flag
        replaceflag=0
        fileAtt = os.stat(xfile)[0]
        if (not fileAtt & stat.S_IWRITE):
            continue

        # open file for read
        readlines=open(xfile,'r').readlines()
        # intialize the list counter
        listindex = -1

        # search and replace in current file printing to the user changed lines
        for currentline in readlines:

            # increment the list counter
            listindex = listindex + 1

            # if the regexp is found
            if cregex.search(currentline):

                # make the substitution
                f=re.sub(searchregx,replacestring,currentline)

                # print the current filename, the old string and the new string
                print '\n' + xfile
                print '- ' + currentline ,
                if currentline[-1:]!='\n': print '\n' ,
                print '+ ' + f ,
                if f[-1:]!='\n': print '\n' ,

                # if substitution is real
                if simulation == 0:

                    # if substitution is step by step
                    if stepbystep == 1:

                        # ask user if the current line must be replaced
                        question = raw_input('write(Y), skip (n), quit (q) ? ')
                        question = string.lower(question)

                        # if quit
                        if question=='q':
                            sys.exit('\ninterrupted by the user !!!')

                        # if skip
                        elif question=='n':
                            pass

                        # if write
                        else:

                            # update the whole file variable ('readlines')
                            readlines[listindex] = f
                            replaceflag=1

                    # if substitution is not step by step
                    else:

                            # update the whole file variable ('readlines')
                            readlines[listindex] = f
                            replaceflag=1

        # if some text was replaced
        # overwrite the original file
        if replaceflag==1:

            # open the file for writting
            write_file=open(xfile,'w')

            # overwrite the file
            for line in readlines:
                write_file.write(line)

            # close the file
            write_file.close()



def replace_word_in_files(fileslist, searchword, replaceword, simulation = False, stepbystep = False):

  replace_in_files(fileslist,"\\b" + searchword + "\\b",replaceword,simulation,stepbystep)

def rename_class(filelist, oldname, newname,classPrefix = "mitk" ):


  suffixes = [ "h","cpp","txx" ]
  for suffix in suffixes:
    origName = classPrefix + oldname + "." + suffix
    newName = classPrefix + newname + "." + suffix
    fileName = filelist.contains(origName)
    if fileName:
      replace_word_in_files(filelist.filelist,origName,newName)
      filelist.rename_file(fileName,newName)

  replace_word_in_files(filelist.filelist,oldname,newname)


  prefixes = [ "Get" , "Set" , "m_" ]
  newnames = map(lambda x : x + newname , prefixes)

  if filelist.exists_somewhere(newnames):
     print "Skipping member variable and getter/setter renaming due to name conflict"
     return

  for prefix in prefixes:
    replace_word_in_files(filelist.filelist,prefix + oldname, prefix + newname)

x = FileList(sys.argv[1])

if len(sys.argv) == 4:
  rename_class(x,sys.argv[2],sys.argv[3])

if len(sys.argv) == 3:
  csvReader = csv.reader(open(sys.argv[2],'r'))
  for row in csvReader:
    print row
    rename_class(x,row[0],row[1])
    if commitCommandString:
      os.system(commitCommandString % ( row[0],row[1] ) )
