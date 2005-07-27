"""
CAL XML base classes
"""


__facility__  = "Offline"
__abstract__  = "CAL XML base classes"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/04/12 13:58:31 $"
__version__   = "$Revision: 1.1 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import logging
import xml.dom.minidom
import xml.dom.ext

from calExcept import *


MODE_CREATE     = 0
MODE_READONLY   = 2



class calXML:
    """
    CAL XML file base class.

    This class provides methods for accessing CAL data stored in XML format.
    """

    
    def __init__(self, fileName, mode = MODE_READONLY):
        """
        Open a CAL XML file

        \param fileName The XML file name.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        """

        self.__log = logging.getLogger()    

        self.__xmlName = fileName
        self.__doc = None
        self.__xmlFile = None
        self.__mode = mode

        if mode == MODE_CREATE:

            # create ouput XML file

            self.__xmlFile = open(fileName, 'w')            
            
            # create output XML document

            impl = xml.dom.minidom.getDOMImplementation()
            self.__doc = impl.createDocument(None, None, None)

        elif mode == MODE_READONLY:

            # open input XML file

            self.__xmlFile = open(fileName, "r")

            # parse into DOM document

            self.__doc = xml.dom.minidom.parse(self.__xmlFile)

        else:
            raise calFileOpenExcept, "calCalibXML: mode %s not supported" % str(mode)            
        
                
    def close(self):              
        """
        Close a CAL XML file
        """
        
        self.__xmlFile.close()
        self.__doc.unlink()
    

    def getDoc(self):
        """
        Get the XML DOM document object.

        Returns: A reference to the DOM document.
        """

        return self.__doc
    

    def getFile(self):
        """
        Get the XML file object.

        Returns: A reference to the XML file handle.
        """

        return self.__xmlFile    
    

    def getMode(self):
        """
        Get the XML file access mode.

        Returns: MODE_CREATE or MODE_READONLY.
        """

        return self.__mode


    def getLog(self):
        """
        Get the CAL XML logger object.

        Returns: A reference to a logging object.
        """

        return self.__log


    def getName(self):
        """
        Get the XML file name.

        Returns: The file name string.
        """

        return self.__xmlName


    def writeFile(self):
        """
        Write the XML document data to the file.
        """

        xml.dom.ext.PrettyPrint(self.__doc, self.__xmlFile)

        