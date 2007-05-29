"""
Classes and functions to read and write CAL XML files derived from FITS data sets.
"""


__facility__  = "Offline"
__abstract__  = "Class to read and write CAL XML files derived from FITS data sets"
__author__    = "D.L.Wood"
__date__      = "$Date: 2006/10/11 21:11:11 $"
__version__   = "$Revision: 1.8 $, $Author: dwood $"
__release__   = "$Name: v4r4 $"
__credits__   = "NRL code 7650"


import os, time
import logging

import Numeric

import calXML
import calConstant
from calExcept import *



MODE_CREATE = calXML.MODE_CREATE
MODE_READONLY = calXML.MODE_READONLY


FILE_TYPES = ('fhe_dac', 'fle_dac', 'log_acpt', 'pedestal value', 'relative gain factor', 'rng_uld_dac')




class calFitsXML(calXML.calXML):
    """
    CAL FITS/XML ancillary data file class.

    This class provides methods for accessing CAL FITS ancillary data stored in XML
    format.  The following file types are supported:

    - fle_dac: FLE DAC characterization
    - fhe_dac: FHE DAC characterization
    - log_acpt: LAC DAC characterization
    - rng_uld_dac: LAC DAC characterization
    - pedestal value: pedestal values 
    - relative gain factor: relative gain factor values
    """
    
  
    def __init__(self, filePath = None, fileName = None, mode = MODE_READONLY, labels = None, \
                calSNs = None, dataset = None, lrefgain = None, hrefgain = None, pedFile = None, \
                erng = None, reportName = None, runId = None, comment = None, fitsName = None, \
                fitsTime = None, type = None, instrument = None, version = 2):
        """
        Open a CAL FITS data XML file

        Param: filePath The XML file directory path.
        Param: fileName The XML file name.
        Param: mode The file access mode (MODE_READONLY or MODE_CREATE).
        Param: labels A sequence of FITS table header values:
            labels[0] = Table header LAXIS1 string value
            labels[1] = Table header LAXIS2 string value
              ...
            labels[-2] = Table header LAXIS<n> string value.
            labels[-1] = Table header TTYPE1 string value.
        Param: calSNs A dictionary of up to 4 table header CALSN<n> string values:
            calSNs['X-'] = CALSNX- header key value
            calSNs['Y-'] = CALSNY- header key value
            calSNs['X+'] = CALSNX+ header key value
            calSNs['Y+'] = CALSNY+ header key value
        Param: dataset The table header DATASET string value.
        Param: lrefgain The table header LREFGAIN integer value.
        Param: hrefgain The table header HREFGAIN integer value.
        Param: pedFile The table header PEDFILE string value.
        Param: erng The table header ERNG string value.
        Param: reportName The name of the associated test report file.
        Param: runId The run collection ID for the data.
        Param: comment The table header COMMENT string value.
        Param: fitsName The name of the FITS file from which the XML file is derived.
        Param: fitsTime The creation time of the FITS file from which the XML file is derived.
        Param: instrument The table header INSTRUMENT string value.
        Param: version The format version number of the CAL XML file.
        """

        self.__numAxes = 7
        self.__numSn = 4

        self.__log = logging.getLogger('calFitsXML')    

        self.__doc = None
        self.__rootNode = None

        self.__fileName = fileName      
        if filePath is None:
          self.__filePath = ""
        else:
          self.__filePath = filePath
          
        self.__mode = mode
        self.__labels = labels
        self.__calSNs = calSNs
        self.__dataset = dataset
        self.__lrefgain = lrefgain
        self.__hrefgain = hrefgain
        self.__pedFile = pedFile
        self.__erng = erng   #LEX8/LEX1/HEX8/HEX1
        self.__fitsName = fitsName
        self.__fitsTime = fitsTime
        self.__xmlVersion = version
        self.__reportName = reportName
        self.__runId = runId
        self.__comment = comment
        self.__type = type
        self.__instrument = instrument

        path = os.path.join(self.__filePath, self.__fileName)    

        # open file

        calXML.calXML.__init__(self, path, mode) 
        self.__fileName = self.getName()       
        self.__doc = self.getDoc()
        

    def getTowers(self):
        """
        Get the ID's of towers contributing to the data file.

        Returns: A list of tower ID's.        
        """

        # get XML document type

        dList = self.__doc.xpath('.//CALdoc')
        dLen = len(dList)
        if dLen != 1:
            raise calFileReadExcept, "wrong number of <CAL_doc> elements: %u (expected 1)" % dLen
        d = dList[0]
        type = d.getAttributeNS(None, 'type')
        if len(type) == 0:
            raise calFileReadExcept, "<CALdoc> element requires type attribute"
        tList = type.split(':')
        if len(tList) != 2 or tList[0] != 'FITS' or tList[1] not in FILE_TYPES:
            raise calFileReadExcept, "<CALdoc> type attribute %s not recognized" % type 
        self.__type = tList[1]

        # determine table element name

        if self.__type == 'fhe_dac' or self.__type == 'fle_dac' or self.__type == 'log_acpt' or \
           self.__type == 'rng_uld_dac' or self.__type == 'pedestal value':
            tableName = 'adc_table'

        elif self.__type == 'relative gain factor':
            tableName = 'gain_table'

        else:
            raise calFileReadExcept, "invalid file type: %s" % self.__type        

        towers = []

        # find <adc_table> element

        aList = self.__doc.xpath('.//%s' % tableName)
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 1)" % (tableName, aNum)

        # find <tem> elements
            
        tList = aList[0].xpath('.//tem')
        tNum = len(tList)
        if tNum == 0 or tNum > 16:
            raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1-16)" % tNum 
        for t in tList:
            tem = int(t.getAttributeNS(None, 'num'))
            if tem < 0 or tem > 15:
                raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem
            towers.append(tem)

        return(towers)        


    def write(self, data, tems = (0,)):
        """
        Write data to a CAL FITS file

        Param: rawEventData - A Numeric array of data to be written in the FITS table.
        Param: tems - A list of TEM ID values to include in the output.
        """
        
        if self.__mode == MODE_READONLY:
            raise calFileWriteExcept, "XML file %s opened in read-only mode" % self.__fileName

        if self.__type == 'fhe_dac' or self.__type == 'fle_dac' or self.__type == 'log_acpt':
            self.__writeDAC(data, tems)
            
        elif self.__type == 'pedestal value':
            self.__writePED(data, tems)

        elif self.__type == 'relative gain factor':
            self.__writeREL(data, tems)

        elif self.__type == 'rng_uld_dac':
            self.__writeULD(data, tems)

        else:
            raise calFileWriteExcept, "invalid file type: %s" % self.__type

        self.writeFile()            
            

    def __writeDAC(self, data, tems):            

        # verify ADC data array shape

        shape = (calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE, 128)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        a = self.__doc.createElementNS(None, 'adc_table')
        self.__rootNode.appendChild(a)
        
        for tem in tems:

            t = self.__doc.createElementNS(None, 'tem')
            t.setAttributeNS(None, 'num', str(tem))
            a.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):
                for end in range(calConstant.NUM_END):
            
                    l = self.__doc.createElementNS(None, 'row')
                    l.setAttributeNS(None, 'num', str(layer))
                    l.setAttributeNS(None, 'end', str(end))
                    l.setAttributeNS(None, 'name', '%s%s' % (calConstant.CROW[layer], calConstant.CPM[end]))
                    t.appendChild(l)

                    for fe in range(calConstant.NUM_FE):

                        f = self.__doc.createElementNS(None, 'fe')
                        f.setAttributeNS(None, 'num', str(fe))
                        l.appendChild(f)
                        
                        s = ''
                        for dac in range(128):               
                            s += '%0.03f,' % data[tem, layer, end, fe, dac]
                            
                        f.appendChild(self.__doc.createTextNode(s.rstrip(',')))


    def __writePED(self, data, tems):

        shape = (9, calConstant.NUM_RNG, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        a = self.__doc.createElementNS(None, 'adc_table')
        self.__rootNode.appendChild(a)

        for tem in tems:

            t = self.__doc.createElementNS(None, 'tem')
            t.setAttributeNS(None, 'num', str(tem))
            a.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):
                for end in range(calConstant.NUM_END):
                
                    l = self.__doc.createElementNS(None, 'row')
                    l.setAttributeNS(None, 'num', str(layer))
                    l.setAttributeNS(None, 'end', str(end))
                    l.setAttributeNS(None, 'name', '%s%s' % (calConstant.CROW[layer], calConstant.CPM[end]))
                    t.appendChild(l)

                    for fe in range(calConstant.NUM_FE):

                        f = self.__doc.createElementNS(None, 'fe')
                        f.setAttributeNS(None, 'num', str(fe))
                        l.appendChild(f)

                        for erng in range(calConstant.NUM_RNG):

                            e = self.__doc.createElementNS(None, 'erng')
                            e.setAttributeNS(None, 'num', str(erng))
                            e.setAttributeNS(None, 'name', calConstant.CRNG[erng])
                            f.appendChild(e)

                            s = ''                        
                            for gain in range(9):
                                s += '%0.03f,' % data[gain, erng, layer, end, fe]
                            
                            e.appendChild(self.__doc.createTextNode(s.rstrip(',')))
                                                        

    def __writeREL(self, data, tems):

        shape = (9, calConstant.NUM_RNG, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, 
            calConstant.NUM_FE)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        g = self.__doc.createElementNS(None, 'gain_table')
        self.__rootNode.appendChild(g)

        for tem in tems:

            t = self.__doc.createElementNS(None, 'tem')
            t.setAttributeNS(None, 'num', str(tem))
            g.appendChild(t)
        
            for layer in range(calConstant.NUM_LAYER):
                for end in range(calConstant.NUM_END):
                
                    l = self.__doc.createElementNS(None, 'row')
                    l.setAttributeNS(None, 'num', str(layer))
                    l.setAttributeNS(None, 'end', str(end))
                    l.setAttributeNS(None, 'name', '%s%s' % (calConstant.CROW[layer], calConstant.CPM[end]))
                    t.appendChild(l)

                    for fe in range(calConstant.NUM_FE):

                        f = self.__doc.createElementNS(None, 'fe')
                        f.setAttributeNS(None, 'num', str(fe))
                        l.appendChild(f)

                        for erng in range(calConstant.NUM_RNG):

                            e = self.__doc.createElementNS(None, 'erng')
                            e.setAttributeNS(None, 'num', str(erng))
                            e.setAttributeNS(None, 'name', calConstant.CRNG[erng])
                            f.appendChild(e)

                            s = ''                        
                            for gain in range(9):
                                s += '%0.03f,' % data[gain, erng, tem, layer, end, fe]
                            
                            e.appendChild(self.__doc.createTextNode(s.rstrip(',')))


    def __writeULD(self, data, tems):            

        # verify ADC data array shape

        shape = (3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END, calConstant.NUM_FE, 128)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        a = self.__doc.createElementNS(None, 'adc_table')
        self.__rootNode.appendChild(a)

        for tem in tems:
            
            t = self.__doc.createElementNS(None, 'tem')
            t.setAttributeNS(None, 'num', str(tem))
            a.appendChild(t)
            
            for layer in range(calConstant.NUM_LAYER):
                for end in range(calConstant.NUM_END):
                
                    l = self.__doc.createElementNS(None, 'row')
                    l.setAttributeNS(None, 'num', str(layer))
                    l.setAttributeNS(None, 'end', str(end))
                    l.setAttributeNS(None, 'name', '%s%s' % (calConstant.CROW[layer], calConstant.CPM[end]))
                    t.appendChild(l)

                    for fe in range(calConstant.NUM_FE):

                        f = self.__doc.createElementNS(None, 'fe')
                        f.setAttributeNS(None, 'num', str(fe))
                        l.appendChild(f)
                        
                        for erng in range(3):

                            e = self.__doc.createElementNS(None, 'erng')
                            e.setAttributeNS(None, 'num', str(erng))
                            e.setAttributeNS(None, 'name', calConstant.CRNG[erng])
                            f.appendChild(e)

                            s = ''
                            for dac in range(128):               
                                s += '%0.03f,' % data[erng, tem, layer, end, fe, dac]
                            
                            e.appendChild(self.__doc.createTextNode(s.rstrip(',')))
                    

                    
    def __writeHeader(self, data):

        ts = time.strftime('%y%m%d%H%M%S', time.gmtime())        

        # create document root node

        self.__rootNode = self.__doc.createElementNS(None, 'CALdoc')
        self.__rootNode.setAttributeNS(None, 'name', self.__fileName)
        self.__rootNode.setAttributeNS(None, 'type', 'FITS:%s' % self.__type)
        self.__rootNode.setAttributeNS(None, 'version', str(self.__xmlVersion))
        self.__rootNode.setAttributeNS(None, 'time', ts)
        self.__doc.appendChild(self.__rootNode)

        # create 'header_keys' element to hold FITS key string values

        hdrNode = self.__doc.createElementNS(None, 'header_keys')
        self.__rootNode.appendChild(hdrNode)            

        self.__writeKey(hdrNode, 'TTYPE1', self.__type)

        size = (Numeric.size(data) * data.itemsize())   
        self.__writeKey(hdrNode, 'TFORM1', '1PB(%u)' % size)

        self.__writeKey(hdrNode, 'TYPECODE', data.typecode())

        self.__writeKey(hdrNode, 'SHAPE', data.shape)

        self.__writeKey(hdrNode, 'EXTNAME', 'GLAST CAL Ancillary Data')

        if self.__fitsName is not None:
            self.__writeKey(hdrNode, 'FITSNAME', self.__fitsName)

        if self.__fitsTime is not None:
            self.__writeKey(hdrNode, 'FITSTIME', self.__fitsTime)

        if self.__dataset is not None:
            self.__writeKey(hdrNode, 'DATASET', self.__dataset)

        if self.__pedFile is not None:
            self.__writeKey(hdrNode, 'PEDFILE', self.__pedFile)

        if self.__lrefgain is not None:
            self.__writeKey(hdrNode, 'LREFGAIN', self.__lrefgain)

        if self.__hrefgain is not None:
            self.__writeKey(hdrNode, 'HREFGAIN', self.__hrefgain)

        if self.__erng is not None:
            self.__writeKey(hdrNode, 'ERNG', self.__erng)

        if self.__reportName is not None:
            self.__writeKey(hdrNode, 'RPTNAME', self.__reportName)

        if self.__runId is not None:
            self.__writeKey(hdrNode, 'RUNID', self.__runId)

        if self.__comment is not None:
            self.__writeKey(hdrNode, 'COMMENT', self.__comment)
            
        if self.__instrument is not None:
            self.__writeKey(hdrNode, 'INSTRUMENT', self.__instrument)    

        try:
          keys = self.__calSNs.keys()
          (temId,dummy) = keys[0].split(':')
          temId += ':'
        except:
          temId =""
        for n in range(self.__numSn):
            key = '%s%s' %(temId,calConstant.CFACE[n])
            name = 'CALSN%s' %(calConstant.CFACE[n])
            try:
                sn = self.__calSNs[key]
            except:
                continue        
            self.__writeKey(hdrNode, name, sn)

        if self.__labels is not None:
            n = 1
            for l in self.__labels[:-1]:
                name = 'LAXIS%d' % n
                n = n + 1
                self.__writeKey(hdrNode, name, l)


    def __writeKey(self, hdrNode, name, value):

        k = self.__doc.createElementNS(None, 'key')
        k.setAttributeNS(None, 'name', str(name))
        k.appendChild(self.__doc.createTextNode(str(value)))
        hdrNode.appendChild(k)


    def getVersion(self):
        """
        Get the format version of a CAL FITS/XML file

        Returns: The version number of the XML file, taken from the
                 <CALdoc> attribute.
        """

        # get XML root document element

        dList = self.__doc.xpath('.//CALdoc')
        dLen = len(dList)
        if dLen != 1:
            raise calFileReadExcept, "wrong number of <CAL_doc> elements: %u (expected 1)" % dLen
        d = dList[0]

        # get XML format version

        xmlVersion = int(d.getAttributeNS(None, 'version'))
        return xmlVersion


    def read(self):
        """
        Read data from a CAL FITS/XML file

        Returns: A Numeric array of data read from the FITS data table.
        """

        # get XML document type

        dList = self.__doc.xpath('.//CALdoc')
        dLen = len(dList)
        if dLen != 1:
            raise calFileReadExcept, "wrong number of <CAL_doc> elements: %u (expected 1)" % dLen
        d = dList[0]
        type = d.getAttributeNS(None, 'type')
        if len(type) == 0:
            raise calFileReadExcept, "<CALdoc> element requires type attribute"
        tList = type.split(':')
        if len(tList) != 2 or tList[0] != 'FITS' or tList[1] not in FILE_TYPES:
            raise calFileReadExcept, "<CALdoc> type attribute %s not recognized" % type 
        self.__type = tList[1]

        # get XML format version

        self.__xmlVersion = int(d.getAttributeNS(None, 'version'))
        err = "XML format version %d not supported for file type %s" % (self.__xmlVersion, self.__type)  
        if self.__xmlVersion > 2:
            raise calFileReadExcept, err
        if self.__type == 'rng_uld_dac' and self.__xmlVersion < 1:
            raise calFileReadExcept, err
                  
        # run type-specific reader
        
        if self.__type == 'fhe_dac' or self.__type == 'fle_dac' or self.__type == 'log_acpt':
            return self.__readDAC()

        elif self.__type == 'pedestal value':
            return self.__readPED()

        elif self.__type == 'relative gain factor':
            return self.__readREL()

        elif self.__type == 'rng_uld_dac':
            return self.__readULD()
        
        else:
            raise calFileReadExcept, "<CALdoc> type %s not supported" % type
    

    def __readDAC(self):        

        data = Numeric.zeros((calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                              calConstant.NUM_FE, 128), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'

        # find <adc_table> element

        aList = self.__doc.xpath('.//adc_table')
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <adc_table> elements: %u (expected 1)" % aNum

        # find <tem> elements

        tList = aList[0].xpath('.//tem')
        tNum = len(tList)
        if tNum == 0 or tNum > 16:
            raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1-16)" % tNum        
        
        for t in tList:
            
            tem = int(t.getAttributeNS(None, 'num'))
            if tem < 0 or tem > 15:
                raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem

            # find <layer> or <row> elements
            
            lList = t.xpath('.//%s' % elName)
            lNum = len(lList)
            if lNum != 16:
                raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
            
            for l in lList:
                layer = int(l.getAttributeNS(None, 'num'))
                end = int(l.getAttributeNS(None, 'end'))
                if layer < 0 or layer > 7:
                    raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                if end < 0 or end > 1:
                    raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)

                # find <fe> elements
                
                fList = l.xpath('.//fe')
                fNum = len(fList)
                if fNum != 12:
                    raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                for f in fList:
                    fe = int(f.getAttributeNS(None, 'num'))
                    if fe < 0 or fe > 11:
                        raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe
                    d = f.childNodes[0]
                    s = d.data.strip()
                    adcList = s.split(',')
                    dNum = len(adcList)
                    if dNum != 128:
                        raise calFileReadExcept, "wrong number of data points: %u (expected 128)" % dNum
                    for dac in range(128):
                        data[tem, layer, end, fe, dac] = float(adcList[dac])
                            
        return data


    def __readPED(self):       

        data = Numeric.zeros((9, calConstant.NUM_RNG, calConstant.NUM_ROW, calConstant.NUM_END,
                              calConstant.NUM_FE), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'        

        # find <adc_table> element

        aList = self.__doc.xpath('.//adc_table')
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <adc_table> elements: %u (expected 1)" % aNum

        # find <tem> elements
        
        tList = aList[0].xpath('.//tem')
        tNum = len(tList)
        if tNum != 1:
            raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1)" % tNum 
        for t in tList:
            tem = int(t.getAttributeNS(None, 'num'))
            if tem < 0 or tem > 15:
                raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem

            # find <row> or <layer> elements
            
            lList = t.xpath('.//%s' % elName)
            lNum = len(lList)
            if lNum != 16:
                raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
            for l in lList:
                layer = int(l.getAttributeNS(None, 'num'))
                if layer < 0 or layer > 7:
                    raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                end = int(l.getAttributeNS(None, 'end'))
                if end < 0 or end > 1:
                    raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)

                # find <fe> elements                
                
                fList = l.xpath('.//fe')
                fNum = len(fList)
                if fNum != 12:
                    raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                for f in fList:
                    fe = int(f.getAttributeNS(None, 'num'))
                    if fe < 0 or fe > 11:
                        raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe

                    # find <erng> elements
                    
                    eList = f.xpath('.//erng')
                    eLen = len(eList)
                    if eLen != 4:
                        raise calFileReadExcept, "wrong number of <erng> elments: %u (expected 4" % eNum
                    for e in eList:
                        erng = int(e.getAttributeNS(None, 'num'))
                        if erng < 0 or erng > 3:
                            raise calFileReadExcept, "read <erng> element attribute \'num\' value %d (expected 1-3)" % erng
                        d = e.childNodes[0]
                        s = d.data.strip()
                        adcList = s.split(',')
                        dNum = len(adcList)
                        if dNum != 9:
                            raise calFileReadExcept, "wrong number of data points: %u (expected 9)" % dNum
                        for gain in range(9):
                            data[gain, erng, layer, end, fe] = float(adcList[gain])   

        return data                        


    def __readREL(self):       

        data = Numeric.zeros((9, calConstant.NUM_RNG, calConstant.NUM_TEM, calConstant.NUM_ROW,
                              calConstant.NUM_END, calConstant.NUM_FE), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'

        # find <gain_table> element            

        gList = self.__doc.xpath('.//gain_table')
        gNum = len(gList)
        if gNum != 1:
            raise calFileReadExcept, "wrong number of <gain_table> elements: %u (expected 1)" % gNum

        # find <tem> elements
        
        tList = gList[0].xpath('.//tem')
        tNum = len(tList)
        if tNum == 0 or tNum > 16:
            raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1-16)" % tNum
        for t in tList:
            tem = int(t.getAttributeNS(None, 'num'))
            if tem < 0 or tem > 15:
                raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem

            # find <row> or <layer> elements            

            lList = t.xpath('.//%s' % elName)
            lNum = len(lList)
            if lNum != 16:
                raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
            for l in lList:
                layer = int(l.getAttributeNS(None, 'num'))
                if layer < 0 or layer > 7:
                    raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                end = int(l.getAttributeNS(None, 'end'))
                if end < 0 or end > 1:
                    raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)

                # find <fe> elements            
                
                fList = l.xpath('.//fe')
                fNum = len(fList)
                if fNum != 12:
                    raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                for f in fList:
                    fe = int(f.getAttributeNS(None, 'num'))
                    if fe < 0 or fe > 11:
                        raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe

                    # find <erng> elements                    
                    
                    eList = f.xpath('.//erng')
                    eLen = len(eList)
                    if eLen != 4:
                        raise calFileReadExcept, "wrong number of <erng> elments: %u (expected 4" % eNum
                    for e in eList:
                        erng = int(e.getAttributeNS(None, 'num'))
                        if erng < 0 or erng > 3:
                            raise calFileReadExcept, "read <erng> element attribute \'num\' value %d (expected 0-3)" % erng
                        d = e.childNodes[0]
                        s = d.data.strip()
                        adcList = s.split(',')
                        dNum = len(adcList)
                        if dNum != 9:
                            raise calFileReadExcept, "wrong number of data points: %u (expected 9)" % dNum
                        for gain in range(9):
                            data[gain, erng, tem, layer, end, fe] = float(adcList[gain])

        return data


    def __readULD(self):        

        data = Numeric.zeros((3, calConstant.NUM_TEM, calConstant.NUM_ROW, calConstant.NUM_END,
                              calConstant.NUM_FE, 128), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'        

        # find <adc_table> element

        aList = self.__doc.xpath('.//adc_table')
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <adc_table> elements: %u (expected 1)" % aNum

        # find <tem> elements
 
        tList = aList[0].xpath('.//tem')
        tNum = len(tList)
        if tNum == 0 or tNum > 16:
            raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1-16)" % tNum 
        for t in tList:
            tem = int(t.getAttributeNS(None, 'num'))
            if tem < 0 or tem > 15:
                raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem

            # find <row> or <layer> elements

            lList = t.xpath('.//%s' % elName)
            lNum = len(lList)
            if lNum != 16:
                raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
            for l in lList:
                layer = int(l.getAttributeNS(None, 'num'))
                if layer < 0 or layer > 7:
                    raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                end = int(l.getAttributeNS(None, 'end'))
                if end < 0 or end > 1:
                    raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)

                # find <fe> elements
                
                fList = l.xpath('.//fe')
                fNum = len(fList)
                if fNum != 12:
                    raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                for f in fList:
                    fe = int(f.getAttributeNS(None, 'num'))
                    if fe < 0 or fe > 11:
                        raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe

                    # find <erng> elements
            
                    eList = f.xpath('.//erng')
                    eLen = len(eList)
                    if eLen != 3:
                        raise calFileReadExcept, "wrong number of <erng> elments: %u (expected 3)" % eNum
                    for e in eList:
                        erng = int(e.getAttributeNS(None, 'num'))
                        if erng < 0 or erng > 2:
                            raise calFileReadExcept, "read <erng> element attribute \'num\' value %d (expected 0-3)" % (erng + 1)
                        d = e.childNodes[0]
                        s = d.data.strip()
                        adcList = s.split(',')
                        dNum = len(adcList)
                        if dNum != 128:
                            raise calFileReadExcept, "wrong number of data points: %u (expected 128)" % dNum
                        for dac in range(128):
                            data[erng, tem, layer, end, fe, dac] = float(adcList[dac])
                            
        return data
    

    def info(self):
        """
        Get a CAL FITS/XML file table header values
        
        The following key values are recognized in the FITS file table header: 

          - TTYPE1 - The file type ('fle_dac', 'fhe_dac', 'log acpt', 'pedestal values', ... 
          - TFORM1 - The FITS file format
          - EXTNAME - The FITS table name -  'GLAST CAL Ancillary Data'
          - SHAPE - The Python shape tuple for the FITS data
          - TYPECODE - The Python typecode for the FITS data
          - FITSNAME - The orignal name of the FITS file
          - FITSTIME - The time the FITS file was created
          - LAXIS1 - LAXIS7 - The FITS table dimension labels.
          - CALSNX+,CALSNX-,CALSNY+,CALSNY- - The CAL AFEE board serial numbers
          - DATASET - The name of the event data file associated with the FITS data 
          - LREFGAIN - The low reference gain used during the collection of the FITS data.
          - HREFGAIN - The high reference gain used during the collection of the FITS data.
          - PEDFILE - The name of the pedestal FITS file associated with the FITS data.
          - ERNG - The energy range used during the collection of the FITS data.
          - RPTNAME - The name of the associated test report file.
          - RUNID - The run collection ID for the data.
          - COMMENT - A general comment and notes string for the FITS file.
          - INSTRUMENT - A summary name for the instrument under test.
          
        Returns: A dictionary of <table header key>:<value> pairs.
        """
        
        i = {}
        laxis = []
        sn = []

        i['SHAPE'] = None
        i['TYPECODE'] = None
        i['FITSNAME'] = None
        i['FITSTIME'] = None
        i['DATASET'] = None
        i['LREFGAIN'] = None
        i['HREFGAIN'] = None
        i['PEDFILE'] = None
        i['TTYPE1'] = None
        i['TFORM1'] = None
        i['EXTNAME'] = None
        i['ERNG'] = None
        i['RPTNAME'] = None
        i['RUNID'] = None
        i['COMMENT'] = None
        i['INSTRUMENT'] = None
        
        for n in range(self.__numAxes):
            name = 'LAXIS%d' % (n + 1)
            i[name] = None
        for n in range(self.__numSn):
            name = 'CALSN%s' % calConstant.CFACE[n]
            i[name] = None

        ikeys = i.keys()            

        hList = self.__doc.xpath('.//header_keys')
        hNum = len(hList)
        if hNum != 1:
            raise calFileReadExcept, "wrong number of <header_keys> elements: %u (expected 1)" % hNum            

        kList = hList[0].xpath('.//key')
        for k in kList:
            d = k.childNodes[0]
            name = k.getAttributeNS(None, 'name')
            s = d.data.strip()
            if name in ikeys:
                i[name] = str(s)
            else:
                self.__log.warning("unknown <key> element name: %s", name)
        
        return i    

