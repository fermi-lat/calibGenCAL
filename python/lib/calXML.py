"""
CAL XML base classes
"""


__facility__  = "Offline"
__abstract__  = "CAL XML base classes"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/05/12 21:31:59 $"
__version__   = "$Revision: 1.8 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import Ft.Xml.Domlette 
import Ft.Lib.Uri

from calExcept import *


MODE_CREATE     = 0
MODE_READONLY   = 2



class calXML:
    """
    CAL XML file base class.

    This class provides methods for accessing CAL data stored in XML format.
    """

    
    def __init__(self, fileName, mode = MODE_READONLY, validating = False):
        """
        Open a CAL XML file

        Param: fileName The XML file name.
        Param: mode The file access mode (MODE_READONLY or MODE_CREATE).
        Param: validating - True/False do document DTD validation.
        """

        self.__xmlName = fileName
        self.__doc = None
        self.__xmlFile = None
        self.__mode = mode

        uri = Ft.Lib.Uri.OsPathToUri(fileName)

        if mode == MODE_CREATE:

            # create ouput XML file

            self.__xmlFile = open(fileName, 'w')            
            
            # create output XML document

            self.__doc = Ft.Xml.Domlette.implementation.createRootNode(uri)

        elif mode == MODE_READONLY:

            # open input XML file

            self.__xmlFile = open(fileName, "r")

            # parse into DOM document

            if validating:
                reader = Ft.Xml.Domlette.ValidatingReader
            else:
                reader = Ft.Xml.Domlette.NonvalidatingReader    
            try:
                self.__doc = reader.parseUri(uri)
            except Ft.Xml.ReaderException, e:
                raise calFileOpenExcept, "XML parse error: %s" % e

        else:
            raise calFileOpenExcept, "calCalibXML: mode %s not supported" % str(mode)            
        
                
    def close(self):              
        """
        Close a CAL XML file
        """

        self.__xmlFile.close()
    

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

        if self.__mode == MODE_READONLY:
            raise calFileWriteExcept, "File %s opened in READONLY mode" % self.__xmlName
        Ft.Xml.Domlette.PrettyPrint(self.__doc, self.__xmlFile)

        
