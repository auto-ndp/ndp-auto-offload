from distutils.core import setup, Extension

module1 = Extension('pinnearmap',
                    sources = ['stublib.c'])

setup (name = 'PinNearMAP',
       version = '1.0',
       description = 'PinNearMAP instrumentation callback',
       ext_modules = [module1])