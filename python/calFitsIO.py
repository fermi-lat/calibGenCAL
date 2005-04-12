"""
CAL FITS ancillary data classes
"""


__facility__ = "Offline"
__abstract__ = "CAL FITS ancillary data writer class"
__author__   = "B. Leas"
__date__     = "$Date: 2005/03/18 14:56:03 $"
__version__  = "$Revision: 1.21 $, $Author: cm $"
__release__  = "$Name:  $"
__credits__  = "SLAC"


import os.path
from cfitsio import *
import time
import struct
import types
import sys
import logging
from Numeric import size,fromstring
from calExcept import *
from calConstant import CFACE


CFITSFACE = {'X+':'XP','Y+':'YP','X-':'XM','Y-':'YM'}  #Standard FITs does not support + or - in KEYWORDS so convert to 'P' and 'M'


MODE_CREATE = 0
MODE_READONLY = 2



##########################################################################################
##
## \class calFitsIO
##
## \brief CAL FITS ancillary data file class.
##
## This class provides methods for accessing CAL ancillary data files as stored in FITS
## format.
##
##########################################################################################

class calFitsIO(object):
  
  def __init__(self, filePath=None, fileName=None, mode=MODE_CREATE, compress=False , labels = None, \
               labelComments = None, calSNs = None, dataset = None, lrefgain=None, hrefgain=None, \
               pedFile = None, erng=None, reportName = None, runId = None, comment = None):
    """
    \brief Open a CAL FITS file

    \param filePath The FITS file directory path.
    \param fileName The FITS file name.
    \param mode The file access mode (MODE_READONLY or MODE_CREATE).
    \param compress Assume FITS file is gzip compressed.
    \param labels A sequence of FITS table header values: \n
        labels[0] = Table header \e LAXIS1 string value
        labels[1] = Table header \e LAXIS2 string value
          ...
        labels[-2] = Table header \e LAXIS<n> string value.
        labels[-1] = Table header \e TTYPE1 string value.
    \param lableComments Descriptions for the \e labels sequence values.
    \param calSNs A sequence of up to table header \e CALSN<n> string values.
    \param dataset The table header \e DATASET string value.
    \param lrefgain The table header \e LREFGAIN integer value.
    \param hrefgain The table header \e HREFGAIN integer value.
    \param pedFile The table header \e PEDFILE string value.
    \param erng The table header \e ERNG string value.
    \param reportName The name of the associated test report file.
    \param runId The run collection ID for the data.
    \param comment The table header \e COMMENT string value.
    """
    
    self.__numAxes = 7
    self.__numSn = 4

    self.__log = logging.getLogger()

    self.__ts   = time.strftime('%y%m%d%H%M%S', time.gmtime())    

    if fileName is None:
      self.__fileName = getFileNameFromTS(self.__ts)
    else:
      self.__fileName = fileName
      
    if compress:
      self.__fileName += '.gz'
      
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
    self.__reportName = reportName
    self.__runId = runId
    self.__comment = comment

    path = os.path.join(self.__filePath, self.__fileName)    

    # open existing file in read-only mode

    if mode == MODE_READONLY:
      (self.__status, self.__fptr) = cfitsio.fits_open_file(path, cfitsio.READONLY)
      if self.__status != 0:
        raise calFileOpenExcept, "fits_open_file: %s: %s" % (path, cfitsio.fits_get_errstatus(self.__status))
      else:
        self.__tablenum = 2
        status = self.__moveToHDU(self.__tablenum)
        self.__rownum = 0

    # create a new file
    # will fail if file already exists
        
    elif mode == MODE_CREATE:
      (self.__status, self.__fptr) = cfitsio.fits_create_file(path)
      if self.__status != 0:
        raise calFileOpenExcept, "fits_create_file: %s: %s" % (path, cfitsio.fits_get_errstatus(self.__status))
      else:
        naxis = 0
        naxes = []
        self.__status = cfitsio.fits_create_img(self.__fptr, cfitsio.SHORT_IMG, naxis, naxes)
        if self.__status != 0:
          raise calFileOpenExcept, "fits_create_img: %s: %s" % (path, cfitsio.fits_get_errstatus(self.__status))

    # file access mode not supported    
     
    else:
      raise calFileOpenExcept, "Invalid FITS file mode"


  def __createNewTable(self,sizeInBytes):
    
    try:
      ttype = [self.__labels[-1]]
    except:
      ttype = [' ']
    tform = ['1PB(%d)' % sizeInBytes]
    tunit = [' ']
    extname='GLAST CAL Ancillary Data'
    tfields=1
    self.__status = cfitsio.fits_create_tbl(self.__fptr, cfitsio.BINARY_TBL,
                        1, tfields, ttype, tform, tunit, extname)
    if self.__status != 0:
      raise calFileWriteExcept, "fits_create_tbl: %s" % cfitsio.fits_get_errstatus(self.__status)


  def write(self, rawEventData):
    """
    \brief Write data to a CAL FITS file

    \param rawEventData A Numeric array of data to be written in the FITS table.
    """
    
    sizeInBytes = size(rawEventData)*rawEventData.itemsize()
    self.__createNewTable(sizeInBytes)
    self.__status = cfitsio.fits_write_col_byt(self.__fptr, 1, 1,
                                                 1, sizeInBytes, rawEventData.tostring())
    if self.__status != 0:
      raise calFileWriteExcept, "fits_write_col_byt: %s" % cfitsio.fits_get_errstatus(self.__status)

    self.__status = cfitsio.fits_flush_buffer(self.__fptr, 0)
    if self.__status != 0:
      raise calFileWriteExcept, "fits_flush_buffer: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "SHAPE",
                                                    `rawEventData.shape`,
                                                    "CAL table shape (Python Numeric syntax)")
    if self.__status != 0:
      raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "TYPECode",
                                                    rawEventData.typecode(),
                                                    "CAL table item typecode (Python Numeric syntax)")
    if self.__status != 0:
      raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)

    self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "FITSNAME",
                                                    self.__fileName,
                                                    "FITS file name")
    if self.__status != 0:
      raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)

    self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "FITSTIME",
                                                    self.__ts,
                                                    "FITS file creation time")
    if self.__status != 0:
      raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    for i in range(len(self.__labels)-1):
      try:
        comment = "Dimension %d Label " %i +labelComments[i]
      except:
        comment = "Dimension %d Label" %i
      self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "LAXIS%d" %(i+1),
                                                    self.__labels[i],
                                                    comment)
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)    

    try:
      keys = self.__calSNs.keys()
      (temId,dummy) = keys[0].split(':')
      temId += ':'
    except:
      temId =""
    for i in CFACE:      
      try:
        self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "CALSN%s" %(CFITSFACE[i]),
                                                    self.__calSNs['%s%s' %(temId,i)],
                                                    "Calorimeter %s%s FACE Serial Number" %(temId,i))
      except:
        self.__log.warning('calFitsIO.write(): key CALSN%s (CALSN%s) not found', i, CFITSFACE[i])

      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "DATASET",
                                                    self.__dataset,
                                                    "Parent Dataset for this table ")
    if self.__status != 0:
      raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    if self.__lrefgain is not None:
      self.__status = cfitsio.fits_write_key_lng(self.__fptr, 
                                                    "LREFGAIN",
                                                    self.__lrefgain,
                                                    "Low Energy Reference Gain for this Table")
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    if self.__hrefgain is not None:
      self.__status = cfitsio.fits_write_key_lng(self.__fptr, 
                                                    "HREFGAIN",
                                                    self.__hrefgain,
                                                    "High Energy Reference Gain for this Table")
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)

    if self.__pedFile is not None:      
      self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "pedFile",
                                                    self.__pedFile,
                                                    "CAL Pedestals Table used for this Table")

    if self.__erng is not None:      
      self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "erng",
                                                    self.__erng,
                                                    "Energy Range")
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)    


    if self.__reportName is not None:
      self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "RPTNAME",
                                                    self.__reportName,
                                                    "Associated Report File Name")
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)

    if self.__runId is not None:
      self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "RUNID",
                                                    self.__runId,
                                                    "Run collection ID")
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)

    if self.__comment is not None:
      self.__status = cfitsio.fits_write_key_str(self.__fptr, 
                                                    "COMMENT",
                                                    self.__comment,
                                                    "File comments and notes")
      if self.__status != 0:
        raise calFileWriteExcept, "fits_write_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)      


  def __moveToHDU(self, tablenum):
    
    (self.__status, hdutype) = cfitsio.fits_movabs_hdu(self.__fptr, tablenum)
    if self.__status != 0:
      return self.__status
    return self.__status


  def read(self):
    """
    \brief Read data from a CAL FITS file

    \returns rawEventData A Numeric array of data to read from the FITS table.
    """
    
    (self.__status, event_size, heap_addr) = cfitsio.fits_read_descript(self.__fptr, 1, 1)
    if self.__status != 0:
      raise calFileReadExcept, "fits_read_descript: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    (self.__status, shape, comment) = cfitsio.fits_read_key_str(self.__fptr, "SHAPE")      
    if self.__status != 0:
      raise calFileReadExcept, "fits_read_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    (self.__status, typecode, comment) = cfitsio.fits_read_key_str(self.__fptr, "TYPECODE")      
    if self.__status != 0:
      raise calFileReadExcept, "fits_read_key_str: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    (self.__status, dat) = cfitsio.fits_read_col_byt(self.__fptr, 1, 1, 1, event_size, None)
    if self.__status != 0:
      raise calFileReadExcept, "fits_read_col_byt: %s" % cfitsio.fits_get_errstatus(self.__status)
    
    data =fromstring(dat,typecode)
    data.shape = eval(shape)
    return data


  def info(self):
    """
    \brief Get a CAL FITS file table header values

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
      - \e RUNID - The run collection ID for the data \n
      - \e COMMENT - A general comment and notes string for the FITS file. \n
      
    \returns A dictionary of <table header key>:<value> pairs.
    """
    
    i = {}
    laxis = []
    sn = []
    
    (self.__status, shape, comment) = cfitsio.fits_read_key_str(self.__fptr, "SHAPE")      
    if self.__status != 0:
      shape = None
    
    (self.__status, typecode, comment) = cfitsio.fits_read_key_str(self.__fptr, "TYPECODE")      
    if self.__status != 0:
      typecode = None

    (self.__status, fitsname, comment) = cfitsio.fits_read_key_str(self.__fptr, "FITSNAME")      
    if self.__status != 0:
      fitsname = None

    (self.__status, fitstime, comment) = cfitsio.fits_read_key_str(self.__fptr, "FITSTIME")      
    if self.__status != 0:
      fitstime = None       

    for n in range(self.__numAxes):    
        (self.__status, la, comment) = cfitsio.fits_read_key_str(self.__fptr, "LAXIS%d" % (n + 1))
        if self.__status != 0:
          la = None
        laxis.append(la)    

    for n in range(self.__numSn):     
        (self.__status, s, comment) = cfitsio.fits_read_key_str(self.__fptr, "CALSN%d" % (n + 1))
        if self.__status != 0:
            s = None
        sn.append(s)    

    for n in range(self.__numSn):     
        (self.__status, s, comment) = cfitsio.fits_read_key_str(self.__fptr, "CALSN%s" %(CFITSFACE[CFACE[n]]))
        if self.__status != 0:
            s = None
        if sn[n] is None:
          sn[n] = s   

    (self.__status, dataset, comment) = cfitsio.fits_read_key_str(self.__fptr, "DATASET")
    if self.__status != 0:
      dataset = None
    
    (self.__status, lrefgain, comment) = cfitsio.fits_read_key_lng(self.__fptr, "LREFGAIN")
    if self.__status != 0:
        lrefgain = None
    
    (self.__status, hrefgain, comment) = cfitsio.fits_read_key_lng(self.__fptr, "HREFGAIN")
    if self.__status != 0:
        hrefgain = None
    
    (self.__status, pedfile, comment) = cfitsio.fits_read_key_str(self.__fptr, "PEDFILE")
    if self.__status != 0:
        pedfile = None

    (self.__status, ttype1, comment) = cfitsio.fits_read_key_str(self.__fptr, "TTYPE1")
    if self.__status != 0:
        ttype1 = None

    (self.__status, tform1, comment) = cfitsio.fits_read_key_str(self.__fptr, "TFORM1")
    if self.__status != 0:
        tform1 = None

    (self.__status, extname, comment) = cfitsio.fits_read_key_str(self.__fptr, "EXTNAME")
    if self.__status != 0:
        extname = None

    (self.__status, erng, comment) = cfitsio.fits_read_key_str(self.__fptr, "ERNG")
    if self.__status != 0:
        erng = None

    (self.__status, reportName, comment) = cfitsio.fits_read_key_str(self.__fptr, "RPTNAME")
    if self.__status != 0:
      reportName = None

    (self.__status, runId, comment) = cfitsio.fits_read_key_str(self.__fptr, "RUNID")
    if self.__status != 0:
      runId = None

    (self.__status, commentVal, comment) = cfitsio.fits_read_key_str(self.__fptr, "COMMENT")
    if self.__status != 0:
      commentVal = None      
        
    i['SHAPE'] = shape
    i['TYPECODE'] = typecode
    i['FITSNAME'] = fitsname
    i['FITSTIME'] = fitstime
    i['DATASET'] = dataset
    i['LREFGAIN'] = lrefgain
    i['HREFGAIN'] = hrefgain
    i['PEDFILE'] = pedfile
    i['TTYPE1'] = ttype1
    i['TFORM1'] = tform1
    i['EXTNAME'] = extname
    i['ERNG'] = erng
    i['RPTNAME'] = reportName
    i['RUNID'] = runId
    i['COMMENT'] = commentVal
    
    for n in range(self.__numAxes):
        name = 'LAXIS%d' % (n + 1)
        i[name] = laxis[n]
        
    for n in range(self.__numSn):
        name = 'CALSN%s' %CFACE[n]
        i[name] = sn[n]
    
    return i


  def close(self, inShutdown=0):
    """
    \brief Close a CAL FITS file.
    """
    
    self.__status = cfitsio.fits_close_file(self.__fptr)
    if self.__status != 0:
      raise calFileCloseExcept, "fits_close_file: %s" % cfitsio.fits_get_errstatus(self.__status)
    self.__fptr = None
      

  def getStatus(self):
    """
    \brief Get the CAL FITS current status

    \returns The status string of the last file operation attempted.
    """
      
    return self.__status


  def getFileName(self):
      """
      \brief Get the CAL FITS file name

      \returns The name of the FITS file.
      """
      
      return self.__fileName

      
  def getFilePath(self):
    """
    \brief Get the CAL FITS file directory

    \returns The path name of the FITS file directory.
    """
    
    return self.__filePath

    
  def getHeaders(self):
    pass

        
  def processTable(self):
    pass
           
  
  def getFileNameFromTS(ts):
    return "ebf" + ts + ".fits"
      
