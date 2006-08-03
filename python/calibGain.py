"""
calibGain file contain one coefficient per crystal face. representing
"""


"""
read in txt file from calibGainCoef.py
return tuple of (calibGainRatio, twrSet) where:
 - calibGainRatio = calibGainCoef ratio in numeric multidimensional array.
   array shape is [16,8,2,12] for online twr,lyr,col,face
 - twrSet is python set of active LAT towers.

"""

import Numeric

import calConstant
import zachUtil
import calCalibXML

__facility__  = "Offline"
__abstract__  = "apply calibGain correction to asymmetry xml file"
__author__    = "Z.Fewtrell"
__date__      = "$Date: 2006/06/29 16:02:41 $"
__version__   = "$Revision: 1.2 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

def read_calibGain_txt(filename):
    # constants
    nTXTFields = 5
    
    # intialize output array
    outData = Numeric.zeros((calConstant.NUM_TEM,
                             calConstant.NUM_ROW,
                             calConstant.NUM_END,
                             calConstant.NUM_FE),
                            Numeric.Float32)
    twrSet = set()

    # read input file
    inFile = open(filename, 'r')

    lines = inFile.readlines()

    nLine = -1
    for line in lines:
        nLine+=1
        vals = line.split()
        if (len(vals) != nTXTFields):
            log.error("input line# %d expecting %d column input, got %d" % (nLine, nTXTFields, len(vals)) +
                      "fmt=[twr lyr col face ratio] " + line)
            sys.exit(-1)

        # convert vals array to floats instead of strings
        for i in range(len(vals)):
            vals[i] = float(vals[i])

        (twr, lyr, col, face, ratio) = vals
        
        # convert array index values to integer.
        twr  = int(twr)
        lyr  = int(lyr)
        col  = int(col)
        face = int(face)
        ratio = float(ratio)

        # convert offline face numbering to online face numbering
        face = zachUtil.offline_face_to_online[face]
        # convert offline lyr to row
        row = calCalibXML.layerToRow(lyr)

        # make sure current tower is on list
        twrSet.add(twr)

        outData[twr, row, face, col] = ratio

    return (outData, twrSet)

