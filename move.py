import os
import glob
import fnmatch

basepath = 'C:\\mitk\\s\\mitk\\Plugins\\org.mitk.gui.qt.diffusionimaging\\'
targetpath = 'C:\\mitk\\s\\mitk\\Plugins\\org.mitk.gui.qt.diffusionimaging.fiberfox\\'
name = 'Fiberfox'

for root, dirnames, filenames in os.walk(basepath):
    for filename in fnmatch.filter(filenames, '*'):
        path = os.path.join(root, filename)

        if filename.__contains__(name) :
            print filename
            continue

        file = open(path,'r')
        for line in file.readlines() :
            if line.__contains__(name) :
                #print '****************************************'
                print filename
                #print line
                continue



