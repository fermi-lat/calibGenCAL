"""
Generate pedestal settings for use by flight software.  The output is a file named cal_pedestals.h.
This file will be put in the FSW CxP_DB package for incorporation into the on-board loads.
The command line is:

genFlightPed [-v] [-k <key>] <ped_xml_file>

where:
    -v              = verbose; turn on debug output
    -o              = set ORIGIN macro value to 'ON_ORBIT'; default is 'PRELAUNCH'
    -k <key>        = value for KEY macro; default is '0'  
    <ped_xml_file>  = The CAL_Ped calibration XML file for obtaining values.
"""


__facility__    = "Offline"
__abstract__    = "Generate pedestal settings for use by flight software"
__author__      = "D.L.Wood"
__date__        = "$Date: 2006/06/20 19:15:34 $"
__version__     = "$Revision: 1.4 $, $Author: dwood $"
__release__     = "$Name:  $"
__credits__     = "NRL code 7650"



import sys
import time
import getopt
import logging

import calCalibXML
import calConstant



END_MAP = ('N', 'P')


if __name__ == '__main__':
    

    # setup logger

    logging.basicConfig()
    log = logging.getLogger('genFlightPed')
    log.setLevel(logging.INFO)

    key = 0
    origin = 'PRELAUNCH'
    
    # check command line

    try:
        opts = getopt.getopt(sys.argv[1:], "-v-o-k:")
    except getopt.GetoptError:
        log.error(__doc__)
        sys.exit(1)

    optList = opts[0]
    for o in optList:
        if o[0] == '-v':
            log.setLevel(logging.DEBUG)
        elif o[0] == '-o':
            origin = 'ON_ORBIT'    
        elif o[0] == '-k':
            key = int(o[1])    
        
    args = opts[1]
    if len(args) != 1:
        log.error(__doc__)
        sys.exit(1)

    inName = args[0]
 
    # read pedestal values file
    
    log.info("Reading pedestals file %s", inName)
    fio = calCalibXML.calPedCalibXML(inName)
    peds = fio.read()
    fio.close()
      
    # create output file

    outName = 'cal_pedestals.h'
    log.info('Writing output file %s', outName)
    fio = open(outName, 'w')
    
    ts = time.strftime('%y-%m-%d %H:%M:%S')
    
    rel = __release__[7:-2].strip()
    if len(rel) == 0:
        ver = 'dev'
    else:
        ver = rel      
    
    fio.write('// %s\n' % outName)
    fio.write('// On-board calorimeter pedestal values\n')
    fio.write('// Generated by genFlightPed, calibGenCAL version %s, on %s\n' % (ver, ts)) 
    fio.write('// Input calibration file: %s\n\n' % inName) 
    fio.write('// CAL_Ped input file is documented in LAT Calibration Metadata Database hosted at SLAC.\n')
    fio.write('// The KEY macro value contained in this file references the metadata row entry.\n')
    fio.write('// Database:   calib\n')
    fio.write('// Schema:     metadata_v2r1\n')
    fio.write('// ser_no:     %d\n' % key) 
    fio.write('\n\n')
    
    fio.write('// $Id: genFlightPed.py,v 1.4 2006/06/20 19:15:34 dwood Exp $\n\n')
    
    # create preamble macros
    
    fio.write('#define SUBSYSTEM    CAL\n')
    fio.write('#define TYPE         PEDESTALS\n')
    fio.write('#define STYLE        FULL\n')
    fio.write('#define ORIGIN       %s\n' % origin)
    fio.write('#define KEY          0x%08x\n\n' % key)
    
    # create pedestal value macros
    
    for tem in range(calConstant.NUM_TEM):
        for row in range(calConstant.NUM_ROW):
            for fe in range(calConstant.NUM_FE):
                for end in range(calConstant.NUM_END):
                    for erng in range(calConstant.NUM_RNG):
                        
                        p = peds[tem, row, end, fe, erng, 0]
                        s = '#define PED_T%1X_%s_C%1X_R%d_%s    %f\n' % (tem, calConstant.CROW[row], fe,
                            erng, END_MAP[end], p)
                        fio.write(s)
    
    fio.close()

    sys.exit(0)
    
    

   
