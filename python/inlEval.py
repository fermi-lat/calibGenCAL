"""
Evaluate IntNonlin (CIDAC2ADC spline @ single point)

Usage:
inlEval d2a|a2d intNonlin.xml tower layer column face adc_range input_val

where:
   d2a|a2d - choose dac2adc() or adc2dac() curve
   intNonlin.xml - input offline intNonlin calibration file
   tower, layer, column, face, adc_range = offline Cal ADC channel indexing
   input_val = input value to spline function

output:
   stdout - selected spline function evaluated @ specified point.


"""

__facility__  = "Offline"
__abstract__  = "Evaluate IntNonlin (CIDAC2ADC spline @ single point)"
__author__    = "Z. Fewtrell"
__date__      = "$Date: 2008/06/03 01:41:14 $"
__version__   = "$Revision: 1.1 $, $Author: fewtrell $"
__release__   = "$Name:  $"
__credits__   = "NRL code 7650"

if __name__ == '__main__':

    # setup logger
    import logging
    logging.basicConfig()
    log = logging.getLogger('inlEval')
    log.setLevel(logging.INFO)

    # parse commandline
    #  - code stolen from: http://python.active-venture.com/lib/module-getopt.html
    import sys
    import getopt
    try:
        opts, args = getopt.getopt(sys.argv[1:], "")
    except getopt.GetoptError:
        log.exception("getopt exception: "+__doc__)
        sys.exit(-1)

    (dir, inlPath, twr, lyr, col, face, rng, inVal) = args

    if not dir in ("d2a","a2d"):
        log.error("Invalid f() name: '" + dir + "' should be 'd2a' or 'a2d'")
        sys.exit(1)

    # open and read XML intNonlin file
    import calCalibXML
    xmlFile = calCalibXML.calIntNonlinCalibXML(inlPath)
    inlData = xmlFile.read()
    (lenData, dacData, adcData) = inlData
    towers = xmlFile.getTowers()
    xmlFile.close()

    # convert indeces
    [twr, lyr, col, face, rng] = [int(x) for x in  twr, lyr, col, face, rng]
    inVal = float(inVal)
    
    row = calCalibXML.layerToRow(lyr)
    import calConstant
    online_face = calConstant.offline_face_to_online[face]

    # build spline for this channel
    length = int(lenData[rng][twr,row,online_face,col])
    # skip empty channels: HACK WARNING!
    # unfortunately i fear that calCalib .dtd requires that all channels have
    # some entry, so sometimes I have put in empty channels w/ single point
    # 0,0  This seems to break the TSpline objects in this script
    # so I skip any channel w/ either 0 _or_ 1 entry in it.
    if length <= 1:
        log.error("Empty intNonlin channel")
        sys.exit(-1)
    
    import array
    dacArray = array.array('d', dacData[rng][twr,row,online_face,col,0:length])
    adcArray = array.array('d', adcData[rng][twr,row,online_face,col,0:length])

    import ROOT
    if dir == "d2a":
        spline = ROOT.TSpline3("inlSpline",
                               dacArray,
                               adcArray,
                               length)
    else:
        spline = ROOT.TSpline3("inlSpline",
                               adcArray,
                               dacArray,
                               length)
        
    outVal = spline.Eval(inVal)

    print outVal

    sys.exit(0)                            
