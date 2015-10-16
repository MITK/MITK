'''
Created on Oct 15, 2015

@author: wirkert
'''

from mc.tissuemodels import AbstractTissue


class AbstractMcFactory(object):
    '''
    Monte Carlo Factory.
    Will create fitting models and batches, dependent on your task
    '''

    def create_tissue_model(self):
        return AbstractTissue()

    def create_batch_to_simulate(self):
        return AbstractBatch()

    def __init__(self, params):
        '''
        Constructor
        '''
