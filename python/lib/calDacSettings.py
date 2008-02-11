"""
Algorithms for generating DAC settings values from CAL_DacSlope calibration
information.
"""


__facility__  = "Offline"
__abstract__  = "Algorithms for generating DAC settings"
__author__    = "D.L.Wood"
__date__      = "$Date: 2007/03/15 17:37:25 $"
__version__   = "$Revision: 1.1 $, $Author: dwood $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"


import numarray

import calConstant


def dacSettings(slope, offset, energy):
    """
    Generate settings for LAC, FLE, or FHE DAC's.
    Param: slope - An array of shape (16, 8, 2, 12) giving DAC linear fit
                   parameter c1 in Mev/DAC.
    Param: offset - An array of shape (16, 8, 2, 12) giving DAC linear fit
                    parameter c0 in Mev.
    Param: energy - The threshold energy in MeV.
    Returns: An array of shape (16, 8, 2, 12) giving the DAC settings values.
    """

    d = (energy - offset) / slope
    d = numarray.clip(d, 0, 63)
    d = numarray.around(d) 
    return d.astype(numarray.Int8)
     
       
     
def uldSettings(slopes, offsets, sats, margin):
    """
    Generate settings for ULD DAC's.
    Param: slope - An array of shape (3, 16, 8, 2, 12) giving ULD linear fit
                   parameter c1 in Mev/DAC.
    Param: offset - An array of shape (3, 16, 8, 2, 12) giving ULD linear fit
                    parameter c0 in Mev.
    Param: sats - An array of shape (3, 16, 8, 2, 12) giving ULD saturation
                  value in MeV.
    Param: margin - The ULD threshold limit as fraction of the saturation
                    value.
    Param: energy - The threshold energy in MeV.
    Returns: An array of shape (16, 8, 2, 12) giving the ULD settings values.
    """
    
    sats = sats - (sats * margin)
    d = (sats - offsets) / slopes
    d = numarray.clip(d, 0, 63)
    d = numarray.around(d)
    lex8 = d[calConstant.CRNG_LEX8, ...]
    lex1 = d[calConstant.CRNG_LEX1, ...]  
    hex8 = d[calConstant.CRNG_HEX8, ...]
    s = numarray.where((lex8 < lex1), lex8, lex1)
    s = numarray.where((s < hex8), s, hex8)
    return s.astype(numarray.Int8) 
              
     
     
def setRange(dacs, rangeData):
    """
    Adjust DAC settings to account for DAC FINE/COARSE range.
    Param: dacs - An array of shape ([3,] 16, 8, 2, 12) giving the raw 
                  DAC settings values.
    Param: rangeData - A boolean array of shape ([3,] 16, 8, 2, 12) giving
                       the desired range for each DAC (0=FINE, 1=COARSE).
    Returns: An array of shape ([3,] 16, 8, 2, 12) giving the DAC settings 
             with the range bit (MSB) set if appropriate.
    """
    
    return numarray.where(rangeData, (dacs + 64), dacs)
    
    


    
                        

    
