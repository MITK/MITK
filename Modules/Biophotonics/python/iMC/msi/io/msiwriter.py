'''
Created on Aug 25, 2015

@author: wirkert
'''

import pickle

from writer import Writer

class MsiWriter(Writer):
    '''
    The MsiReader writing the Msi as a serialized python object.
    '''

    def __init__(self, msiToWrite):
        """
        initialize the write with a specific multi spectral image (class Msi)
        """
        self.msiToWrite = msiToWrite

    def write (self, uri):
        f = open(uri, 'w')
        pickle.dump(self.msiToWrite, f)
        f.close()
