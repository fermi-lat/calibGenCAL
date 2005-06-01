"""
Class to read and write CAL XML files derived from FITS data sets.
"""


__facility__  = "Offline"
__abstract__  = "Class to read and write CAL XML files derived from FITS data sets"
__author__    = "D.L.Wood"
__date__      = "$Date: 2005/05/23 18:19:47 $"
__version__   = "$Revision: 1.4 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import sys, os, time
import logging
import xml.dom.minidom
import xml.dom.ext
import Numeric
from calExcept import *

from calConstant import CFACE, CROW, CPM, CRNG


MODE_CREATE = 0
MODE_READONLY = 2

FILE_TYPES = ('fhe_dac', 'fle_dac', 'log_acpt', 'pedestal value', 'relative gain factor', 'rng_uld_dac')


##########################################################################################
##
## \class calFitsXML
##
## \brief CAL FITS/XML ancillary data file class.
##
## This class provides methods for accessing CAL FITS ancillary data stored in XML
## format.  The following file types are supported: \n
##
## - fle_dac: FLE DAC characterization \n
## - fhe_dac: FHE DAC characterization \n
## - log_acpt: LAC DAC characterization \n
## - rng_uld_dac: LAC DAC characterization \n
## - pedestal value: pedestal values \n
## - relative gain factor: relative gain factor values \n
##
##########################################################################################

class calFitsXML(object):
  
    def __init__(self, filePath = None, fileName = None, mode = MODE_CREATE, labels = None, \
                calSNs = None, dataset = None, lrefgain = None, hrefgain = None, pedFile = None, \
                erng = None, reportName = None, runId = None, comment = None, fitsName = None, \
                fitsTime = None):
        """
        \brief Open a CAL FITS data XML file

        \param filePath The XML file directory path.
        \param fileName The XML file name.
        \param mode The file access mode (MODE_READONLY or MODE_CREATE).
        \param labels A sequence of FITS table header values: \n
            labels[0] = Table header \e LAXIS1 string value
            labels[1] = Table header \e LAXIS2 string value
              ...
            labels[-2] = Table header \e LAXIS<n> string value.
            labels[-1] = Table header \e TTYPE1 string value.
        \param calSNs A sequence of up to table header \e CALSN<n> string values.
        \param dataset The table header \e DATASET string value.
        \param lrefgain The table header \e LREFGAIN integer value.
        \param hrefgain The table header \e HREFGAIN integer value.
        \param pedFile The table header \e PEDFILE string value.
        \param erng The table header \e ERNG string value.
        \param reportName The name of the associated test report file.
        \param runId The run collection ID for the data.
        \param comment The table header \e COMMENT string value.
        \param fitsName The name of the FITS file from which the XML file is derived.
        \param fitsName The creation time of the FITS file from which the XML file is derived.
        """

        self.__numAxes = 7
        self.__numSn = 4

        self.__log = logging.getLogger()    

        self.__doc = None
        self.__xmlFile = None
        self.__type = None
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
        self.__xmlVersion = 1
        self.__reportName = reportName
        self.__runId = runId
        self.__comment = comment

        path = os.path.join(self.__filePath, self.__fileName)    

        # open existing file in read-only mode

        if mode == MODE_READONLY:

            self.__xmlFile = file(path, "r")         
            self.__doc = xml.dom.minidom.parse(self.__xmlFile)            

        # create a new file        

        elif mode == MODE_CREATE:  

            self.__type = labels[-1]
            if self.__type not in FILE_TYPES:
                raise calFileOpenExcept, "invalid file type: %s" % self.__type

            self.__xmlFile = file(path, "w")
            impl = xml.dom.minidom.getDOMImplementation()
            self.__doc = impl.createDocument(None, None, None)            
            
        # file access mode not supported    
         
        else:
          raise calFileOpenExcept, "Invalid FITS file mode: %s" % str(mode)


    def write(self, data):
        """
        \brief Write data to a CAL FITS file

        \param rawEventData A Numeric array of data to be written in the FITS table.
        """
        
        if self.__mode == MODE_READONLY:
            raise calFileWriteExcept, "XML file %s opened in read-only mode" % self.__fileName

        if self.__type == 'fhe_dac' or self.__type == 'fle_dac' or self.__type == 'log_acpt':
            self.__writeDAC(data)
            
        elif self.__type == 'pedestal value':
            self.__writePED(data)

        elif self.__type == 'relative gain factor':
            self.__writeREL(data)

        elif self.__type == 'rng_uld_dac':
            self.__writeULD(data)

        else:
            raise calFileWriteExcept, "invalid file type: %s" % self.__type

        xml.dom.ext.PrettyPrint(self.__doc, self.__xmlFile)            
            

    def __writeDAC(self, data):            

        # verify ADC data array shape

        shape = (16, 8, 2, 12, 128)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        a = self.__doc.createElement('adc_table')
        t = self.__doc.createElement('tem')
        t.setAttribute('num', '0')
        a.appendChild(t)
        self.__rootNode.appendChild(a)

        for layer in range(8):
            for end in range(2):
            
                l = self.__doc.createElement('row')
                l.setAttribute('num', str(layer))
                l.setAttribute('end', str(end))
                l.setAttribute('name', '%s%s' % (CROW[layer], CPM[end]))
                t.appendChild(l)

                for fe in range(12):

                    f = self.__doc.createElement('fe')
                    f.setAttribute('num', str(fe))
                    l.appendChild(f)
                    
                    s = ''
                    for dac in range(128):               
                        s += '%0.03f,' % data[0, layer, end, fe, dac]
                        
                    f.appendChild(self.__doc.createTextNode(s.rstrip(',')))


    def __writePED(self, data):

        shape = (9, 4, 8, 2, 12)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        a = self.__doc.createElement('adc_table')
        t = self.__doc.createElement('tem')
        t.setAttribute('num', '0')
        a.appendChild(t)
        self.__rootNode.appendChild(a)        

        for layer in range(8):
            for end in range(2):
            
                l = self.__doc.createElement('row')
                l.setAttribute('num', str(layer))
                l.setAttribute('end', str(end))
                l.setAttribute('name', '%s%s' % (CROW[layer], CPM[end]))
                t.appendChild(l)

                for fe in range(12):

                    f = self.__doc.createElement('fe')
                    f.setAttribute('num', str(fe))
                    l.appendChild(f)

                    for erng in range(4):

                        e = self.__doc.createElement('erng')
                        e.setAttribute('num', str(erng))
                        e.setAttribute('name', CRNG[erng])
                        f.appendChild(e)

                        s = ''                        
                        for gain in range(9):
                            s += '%0.03f,' % data[gain, erng, layer, end, fe]
                        
                        e.appendChild(self.__doc.createTextNode(s.rstrip(',')))
                                                        

    def __writeREL(self, data):

        shape = (9, 1, 4, 8, 2, 12)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        g = self.__doc.createElement('gain_table')
        t = self.__doc.createElement('tem')
        t.setAttribute('num', '0')
        g.appendChild(t)
        self.__rootNode.appendChild(g)        

        for layer in range(8):
            for end in range(2):
            
                l = self.__doc.createElement('row')
                l.setAttribute('num', str(layer))
                l.setAttribute('end', str(end))
                l.setAttribute('name', '%s%s' % (CROW[layer], CPM[end]))
                t.appendChild(l)

                for fe in range(12):

                    f = self.__doc.createElement('fe')
                    f.setAttribute('num', str(fe))
                    l.appendChild(f)

                    for erng in range(4):

                        e = self.__doc.createElement('erng')
                        e.setAttribute('num', str(erng))
                        e.setAttribute('name', CRNG[erng])
                        f.appendChild(e)

                        s = ''                        
                        for gain in range(9):
                            s += '%0.03f,' % data[gain, 0, erng, layer, end, fe]
                        
                        e.appendChild(self.__doc.createTextNode(s.rstrip(',')))


    def __writeULD(self, data):            

        # verify ADC data array shape

        shape = (3, 16, 8, 2, 12, 128)
        if data.shape != shape:
            raise calFileWriteExcept, "ADC data wrong shape: %s (expected %s)" % (data.shape, shape)

        # write FITS table header keys into XML text

        self.__writeHeader(data)

        # write ADC data into XML text

        a = self.__doc.createElement('adc_table')
        t = self.__doc.createElement('tem')
        t.setAttribute('num', '0')
        a.appendChild(t)
        self.__rootNode.appendChild(a)

        for layer in range(8):
            for end in range(2):
            
                l = self.__doc.createElement('row')
                l.setAttribute('num', str(layer))
                l.setAttribute('end', str(end))
                l.setAttribute('name', '%s%s' % (CROW[layer], CPM[end]))
                t.appendChild(l)

                for fe in range(12):

                    f = self.__doc.createElement('fe')
                    f.setAttribute('num', str(fe))
                    l.appendChild(f)
                    
                    for erng in range(3):

                        e = self.__doc.createElement('erng')
                        e.setAttribute('num', str(erng))
                        e.setAttribute('name', CRNG[erng])
                        f.appendChild(e)

                        s = ''
                        for dac in range(128):               
                            s += '%0.03f,' % data[erng, 0, layer, end, fe, dac]
                        
                        e.appendChild(self.__doc.createTextNode(s.rstrip(',')))
                    

                    
    def __writeHeader(self, data):

        ts = time.strftime('%y%m%d%H%M%S', time.gmtime())        

        # create document root node

        self.__rootNode = self.__doc.createElement('CALdoc')
        self.__rootNode.setAttribute('name', self.__fileName)
        self.__rootNode.setAttribute('type', 'FITS:%s' % self.__type)
        self.__rootNode.setAttribute('version', str(self.__xmlVersion))
        self.__rootNode.setAttribute('time', ts)
        self.__doc.appendChild(self.__rootNode)

        # create 'header_keys' element to hold FITS key string values

        hdrNode = self.__doc.createElement('header_keys')
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

        try:
          keys = self.__calSNs.keys()
          (temId,dummy) = keys[0].split(':')
          temId += ':'
        except:
          temId =""
        for n in range(self.__numSn):
            key = '%s%s' %(temId,CFACE[n])
            name = 'CALSN%s' %(CFACE[n])
            try:
                sn = self.__calSNs[key]
            except:
                continue        
            self.__writeKey(hdrNode, name, sn)

        n = 1
        for l in self.__labels[:-1]:
            name = 'LAXIS%d' % n
            n = n + 1
            self.__writeKey(hdrNode, name, l)


    def __writeKey(self, hdrNode, name, value):

        k = self.__doc.createElement('key')
        k.setAttribute('name', str(name))
        k.appendChild(self.__doc.createTextNode(str(value)))
        hdrNode.appendChild(k)


    def getVersion(self):
        """
        \brief Get the format version of a CAL FITS/XML file

        \returns The version number of the XML file, taken from the
                 <CALdoc> attribute.
        """

        # get XML root document element

        dList = self.__doc.getElementsByTagName('CALdoc')
        dLen = len(dList)
        if dLen != 1:
            raise calFileReadExcept, "wrong number of <CAL_doc> elements: %u (expected 1)" % dLen
        d = dList[0]

        # get XML format version

        xmlVersion = int(d.getAttribute('version'))
        return xmlVersion


    def read(self):
        """
        \brief Read data from a CAL FITS/XML file

        \returns A Numeric array of data read from the FITS data table.
        """

        # get XML document type

        dList = self.__doc.getElementsByTagName('CALdoc')
        dLen = len(dList)
        if dLen != 1:
            raise calFileReadExcept, "wrong number of <CAL_doc> elements: %u (expected 1)" % dLen
        d = dList[0]
        type = d.getAttribute('type')
        if len(type) == 0:
            raise calFileReadExcept, "<CALdoc> element requires type attribute"
        tList = type.split(':')
        if len(tList) != 2 or tList[0] != 'FITS' or tList[1] not in FILE_TYPES:
            raise calFileReadExcept, "<CALdoc> type attribute %s not recognized" % type 
        self.__type = tList[1]

        # get XML format version

        self.__xmlVersion = int(d.getAttribute('version'))
        err = "XML format version %d not supported for file type %s" % (self.__xmlVersion, self.__type)  
        if self.__xmlVersion > 1:
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

        data = Numeric.zeros((16, 8, 2, 12, 128), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'

        aList = self.__doc.getElementsByTagName('adc_table')
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <adc_table> elements: %u (expected 1)" % aNum
        for a in aList:
            tList = a.getElementsByTagName('tem')
            tNum = len(tList)
            if tNum == 0 or tNum > 16:
                raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1-16)" % tNum 
            for t in tList:
                tem = int(t.getAttribute('num'))
                if tem < 0 or tem > 15:
                  raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem
                lList = t.getElementsByTagName(elName)
                lNum = len(lList)
                if lNum != 16:
                    raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
                for l in lList:
                    layer = int(l.getAttribute('num'))
                    end = int(l.getAttribute('end'))
                    if layer < 0 or layer > 7:
                      raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                    if end < 0 or end > 1:
                      raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)
                    fList = l.getElementsByTagName('fe')
                    fNum = len(fList)
                    if fNum != 12:
                        raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                    for f in fList:
                        fe = int(f.getAttribute('num'))
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

        data = Numeric.zeros((9, 4, 8, 2, 12), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'        

        aList = self.__doc.getElementsByTagName('adc_table')
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <adc_table> elements: %u (expected 1)" % aNum
        for a in aList:
            tList = a.getElementsByTagName('tem')
            tNum = len(tList)
            if tNum != 1:
                raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1)" % tNum 
            for t in tList:
                tem = int(t.getAttribute('num'))
                if tem < 0 or tem > 15:
                  raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem
                lList = t.getElementsByTagName(elName)
                lNum = len(lList)
                if lNum != 16:
                    raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
                for l in lList:
                    layer = int(l.getAttribute('num'))
                    if layer < 0 or layer > 7:
                      raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                    end = int(l.getAttribute('end'))
                    if end < 0 or end > 1:
                      raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)
                    fList = l.getElementsByTagName('fe')
                    fNum = len(fList)
                    if fNum != 12:
                        raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                    for f in fList:
                        fe = int(f.getAttribute('num'))
                        if fe < 0 or fe > 11:
                          raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe
                        eList = f.getElementsByTagName('erng')
                        eLen = len(eList)
                        if eLen != 4:
                            raise calFileReadExcept, "wrong number of <erng> elments: %u (expected 4" % eNum
                        for e in eList:
                            erng = int(e.getAttribute('num'))
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

        data = Numeric.zeros((9, 4, 16, 8, 2, 12), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'        

        gList = self.__doc.getElementsByTagName('gain_table')
        gNum = len(gList)
        if gNum != 1:
            raise calFileReadExcept, "wrong number of <gain_table> elements: %u (expected 1)" % gNum
        for g in gList:
            tList = g.getElementsByTagName('tem')
            tNum = len(tList)
            if tNum != 1:
                raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1)" % tNum 
            for t in tList:
                tem = int(t.getAttribute('num'))
                if tem < 0 or tem > 15:
                  raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem
                lList = t.getElementsByTagName(elName)
                lNum = len(lList)
                if lNum != 16:
                    raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
                for l in lList:
                    layer = int(l.getAttribute('num'))
                    if layer < 0 or layer > 7:
                      raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                    end = int(l.getAttribute('end'))
                    if end < 0 or end > 1:
                      raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)
                    fList = l.getElementsByTagName('fe')
                    fNum = len(fList)
                    if fNum != 12:
                        raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                    for f in fList:
                        fe = int(f.getAttribute('num'))
                        if fe < 0 or fe > 11:
                          raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe
                        eList = f.getElementsByTagName('erng')
                        eLen = len(eList)
                        if eLen != 4:
                            raise calFileReadExcept, "wrong number of <erng> elments: %u (expected 4" % eNum
                        for e in eList:
                            erng = int(e.getAttribute('num'))
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

        data = Numeric.zeros((3, 16, 8, 2, 12, 128), Numeric.Float32)

        if self.__xmlVersion >= 1:
            elName = 'row'
        else:
            elName = 'layer'        

        aList = self.__doc.getElementsByTagName('adc_table')
        aNum = len(aList)
        if aNum != 1:
            raise calFileReadExcept, "wrong number of <adc_table> elements: %u (expected 1)" % aNum
        for a in aList:
            tList = a.getElementsByTagName('tem')
            tNum = len(tList)
            if tNum == 0 or tNum > 16:
                raise calFileReadExcept, "wrong number of <tem> elements: %u (expected 1-16)" % tNum 
            for t in tList:
                tem = int(t.getAttribute('num'))
                if tem < 0 or tem > 15:
                  raise calFileReadExcept, "read <tem> element attribute \'num\' value %d (expected 0-15)" % tem
                lList = t.getElementsByTagName(elName)
                lNum = len(lList)
                if lNum != 16:
                    raise calFileReadExcept, "wrong number of <%s> elements: %u (expected 16)" % (elName, lNum)
                for l in lList:
                    layer = int(l.getAttribute('num'))
                    if layer < 0 or layer > 7:
                      raise calFileReadExcept, "read <%s> element attribute \'num\' value %d (expected 0-7)" % (elName, layer)
                    end = int(l.getAttribute('end'))
                    if end < 0 or end > 1:
                      raise calFileReadExcept, "read <%s> element attribute \'end\' value %d (expected 0-1)" % (elName, end)
                    fList = l.getElementsByTagName('fe')
                    fNum = len(fList)
                    if fNum != 12:
                        raise calFileReadExcept, "wrong number of <fe> elements: %u (expected 11)" % fNum
                    for f in fList:
                        fe = int(f.getAttribute('num'))
                        if fe < 0 or fe > 11:
                          raise calFileReadExcept, "read <fe> element attribute \'num\' value %d (expected 0-11)" % fe
                        eList = f.getElementsByTagName('erng')
                        eLen = len(eList)
                        if eLen != 3:
                            raise calFileReadExcept, "wrong number of <erng> elments: %u (expected 3)" % eNum
                        for e in eList:
                            erng = int(e.getAttribute('num'))
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
        \brief Get a CAL FITS/XML file table header values
        
        The following key values are recognized in the FITS file table header: \n

          - \e TTYPE1 - The file type ('fle_dac', 'fhe_dac', 'log acpt', 'pedestal values', ... \n
          - \e TFORM1 - The FITS file format \n
          - \e EXTNAME - The FITS table name -  'GLAST CAL Ancillary Data' \n
          - \e SHAPE - The Python shape tuple for the FITS data \n
          - \e TYPECODE - The Python typecode for the FITS data \n
          - \e FITSNAME - The orignal name of the FITS file \n
          - \e FITSTIME - The time the FITS file was created \n
          - \e LAXIS1 - LAXIS7 - The FITS table dimension labels. \n
          - \e CALSNX+,CALSNX-,CALSNY+,CALSNY- - The CAL AFEE board serial numbers \n
          - \e DATASET - The name of the event data file associated with the FITS data \n
          - \e LREFGAIN - The low reference gain used during the collection of the FITS data. \n
          - \e HREFGAIN - The high reference gain used during the collection of the FITS data. \n
          - \e PEDFILE - The name of the pedestal FITS file associated with the FITS data. \n
          - \e ERNG - The energy range used during the collection of the FITS data. \n
          - \e RPTNAME - The name of the associated test report file. \n
          - \e RUNID - The run collection ID for the data. \n
          - \e COMMENT - A general comment and notes string for the FITS file. \n
          
        \returns A dictionary of <table header key>:<value> pairs.
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
        
        for n in range(self.__numAxes):
            name = 'LAXIS%d' % (n + 1)
            i[name] = None
        for n in range(self.__numSn):
            name = 'CALSN%s' %CFACE[n]
            i[name] = None

        ikeys = i.keys()            

        hList = self.__doc.getElementsByTagName('header_keys')
        hNum = len(hList)
        if hNum != 1:
            raise calFileReadExcept, "wrong number of <header_keys> elements: %u (expected 1)" % hNum            

        kList = hList[0].getElementsByTagName('key')
        for k in kList:
            d = k.childNodes[0]
            name = k.getAttribute('name')
            s = d.data.strip()
            if name in ikeys:
                i[name] = str(s)
            else:
                self.__log.warning("calFitsXML: unknown <key> element name: %s", name)
        
        return i    

                    
    def close(self):
        """
        \brief Close a CAL FITS/XML file.
        """
        
        self.__xmlFile.close()
        try:
          self.__doc.unlink()
        except:
          self.__log.warning("calFitsXML: XML document unlink failed")
        self.__doc = None

        