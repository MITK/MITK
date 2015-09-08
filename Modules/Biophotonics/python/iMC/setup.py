# -*- coding: utf-8 -*-
"""
Created on Fri Aug  7 18:41:50 2015

@author: wirkert
"""



from setuptools import setup, find_packages

setup(name= 'MultiSpectralImaging',
      version= '0.1',
      description= 'Multi spectral imaging utilities',
      author= 'Sebastian Wirkert',
      author_email= 's.wirkert@dkfz-heidelberg.de',
      license = 'BSD',
      packages= find_packages(where='msi', exclude=['test*']),
      package_dir = {'':'msi'},
      package_data= {'data':['*.txt']},
      install_requires= ['numpy>=1.8.2', 'SimpleITK>=0.9.0', 'scipy>=0.15.1'],
      entry_points= {} # for scritps, add later
     )
