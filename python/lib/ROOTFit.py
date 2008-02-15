"""
simple fitting front end for ROOT fitting code
"""


__facility__      = "Online"
__abstract__      = __doc__
__author__        = "D.L. Wood"
__date__          = "$Date: 2007/12/20 00:36:51 $"
__version__       = "$Revision: 1.8 $, $Author: fewtrell $"
__release__       = "$Name:  $"
__credits__       = "NRL code 7650"


def ROOTFit(func,
            xpts,
            ypts,
            parms):
    """
    Fit func to data specified by xpts and ypts.

    ARGS:
    fname - ROOT TF1 object
    xpts - x data sequence
    ypts - y data sequence 
    parms - sequence of initial function parameters

    RETURN:
    a tuple (parms, errors, chisq) where:
    parms - list of fitted parameter values, same length as parms input sequence
    errors - list of error values on each parameter, same length as parms
    chisq - chi-squared value from fit result.
    """

    import ROOT
    import array
    g = ROOT.TGraph(len(xpts),
                    array.array('d',xpts),
                    array.array('d',ypts))

    # set initial function parameter values
    func.SetParameters(array.array('d',parms))

    g.Fit(func,"Q")

    nParms = func.GetNpar()
    fittedParms = []
    errors = []
    for idx in range(nParms):
        fittedParms.append(func.GetParameter(idx))
        errors.append(func.GetParError(idx))
    chisq = func.GetChisquare()

    return (fittedParms, errors, chisq)

if __name__ == '__main__':
    """
    Unit test for ROOTFit pkg.
    """

    # test f() is 3x^2 + 2
    xpts = [-1,0,1,2]
    ypts = [5,2,5,14]

    import ROOT
    func = ROOT.TF1("p2","pol2")
    parms = [1,1]

    (fitParms, errors, chisq) = ROOTFit(func, xpts, ypts, parms)

    # check fitted parms
    intendedParms = [2,0,3]

    import sys
    for (intent, fit) in zip(intendedParms, fitParms):
        if abs(intent - fit) > .0001:
            print "Bad fitted parm %f, epxected %f"%(fit, intent)
            sys.exit(-1)

    print "unit test successful"
    sys.exit(0)


    
        
