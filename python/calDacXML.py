"""
Classes to represent CAL DAC settings XML documents.
"""


__facility__  = "Offline"
__abstract__  = "Classes to represent CAL DAC settings XML documents"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/12 14:03:16 $"
__version__   = "$Revision: 1.2 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"



import logging
import xml.dom.minidom

import Numeric

import calXML
from calExcept import *



MODE_CREATE     = calXML.MODE_CREATE
MODE_READONLY   = calXML.MODE_READONLY


FILE_TYPES = ('fle_dac', 'fhe_dac', 'log_acpt', 'rng_uld_dac')




class calSnapshotXML(calXML.calXML):
    """
    CAL hardware configuration snapshot XML data file class.
    
    This class provides methods for accessing CAL HW configuration
    data stored in XML snapshot file format.
    """

    def __init__(self, fileName, mode = MODE_READONLY):
    
        calXML.calXML.__init__(self, fileName, mode)

        
    def read(self, dacName):
        """
        Read CAL data from a snapshot XML file

        Param: dacName - The name of the DAC element to read.  The following
        HW items are supported:

            fle_dac - CAL FLE DAC setttings
            fhe_dac - CAL FHE DAC settings
            log_acpt - CAL LAC DAC settings
            rng_uld_dac - CAL ULD DAC settings        

        Returns: A Numeric array of data (16, 8, 2, 12) read from the
        XML file.
        """

        # create empty DAC data array

        dacData = Numeric.zeros((16, 8, 2, 12), Numeric.Int16)

        # find <fle_dac> elements

        doc = self.getDoc()

        latList = doc.getElementsByTagName('GLAT')
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
                        dacList = f.getElementsByTagName(dacName)
                        dacLen = len(dacList)
                        if dacLen != 1:
                            raise calFileReadExcept, "found %d %s elements, expected 1" % (dacLen, dacName)
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

    

class calDacXML(calSnapshotXML):   
    """
    CAL DAC configuration XML data file class.
    
    This class provides methods for accessing CAL DAC configuration
    data stored in XML format.  These files are fragments of
    full HW snapshot records.

    The following file types are supported:

        fle_dac - CAL FLE DAC setttings
        fhe_dac - CAL FHE DAC settings
        log_acpt - CAL LAC DAC settings
        rng_uld_dac - CAL ULD DAC settings
        
    """

    def __init__(self, fileName, dacName, mode = MODE_READONLY):
        """
        Open a CAL DAC configuration XML file

        \param fileName The XML file name.
        \param dacName The name of the bottom level XML data element.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """

        calSnapshotXML.__init__(self, fileName, mode)  

        if dacName not in FILE_TYPES:
            raise calFileOpenExcept, "DAC name %s not supported" % str(dacName)
        self.__dacName = dacName


    def read(self):
        """
        Read data from a CAL DAC configuration XML file

        Returns: A Numeric array of data (16, 8, 2, 12) read from the
        XML file.
        """

        return calSnapshotXML.read(self, self.__dacName)



