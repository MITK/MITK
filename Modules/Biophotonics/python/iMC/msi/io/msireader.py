'''
Created on Aug 25, 2015

@author: wirkert
'''

import pickle

class MsiReader(object):
    '''
    The MsiReader reads the Msi from the serialized python object.
    This is the prefered way of reading an Msi.
    '''

    def __init__(self):
        pass

    def read(self, filename_to_read):
        msi_file = open(filename_to_read, 'r')
        msi = pickle.load(msi_file)
        msi_file.close()
        return msi
