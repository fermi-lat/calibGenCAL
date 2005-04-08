"""
calDacXML
"""

import xml.dom.minidom
import Numeric
import logging

from calExcept import *



MODE_CREATE     = 0
MODE_READONLY   = 2


FILE_TYPES = ('fle_dac', 'fhe_dac', 'log_acpt', 'rng_uld_dac')



##########################################################################################
##
## \class calDacXML
##
## \brief CAL DAC configuration XML data file class.
##
## This class provides methods for accessing CAL DAC configuration data stored in XML
## format.
##
## The following file types are supported: \n
##
## - fle_dac CAL FLE DAC setttings \n
## - fhe_dac CAL FHE DAC settings \n
## - log_acpt CAL LAC DAC settings \n
## - rng_uld_dac CAL ULD DAC settings \n
##
##########################################################################################

class calDacXML:   
  
    def __init__(self, fileName, dacName, mode = MODE_READONLY):
        """
        \brief Open a CAL DAC configuration XML file

        \param fileName The XML file name.
        \param dacName The name of the bottom level XML data element.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """

        self.__log = logging.getLogger()    

        self.__doc = None
        self.__xmlFile = None
        self.__xmlName = fileName
        self.__mode = mode

        if dacName not in FILE_TYPES:
            raise calFileOpenExcept, "DAC name %s not supported" % str(dacName)
        self.__dacName = dacName        

        if mode == MODE_READONLY:

            # open and parse DAC configuration XML file

            self.__xmlFile = open(fileName, "r")
            self.__doc = xml.dom.minidom.parse(self.__xmlFile)

        else:
            raise calFileOpenExcept, "file access mode %s not supported" % str(mode)


    def read(self):
        """
        \brief Read data from a CAL DAC configuration XML file

        \returns A Numeric array of data (16, 8, 2, 12) read from the XML file.
        """

        # create empty DAC data array

        dacData = Numeric.zeros((16, 8, 2, 12), Numeric.Int16)

        # find <fle_dac> elements

        latList = self.__doc.getElementsByTagName('GLAT')
        latLen = len(latList)
        if latLen != 1:
            raise calFileReadExcept, "found %d <GLAT> elements, expected 1" % latLen
        temList = latList[0].getElementsByTagName('GTEM')
        temLen = len(temList)
        if temLen > 16:
            raise calFileReadExcept, "found %d <GTEM> elements, expected <= 16" % temLen
        for t in temList:
            tem = int(t.getAttribute('ID'))
            if tem < 0 or tem > 16:
                raise calFileReadExcept, "<GTEM> ID attribute value %d, expected (0 - 15)" % tem
            cccList = t.getElementsByTagName('GCCC')
            cccLen = len(cccList)
            if cccLen > 4:
                raise calFileReadExcept, "found %d <GCCC> elements, expected <= 4" % cccLen
            for c in cccList:
                ccc = int(c.getAttribute('ID'))
                if ccc < 0 or ccc > 3:
                    raise calFileReadExcept, "<GCCC> ID attribute value %d, expected (0 - 3)" % ccc
                rcList = c.getElementsByTagName('GCRC')
                rcLen = len(rcList)
                if rcLen > 4:
                    raise calFileReadExcept, "found %d <GCRC> elements, expected <= 4" % rcLen
                for r in rcList:
                    rc = int(r.getAttribute('ID'))
                    if rc < 0 or rc > 3:
                        raise calFileReadExcept, "<GCRC> ID attribute value %d, expected (0 - 3)" % rc
                    feList = r.getElementsByTagName('GCFE')
                    feLen = len(feList)
                    if feLen > 12:
                        raise calFileReadExcept, "found %d <GCFE> elements, expected <= 12" % feLen
                    for f in feList:
                        fe = int(f.getAttribute('ID'))
                        if fe < 0 or fe > 11:
                            raise calFileReadExcept, "<GCFE> ID attribute value %d, expected (0 - 11)" % fe
                        dacList = f.getElementsByTagName(self.__dacName)
                        dacLen = len(dacList)
                        if dacLen != 1:
                            raise calFileReadExcept, "found %d %s elements, expected 1" % (dacLen, self.__dacName)
                        d = dacList[0]
                        dd = d.childNodes[0]
                        dac = int(dd.data.strip(), 16)
                        if (ccc % 2) != 0:
                            row = (rc + 4)
                        else:
                            row = rc
                        if ccc < 2:
                            end = 1
                        else:
                            end = 0
                        dacData[tem, row, end, fe] = dac

        return dacData                        


    def close(self):
        """
        \brief Close a CAL DAC configuration XML file.
        """
        
        self.__xmlFile.close()
        self.__doc.unlink()
