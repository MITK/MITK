'''
Created on Oct 15, 2015

@author: wirkert
'''

from mc.tissuemodels import AbstractTissue, GenericTissue, PhantomTissue
from mc.batches import AbstractBatch
from mc.batches import GenericBatch, LessGenericBatch, GenericMeanScatteringBatch
from mc.batches import ColonMuscleBatch, ColonMuscleMeanScatteringBatch
from mc.batches import VisualizationBatch, IntralipidPhantomBatch


class AbstractMcFactory(object):
    '''
    Monte Carlo Factory.
    Will create fitting models and batches, dependent on your task
    '''

    def create_tissue_model(self):
        return AbstractTissue()

    def create_batch_to_simulate(self):
        return AbstractBatch()

    def __init__(self):
        '''
        Constructor
        '''


class GenericMcFactory(AbstractMcFactory):

    def create_tissue_model(self):
        return GenericTissue()

    def create_batch_to_simulate(self):
        return GenericBatch()

    def __init__(self):
        '''
        Constructor
        '''


class LessGenericMcFactory(GenericMcFactory):

    def create_batch_to_simulate(self):
        return LessGenericBatch()

    def __init__(self):
        '''
        Constructor
        '''


class ColonMuscleMcFactory(GenericMcFactory):

    def create_batch_to_simulate(self):
        return ColonMuscleBatch()

    def __init__(self):
        '''
        Constructor
        '''


class GenericMeanScatteringFactory(GenericMcFactory):

    def create_batch_to_simulate(self):
        return GenericMeanScatteringBatch()

    def __init__(self):
        '''
        Constructor
        '''


class ColonMuscleMeanScatteringFactory(GenericMcFactory):

    def create_batch_to_simulate(self):
        return ColonMuscleMeanScatteringBatch()

    def __init__(self):
        '''
        Constructor
        '''


class VisualizationMcFactory(AbstractMcFactory):

    def create_tissue_model(self):
        return GenericTissue()

    def create_batch_to_simulate(self):
        return VisualizationBatch()

    def __init__(self):
        '''
        Constructor
        '''


class PhantomFactory(AbstractMcFactory):

    def create_tissue_model(self):
        return PhantomTissue()

    def create_batch_to_simulate(self):
        return IntralipidPhantomBatch()

    def __init__(self):
        '''
        Constructor
        '''
