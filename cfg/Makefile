# $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/cfg/Makefile,v 1.10 2007/04/13 01:43:08 fewtrell Exp $
# @file calibGenCAL analysis Makefile - manage generation of all calibGenCAL outputs and accessory files
# @requires GNU Make
# @requires calibGenCAL package CMT environment setup - this means source your $CALIBGENCALROOT/cmt/setup.(csh|sh|bat)


##################
## INPUT FILES ###
##################

## SECTION A: PEDESTALS ##

### A1: MUONGAIN PEDESTALS
MUONGAIN_PED_DIGI_FILES = muon_digi_files.txt
# options 'PERIODIC' 'EXTERNAL' 'ALL'
MUONGAIN_PED_EVENT_FILTER     = PERIODIC
# 1000 events minimum per hist is sufficient
MUONGAIN_PED_NENTRIES_PERHIST = 1000

### A2: FLIGHTGAIN PEDESTALS
FLIGHTGAIN_PED_DIGI_FILES = flightped_digi_files.txt
# options 'PERIODIC' 'EXTERNAL' 'ALL'
FLIGHTGAIN_PED_EVENT_FILTER     = PERIODIC
# 1000 events minimum per hist is sufficient
FLIGHTGAIN_PED_NENTRIES_PERHIST = 1000


## SECTION B: INT-NONLIN (CIDAC2ADC) ##
# set to '-c' if inl data is in column mode, blank if not
#INL_COLMODE    = -c

### B1: MUONGAIN INT-NONLIN
# input LE singlex16 digi root event file (muongain, calibGain=on, calibGen element#201)
MUONGAIN_INL_DIGI_LE = /nfs/farm/g/glast/u41/Integration/rootData/077014468/v7r0913p10/digi/digitization-licos-v3r9p2_077014468_digi_DIGI.root
# input HE singlex16 digi root event file (muongain), calibGain=off, calibGen element#204)
MUONGAIN_INL_DIGI_HE = /nfs/farm/g/glast/u41/Integration/rootData/077014471/v7r0913p10/digi/digitization-licos-v3r9p2_077014471_digi_DIGI.root

### B2: FLIGHTGAIN INT-NONLIN
# input LE singlex16 digi root event file (flightgain, calibGain=on, fle=nominal, calibGen element#101)
FLIGHTGAIN_INL_DIGI_LE = /nfs/farm/g/glast/u41/Integration/rootData/077014460/v7r0913p10/digi/digitization-licos-v3r9p2_077014460_digi_DIGI.root
# input HE singlex16 digi root event file (flightgain, calibGain=on, fle=nominla, calibGen element#102)
FLIGHTGAIN_INL_DIGI_HE = /nfs/farm/g/glast/u41/Integration/rootData/077014461/v7r0913p10/digi/digitization-licos-v3r9p2_077014461_digi_DIGI.root


## SECTION C: OPTICAL RESPONSE (MUONGAIN) ##
### C1: MEVPERDAC / ASYM COMBINED USING TRACKER TRACKS
MUOPT_DIGI_FILES = muon_digi_files.txt
MUOPT_SVAC_FILES = muon_svac_files.txt
MUOPT_NENTRIES_PERHIST = 10000


## SECTION D: FLIGHTGAIN EXTRAPOLATION ##
### D1: CALIBGAIN ##
# input HE singlex16 digi root event file (flightgain, calibGain=on, fle=127, calibGen element #106)
CG_DIGI_CGON  = /nfs/farm/g/glast/u41/Integration/rootData/077014465/v7r0913p10/digi/digitization-licos-v3r9p2_077014465_digi_DIGI.root
# input HE singlex16 digi root event file (flightgain, calibGain=off, fle=127, calibGen element #108)
CG_DIGI_CGOFF = /nfs/farm/g/glast/u41/Integration/rootData/077014467/v7r0913p10/digi/digitization-licos-v3r9p2_077014467_digi_DIGI.root

## SECTION E: DAC SETTINGS GENERATION ##
MUONGAIN_DACSLOPES_CFG = 0407.MUON_GAIN.dacSlopes.cfg
MUONGAIN_THOLDCI_CFG   = 0407.MUON_GAIN.tholdci.cfg

FLIGHTGAIN_DACSLOPES_CFG = 0407.FLIGHT_GAIN.dacSlopes.cfg
FLIGHTGAIN_THOLDCI_CFG   = 0407.FLIGHT_GAIN.tholdci.cfg

# most thresholds in MeV
FLIGHTGAIN_FLE_THRESH = 100
FLIGHTGAIN_FHE_THRESH = 1000
FLIGHTGAIN_LAC_THRESH = 2
# uld threshold in percent of adc range
FLIGHTGAIN_ULD_THRESH = 1.5

# most thresholds in MeV
MUONGAIN_FLE_THRESH = 100
MUONGAIN_FHE_THRESH = 1000
MUONGAIN_LAC_THRESH = 2
# uld threshold in percent of adc range
MUONGAIN_ULD_THRESH = 1.5

#########################
### AUTOGEN_FILENAMES ###
#########################
INL_SFX         = .cidac2adc
PED_SFX         = .calPed
ASYM_SFX        = .calAsym
MPD_SFX         = .calMPD
MUSLOPE_SFX     = .muSlope
MUOPT_SFX       = .muonOptical
CG_SFX          = .calibGain
FLIGHTGAIN_SFX = .FLIGHT_GAIN
MUONGAIN_SFX   = .MUON_GAIN
DACSLOPES_SFX   = .dacSlopes
THOLDCI_SFX     = .tholdCI

MUONGAIN_INL_BASE = $(addsuffix ${INL_SFX}, $(addsuffix ${MUONGAIN_SFX}, $(basename $(notdir ${MUONGAIN_INL_DIGI_LE}))))
MUONGAIN_INL_TXT  = $(addsuffix .txt, ${MUONGAIN_INL_BASE})
MUONGAIN_INL_XML  = $(addsuffix .xml, ${MUONGAIN_INL_BASE})
MUONGAIN_INL_VAL  = $(addsuffix .val.root, ${MUONGAIN_INL_BASE})

FLIGHTGAIN_INL_BASE = $(addsuffix ${INL_SFX}, $(addsuffix ${FLIGHTGAIN_SFX}, $(basename $(notdir ${FLIGHTGAIN_INL_DIGI_LE}))))
FLIGHTGAIN_INL_TXT  = $(addsuffix .txt, ${FLIGHTGAIN_INL_BASE})
FLIGHTGAIN_INL_XML  = $(addsuffix .xml, ${FLIGHTGAIN_INL_BASE})
FLIGHTGAIN_INL_VAL  = $(addsuffix .val.root, ${FLIGHTGAIN_INL_BASE})

CG_BASE    = $(basename $(notdir ${CG_DIGI_CGON}))
CG_TXT = $(addsuffix .txt, $(addsuffix ${CG_SFX}, ${CG_BASE}))

CG_ON_BASE = $(addsuffix ${INL_SFX}, $(addsuffix .calibGainOn, ${CG_BASE}))
CG_ON_TXT = $(addsuffix .txt, ${CG_ON_BASE})
CG_ON_XML = $(addsuffix .xml, ${CG_ON_BASE})

CG_OFF_BASE = $(addsuffix ${INL_SFX}, $(addsuffix .calibGainOff, $(basename $(notdir ${CG_DIGI_CGOFF}))))
CG_OFF_TXT = $(addsuffix .txt, ${CG_OFF_BASE})
CG_OFF_XML = $(addsuffix .xml, ${CG_OFF_BASE})


FIRST_MUONGAIN_PED_DIGI = $(shell head -n 1 ${MUONGAIN_PED_DIGI_FILES})
MUONGAIN_PED_BASE = $(addsuffix ${PED_SFX}, $(addsuffix ${MUONGAIN_SFX}, $(basename $(notdir ${FIRST_MUONGAIN_PED_DIGI}))))
MUONGAIN_PED_TXT  = $(addsuffix .txt, ${MUONGAIN_PED_BASE})
MUONGAIN_PED_XML  = $(addsuffix .xml, ${MUONGAIN_PED_BASE})
MUONGAIN_PED_VAL  = $(addsuffix .val.root, ${MUONGAIN_PED_BASE})

FIRST_MUOPT_DIGI = $(shell head -n 1 ${MUOPT_DIGI_FILES})
MUOPT_BASE = $(addsuffix ${MUOPT_SFX}, $(addsuffix ${MUONGAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
MUONGAIN_ASYM_BASE = $(addsuffix ${ASYM_SFX}, ${MUOPT_BASE})
MUONGAIN_ASYM_TXT  = $(addsuffix .txt, ${MUONGAIN_ASYM_BASE})
MUONGAIN_ASYM_XML  = $(addsuffix .xml, ${MUONGAIN_ASYM_BASE})
MUONGAIN_ASYM_VAL  = $(addsuffix .val.root, ${MUONGAIN_ASYM_BASE})

MUONGAIN_MPD_BASE = $(addsuffix ${MPD_SFX}, ${MUOPT_BASE})
MUONGAIN_MPD_TXT  = $(addsuffix .txt, ${MUONGAIN_MPD_BASE})
MUONGAIN_MPD_XML  = $(addsuffix .xml, ${MUONGAIN_MPD_BASE})
MUONGAIN_MPD_VAL  = $(addsuffix .val.root, ${MUONGAIN_MPD_BASE})

MUONGAIN_MUSLOPE_BASE = $(addsuffix ${MUSLOPE_SFX}, $(addsuffix ${MUONGAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
MUONGAIN_MUSLOPE_XML  = $(addsuffix .xml, ${MUONGAIN_MUSLOPE_BASE})
MUONGAIN_MUSLOPE_VAL  = $(addsuffix .val.root, ${MUONGAIN_MUSLOPE_BASE})

FLIGHTGAIN_MUSLOPE_BASE = $(addsuffix ${MUSLOPE_SFX}, $(addsuffix ${FLIGHTGAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
FLIGHTGAIN_MUSLOPE_XML  = $(addsuffix .xml, ${FLIGHTGAIN_MUSLOPE_BASE})
FLIGHTGAIN_MUSLOPE_VAL  = $(addsuffix .val.root, ${FLIGHTGAIN_MUSLOPE_BASE})

FIRST_FLIGHTGAIN_PED_DIGI = $(shell head -n 1 ${FLIGHTGAIN_PED_DIGI_FILES})
FLIGHTGAIN_PED_BASE = $(addsuffix ${PED_SFX}, $(addsuffix ${FLIGHTGAIN_SFX}, $(basename $(notdir ${FIRST_FLIGHTGAIN_PED_DIGI}))))
FLIGHTGAIN_PED_TXT  = $(addsuffix .txt, ${FLIGHTGAIN_PED_BASE})
FLIGHTGAIN_PED_XML  = $(addsuffix .xml, ${FLIGHTGAIN_PED_BASE})
FLIGHTGAIN_PED_VAL  = $(addsuffix .val.root, ${FLIGHTGAIN_PED_BASE})


FLIGHTGAIN_MUOPT_BASE = $(addsuffix ${MUOPT_SFX}, $(addsuffix ${FLIGHTGAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
FLIGHTGAIN_ASYM_BASE = $(addsuffix ${ASYM_SFX}, ${FLIGHTGAIN_MUOPT_BASE})
FLIGHTGAIN_ASYM_XML  =  $(addsuffix .xml, ${FLIGHTGAIN_ASYM_BASE})
FLIGHTGAIN_ASYM_VAL  = $(addsuffix .val.root, ${FLIGHTGAIN_ASYM_BASE})
 
FLIGHTGAIN_MPD_BASE = $(addsuffix ${MPD_SFX}, ${FLIGHTGAIN_MUOPT_BASE})
FLIGHTGAIN_MPD_XML  = $(addsuffix .xml, ${FLIGHTGAIN_MPD_BASE})
FLIGHTGAIN_MPD_VAL  = $(addsuffix .val.root, ${FLIGHTGAIN_MPD_BASE})

FLIGHTGAIN_DACSLOPES_BASE = $(basename $(notdir ${FLIGHTGAIN_DACSLOPES_CFG}))
MUONGAIN_DACSLOPES_BASE = $(basename $(notdir ${MUONGAIN_DACSLOPES_CFG}))
FLIGHTGAIN_DACSLOPES_XML = $(addsuffix .xml, ${FLIGHTGAIN_DACSLOPES_BASE})
FLIGHTGAIN_DACSLOPES_VAL = $(addsuffix .val.root, ${FLIGHTGAIN_DACSLOPES_BASE})
MUONGAIN_DACSLOPES_XML = $(addsuffix .xml, ${MUONGAIN_DACSLOPES_BASE})
MUONGAIN_DACSLOPES_VAL = $(addsuffix .val.root, ${MUONGAIN_DACSLOPES_BASE})

FLIGHTGAIN_THOLDCI_BASE =  $(basename $(notdir ${FLIGHTGAIN_THOLDCI_CFG}))
MUONGAIN_THOLDCI_BASE =  $(basename $(notdir ${MUONGAIN_THOLDCI_CFG}))
FLIGHTGAIN_THOLDCI_XML = $(addsuffix .xml, ${FLIGHTGAIN_THOLDCI_BASE})
FLIGHTGAIN_THOLDCI_VAL = $(addsuffix .val.root, ${FLIGHTGAIN_THOLDCI_BASE})
MUONGAIN_THOLDCI_XML = $(addsuffix .xml, ${MUONGAIN_THOLDCI_BASE})
MUONGAIN_THOLDCI_VAL = $(addsuffix .val.root, ${MUONGAIN_THOLDCI_BASE})


FLIGHTGAIN_FLE_XML = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_FLE_THRESH}MeV.FLIGHT_GAIN.fle.xml
FLIGHTGAIN_FHE_XML = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_FHE_THRESH}MeV.FLIGHT_GAIN.fhe.xml
FLIGHTGAIN_LAC_XML = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_LAC_THRESH}MeV.FLIGHT_GAIN.lac.xml
FLIGHTGAIN_ULD_XML = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_ULD_THRESH}pct.FLIGHT_GAIN.uld.xml
FLIGHTGAIN_DACSETTINGS_XML = ${FLIGHTGAIN_FLE_XML} ${FLIGHTGAIN_FHE_XML} ${FLIGHTGAIN_LAC_XML} ${FLIGHTGAIN_ULD_XML}

MUONGAIN_FLE_XML = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_FLE_THRESH}MeV.MUON_GAIN.fle.xml
MUONGAIN_FHE_XML = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_FHE_THRESH}MeV.MUON_GAIN.fhe.xml
MUONGAIN_LAC_XML = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_LAC_THRESH}MeV.MUON_GAIN.lac.xml
MUONGAIN_ULD_XML = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_ULD_THRESH}pct.MUON_GAIN.uld.xml
MUONGAIN_DACSETTINGS_XML = ${MUONGAIN_FLE_XML} ${MUONGAIN_FHE_XML} ${MUONGAIN_LAC_XML} ${MUONGAIN_ULD_XML}

FLIGHTGAIN_FLE_VAL = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_FLE_THRESH}MeV.FLIGHT_GAIN.fle.val.root
FLIGHTGAIN_FHE_VAL = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_FHE_THRESH}MeV.FLIGHT_GAIN.fhe.val.root
FLIGHTGAIN_LAC_VAL = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_LAC_THRESH}MeV.FLIGHT_GAIN.lac.val.root
FLIGHTGAIN_ULD_VAL = ${FLIGHTGAIN_DACSLOPES_BASE}.${FLIGHTGAIN_ULD_THRESH}pct.FLIGHT_GAIN.uld.val.root

MUONGAIN_FLE_VAL = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_FLE_THRESH}MeV.MUON_GAIN.fle.val.root
MUONGAIN_FHE_VAL = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_FHE_THRESH}MeV.MUON_GAIN.fhe.val.root
MUONGAIN_LAC_VAL = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_LAC_THRESH}MeV.MUON_GAIN.lac.val.root
MUONGAIN_ULD_VAL = ${MUONGAIN_DACSLOPES_BASE}.${MUONGAIN_ULD_THRESH}pct.MUON_GAIN.uld.val.root

####################
## GENERIC RULES ###
####################
%${INL_SFX}.val.root : %${INL_SFX}.xml
	intNonlinVal.sh -R $@ $<

%${INL_SFX}.xml : %${INL_SFX}.txt
	inlTXT2XML.sh $< $@

%${PED_SFX}.xml : %${PED_SFX}.txt
	pedTXT2XML.sh $< $@

%${PED_SFX}.val.root : %${PED_SFX}.xml
	pedVal.sh -R $@ $<


%${ASYM_SFX}.xml : %${ASYM_SFX}.txt
	asymTXT2XML.sh $< $@

%${ASYM_SFX}.val.root : %${ASYM_SFX}.xml
	asymVal.sh -R $@ $<


%${MPD_SFX}.xml : %${MPD_SFX}.txt
	mpdTXT2XML.sh $< $@

%${MPD_SFX}.val.root : %${MPD_SFX}.xml
	mevPerDacVal.sh -R $@ $<

%${MUSLOPE_SFX}.val.root : %${MUSLOPE_SFX}.xml
	muSlopeVal.sh -R $@ $<

##########################
### TARGET OUTPUTS #######
##########################
.PHONY: all
all: flightgain_calib muongain_calib

.PHONY: flightgain_calib
flightgain_calib: flightgain_ped flightgain_inl flightgain_muopt flightgain_dacslopes flightgain_dacsettings flightgain_tholdci flightgain_calibset_val

.PHONY: muongain_calib
muongain_calib: muongain_ped muongain_inl muongain_muopt muongain_dacslopes muongain_dacsettings muongain_tholdci muongain_calibset_val


.PHONY: inl
inl : muongain_inl flightgain_inl

.PHONY : muongain_inl
muongain_inl: muongain_inl_val

.PHONY: muongain_inl_val
muongain_inl_val : ${MUONGAIN_INL_VAL}

.PHONY: muongain_inl_xml
muongain_inl_xml : ${MUONGAIN_INL_XML} 

.PHONY: muongain_inl_txt
muongain_inl_txt : ${MUONGAIN_INL_TXT} 

${MUONGAIN_INL_TXT} : ${MUONGAIN_INL_DIGI_LE} ${MUONGAIN_INL_DIGI_HE}
	genCIDAC2ADC.exe ${INL_COLMODE} -l ${MUONGAIN_INL_DIGI_LE} -h  ${MUONGAIN_INL_DIGI_HE} ${MUONGAIN_INL_BASE}

.PHONY: flightgain_inl
flightgain_inl : flightgain_inl_val

.PHONY: flightgain_inl_val
flightgain_inl_val : ${FLIGHTGAIN_INL_VAL}

.PHONY: flightgain_inl_xml
flightgain_inl_xml : ${FLIGHTGAIN_INL_XML} 

.PHONY: flightgain_inl_txt
flightgain_inl_txt : ${FLIGHTGAIN_INL_TXT} 

${FLIGHTGAIN_INL_TXT} : ${FLIGHTGAIN_INL_DIGI_LE} ${FLIGHTGAIN_INL_DIGI_HE}
	genCIDAC2ADC.exe ${INL_COLMODE} -l ${FLIGHTGAIN_INL_DIGI_LE} -h  ${FLIGHTGAIN_INL_DIGI_HE} ${FLIGHTGAIN_INL_BASE}

.PHONY: calib_gain
calib_gain : cg_txt

.PHONY: cg_txt
cg_txt: ${CG_TXT}
${CG_TXT}: ${CG_ON_XML} ${CG_OFF_XML}
	calibGainCoeff.sh ${CG_ON_XML} ${CG_OFF_XML} > ${CG_TXT}

.PHONY : cg_on_txt
cg_on_txt: ${CG_ON_TXT}
${CG_ON_TXT}:
	genCIDAC2ADC.exe  ${INL_COLMODE} -h ${CG_DIGI_CGON} ${CG_ON_BASE}

.PHONY : cg_off_txt
cg_off_txt: ${CG_OFF_TXT}
${CG_OFF_TXT}:
	genCIDAC2ADC.exe  ${INL_COLMODE} -h ${CG_DIGI_CGOFF} ${CG_OFF_BASE}


.PHONY: peds
peds : ped

.PHONY: ped
ped : muongain_ped flightgain_ped

.PHONY: muongain_ped
muongain_ped: muongain_ped_val

.PHONY: muongain_ped_val
muongain_ped_val : ${MUONGAIN_PED_VAL}

.PHONY: muongain_ped_xml
muongain_ped_xml : ${MUONGAIN_PED_XML}

.PHONY: muongain_ped_txt
muongain_ped_txt : ${MUONGAIN_PED_TXT}

${MUONGAIN_PED_TXT} : ${MUONGAIN_PED_DIGI_FILES}
	genMuonPed.exe ${MUONGAIN_PED_DIGI_FILES} ${MUONGAIN_PED_BASE} --entriesPerHist=${MUONGAIN_PED_NENTRIES_PERHIST} --triggerCut=${MUONGAIN_PED_EVENT_FILTER}




.PHONY : muongain_muopt
muongain_muopt : muongain_muopt_val
.PHONY: muongain_muopt_val 
muongain_muopt_val : muongain_asym_val muongain_mpd_val
.PHONY: muongain_asym_val 
muongain_asym_val : ${MUONGAIN_ASYM_VAL}
.PHONY: muongain_mpd_val 
muongain_mpd_val : ${MUONGAIN_MPD_VAL}



.PHONY: muongain_muopt_xml
muongain_muopt_xml : ${MUONGAIN_ASYM_XML} ${MUONGAIN_MPD_XML}
.PHONY: muongain_asym_xml 
muongain_asym_xml : ${MUONGAIN_ASYM_XML}
.PHONY: muongain_mpd_xml 
muongain_mpd_xml : ${MUONGAIN_MPD_XML}

.PHONY: muongain_asym_txt
muongain_asym_txt : ${MUONGAIN_ASYM_TXT}

.PHONY: muongain_mpd_txt 
muongain_mpd_txt : ${MUONGAIN_MPD_TXT}

.PHONY: muongain_muopt_txt 
muongain_muopt_txt : ${MUONGAIN_ASYM_TXT} ${MUONGAIN_MPD_TXT}


# asym.txt & mpd.txt are created simultaneously
${MUONGAIN_ASYM_TXT} : ${MUONGAIN_MPD_TXT}

${MUONGAIN_MPD_TXT} : ${MUONGAIN_PED_TXT} ${MUONGAIN_INL_TXT} ${MUOPT_DIGI_FILES} ${MUOPT_SVAC_FILES} 
	genMuonCalibTkr.exe ${MUONGAIN_PED_TXT} ${MUONGAIN_INL_TXT}  ${MUOPT_DIGI_FILES} ${MUOPT_SVAC_FILES} ${MUOPT_BASE} --entriesPerHist=${MUOPT_NENTRIES_PERHIST}


.PHONY: flightgain_ped
flightgain_ped : flightgain_ped_val

.PHONY: flightgain_ped_val
flightgain_ped_val : ${FLIGHTGAIN_PED_VAL}
 
.PHONY: flightgain_ped_xml
flightgain_ped_xml : ${FLIGHTGAIN_PED_XML}
 
.PHONY: flightgain_ped_txt
flightgain_ped_txt : ${FLIGHTGAIN_PED_TXT}
 
${FLIGHTGAIN_PED_TXT} : ${FLIGHTGAIN_PED_DIGI_FILES}
	genMuonPed.exe ${FLIGHTGAIN_PED_DIGI_FILES} ${FLIGHTGAIN_PED_BASE} --entriesPerHist=${FLIGHTGAIN_PED_NENTRIES_PERHIST} --triggerCut=${FLIGHTGAIN_PED_EVENT_FILTER}
 

.PHONY: flightgain_muopt
flightgain_muopt : flightgain_asym flightgain_mpd
 
.PHONY: flightgain_asym
flightgain_asym : ${FLIGHTGAIN_ASYM_XML}
${FLIGHTGAIN_ASYM_XML} : ${CG_TXT} ${MUONGAIN_ASYM_XML}
	asymApplyCalibGain.sh ${CG_TXT} ${MUONGAIN_ASYM_XML} ${FLIGHTGAIN_ASYM_XML}
 
.PHONY: flightgain_mpd
flightgain_mpd : ${FLIGHTGAIN_ASYM_XML}
${FLIGHTGAIN_MPD_XML} : ${CG_TXT} ${MUONGAIN_MPD_XML}
	mpdApplyCalibGain.sh ${CG_TXT} ${MUONGAIN_MPD_XML} ${FLIGHTGAIN_ASYM_XML}


.PHONY: onboard_calib
onboard_calib : onboard_ped onboard_gain

 
.PHONY: onboard_ped
onboard_ped : cal_pedestals.h
cal_pedestals.h : ${FLIGHTGAIN_PED_XML}
	genFlightPed.sh ${FLIGHTGAIN_PED_XML}
 
.PHONY: onboard_gain
onboard_gain : cal_gains.h
cal_gains.h: ${FLIGHTGAIN_MUSLOPE_XML}
	genFlightGain.sh ${FLIGHTGAIN_MUSLOPE_XML}


.PHONY : muslope
muslope : muongain_muslope_val  flightgain_muslope_val

.PHONY: muongain_muslope_val
muongain_muslope_val : ${MUONGAIN_MUSLOPE_VAL}

.PHONY: muongain_muslope_xml 
muongain_muslope_xml : ${MUONGAIN_MUSLOPE_XML}

${MUONGAIN_MUSLOPE_XML} :  ${MUONGAIN_INL_XML} ${MUONGAIN_ASYM_XML} ${MUONGAIN_MPD_XML}
	genADC2NRG.sh ${MUONGAIN_INL_XML} ${MUONGAIN_ASYM_XML} ${MUONGAIN_MPD_XML} ${MUONGAIN_ADC2NRG_XML} ${MUONGAIN_MUSLOPE_XML}


.PHONY: flightgain_muslope_val
flightgain_muslope_val : ${FLIGHTGAIN_MUSLOPE_VAL}

.PHONY: flightgain_muslope_xml 
flightgain_muslope_xml : ${FLIGHTGAIN_MUSLOPE_XML}

${FLIGHTGAIN_MUSLOPE_XML} :  ${FLIGHTGAIN_INL_XML} ${FLIGHTGAIN_ASYM_XML} ${FLIGHTGAIN_ASYM_XML}
	genADC2NRG.sh ${FLIGHTGAIN_INL_XML} ${FLIGHTGAIN_ASYM_XML} ${FLIGHTGAIN_ASYM_XML} ${FLIGHTGAIN_ADC2NRG_XML} ${FLIGHTGAIN_MUSLOPE_XML}


.PHONY: dacslopes
dacslopes : dacslopes_val
.PHONY: dacslopes_val
dacslopes_val : flightgain_dacslopes_val muongain_dacslopes_val

.PHONY: flightgain_dacslopes
flightgain_dacslopes : flightgain_dacslopes_val
.PHONY: flightgain_dacslopes_val
flightgain_dacslopes_val: ${FLIGHTGAIN_DACSLOPES_VAL}


.PHONY: muongain_dacslopes
muongain_dacslopes : muongain_dacslopes_val
.PHONY: muongain_dacslopes_val
muongain_dacslopes_val: ${MUONGAIN_DACSLOPES_VAL}

.PHONY: tholdci
tholdci : tholdci_val
.PHONY: tholdci_val
tholdci_val : flightgain_tholdci_val muongain_tholdci_val

.PHONY: flightgain_tholdci
flightgain_tholdci : flightgain_tholdci_val
.PHONY: flightgain_tholdci_val
flightgain_tholdci_val: ${FLIGHTGAIN_THOLDCI_VAL}

.PHONY: muongain_tholdci
muongain_tholdci : muongain_tholdci_val
.PHONY: muongain_tholdci_val
muongain_tholdci_val: ${MUONGAIN_THOLDCI_VAL}



.PHONY: flightgain_dacslopes_xml
flightgain_dacslopes_xml : ${FLIGHTGAIN_DACSLOPES_XML}
${FLIGHTGAIN_DACSLOPES_XML} : ${FLIGHTGAIN_DACSLOPES_CFG} ${FLIGHTGAIN_MUSLOPE_XML}
	dacSlopesGen.sh ${FLIGHTGAIN_DACSLOPES_CFG} ${FLIGHTGAIN_DACSLOPES_XML}

.PHONY: muongain_dacslopes_xml
muongain_dacslopes_xml : ${MUONGAIN_DACSLOPES_XML}
${MUONGAIN_DACSLOPES_XML} : ${MUONGAIN_DACSLOPES_CFG} ${MUONGAIN_MUSLOPE_XML}
	dacSlopesGen.sh ${MUONGAIN_DACSLOPES_CFG} ${MUONGAIN_DACSLOPES_XML}

.PHONY: flightgain_tholdci_xml
flightgain_tholdci_xml : ${FLIGHTGAIN_THOLDCI_XML} 
${FLIGHTGAIN_THOLDCI_XML} : ${FLIGHTGAIN_THOLDCI_CFG} ${FLIGHTGAIN_INL_XML} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_INL_XML} ${FLIGHTGAIN_MUSLOPE_XML}
	tholdCIGen.sh ${FLIGHTGAIN_THOLDCI_CFG} ${FLIGHTGAIN_THOLDCI_XML}

.PHONY: muongain_tholdci_xml
muongain_tholdci_xml : ${MUONGAIN_THOLDCI_XML}
${MUONGAIN_THOLDCI_XML} : ${MUONGAIN_THOLDCI_CFG} ${MUONGAIN_INL_XML} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_INL_XML} ${MUONGAIN_MUSLOPE_XML}
	tholdCIGen.sh ${MUONGAIN_THOLDCI_CFG} ${MUONGAIN_THOLDCI_XML}



.PHONY : dacsettings
dacsettings : dacsettings_val
.PHONY : dacsettings_val
dacsettings_val : muongain_dacsettings_val flightgain_dacsettings_val

.PHONY: muongain_dacsettings
muongain_dacsettings : muongain_dacsettings_val

.PHONY: muongain_dacsettings_val
muongain_dacsettings_val : muongain_fle_val muongain_fhe_val muongain_uld_val muongain_lac_val

.PHONY: muongain_fle_val
muongain_fle_val : ${MUONGAIN_FLE_VAL}

.PHONY: muongain_fhe_val
muongain_fhe_val : ${MUONGAIN_FHE_VAL}

.PHONY: muongain_uld_val
muongain_uld_val : ${MUONGAIN_ULD_VAL}

.PHONY: muongain_lac_val
muongain_lac_val : ${MUONGAIN_LAC_VAL}


${MUONGAIN_FLE_XML} : ${MUONGAIN_DACSLOPES_XML}
	genDACsettings.sh FLE ${MUONGAIN_FLE_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_FLE_XML}

${MUONGAIN_FHE_XML} : ${MUONGAIN_DACSLOPES_XML}
	genDACsettings.sh FHE ${MUONGAIN_FHE_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_FHE_XML}

${MUONGAIN_LAC_XML} : ${MUONGAIN_DACSLOPES_XML}
	genDACsettings.sh LAC ${MUONGAIN_LAC_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_LAC_XML}

${MUONGAIN_ULD_XML} : ${MUONGAIN_DACSLOPES_XML}
	genDACsettings.sh ULD ${MUONGAIN_ULD_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_ULD_XML}


${MUONGAIN_FLE_VAL}  : ${MUONGAIN_FLE_XML} 
	valDACsettings.sh -R ${MUONGAIN_FLE_VAL} FLE ${MUONGAIN_FLE_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_FLE_XML}

${MUONGAIN_FHE_VAL} : ${MUONGAIN_FHE_XML} 
	valDACsettings.sh -R ${MUONGAIN_FHE_VAL} FHE ${MUONGAIN_FHE_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_FHE_XML}

${MUONGAIN_LAC_VAL} : ${MUONGAIN_LAC_XML} 
	valDACsettings.sh -R ${MUONGAIN_LAC_VAL} LAC ${MUONGAIN_LAC_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_LAC_XML}

${MUONGAIN_ULD_VAL} : ${MUONGAIN_ULD_XML} 
	valDACsettings.sh -R ${MUONGAIN_ULD_VAL} ULD ${MUONGAIN_ULD_THRESH} ${MUONGAIN_DACSLOPES_XML} ${MUONGAIN_ULD_XML}


.PHONY : flightgain_dacsettings
flightgain_dacsettings: flightgain_dacsettings_val

.PHONY: flightgain_dacsettings_val
flightgain_dacsettings_val : flightgain_fle_val flightgain_fhe_val flightgain_uld_val flightgain_lac_val

.PHONY: flightgain_fle_val
flightgain_fle_val : ${FLIGHTGAIN_FLE_VAL}

.PHONY: flightgain_fhe_val
flightgain_fhe_val : ${FLIGHTGAIN_FHE_VAL}

.PHONY: flightgain_uld_val
flightgain_uld_val : ${FLIGHTGAIN_ULD_VAL}

.PHONY: flightgain_lac_val
flightgain_lac_val : ${FLIGHTGAIN_LAC_VAL}


${FLIGHTGAIN_FLE_XML} : ${FLIGHTGAIN_DACSLOPES_XML}
	genDACsettings.sh FLE ${FLIGHTGAIN_FLE_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_FLE_XML}

${FLIGHTGAIN_FHE_XML} : ${FLIGHTGAIN_DACSLOPES_XML}
	genDACsettings.sh FHE ${FLIGHTGAIN_FHE_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_FHE_XML}

${FLIGHTGAIN_LAC_XML} : ${FLIGHTGAIN_DACSLOPES_XML}
	genDACsettings.sh LAC ${FLIGHTGAIN_LAC_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_LAC_XML}

${FLIGHTGAIN_ULD_XML} : ${FLIGHTGAIN_DACSLOPES_XML}
	genDACsettings.sh ULD ${FLIGHTGAIN_ULD_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_ULD_XML}


${FLIGHTGAIN_FLE_VAL}  : ${FLIGHTGAIN_FLE_XML} 
	valDACsettings.sh -R ${FLIGHTGAIN_FLE_VAL} FLE ${FLIGHTGAIN_FLE_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_FLE_XML}

${FLIGHTGAIN_FHE_VAL} : ${FLIGHTGAIN_FHE_XML} 
	valDACsettings.sh -R ${FLIGHTGAIN_FHE_VAL} FHE ${FLIGHTGAIN_FHE_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_FHE_XML}

${FLIGHTGAIN_LAC_VAL} : ${FLIGHTGAIN_LAC_XML} 
	valDACsettings.sh -R ${FLIGHTGAIN_LAC_VAL} LAC ${FLIGHTGAIN_LAC_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_LAC_XML}

${FLIGHTGAIN_ULD_VAL} : ${FLIGHTGAIN_ULD_XML} 
	valDACsettings.sh -R ${FLIGHTGAIN_ULD_VAL} ULD ${FLIGHTGAIN_ULD_THRESH} ${FLIGHTGAIN_DACSLOPES_XML} ${FLIGHTGAIN_ULD_XML}


${MUONGAIN_DACSLOPES_VAL} : ${MUONGAIN_DACSLOPES_XML}
	dacSlopesVal.sh -R $@ $<

${MUONGAIN_THOLDCI_VAL} : ${MUONGAIN_THOLDCI_XML}
	tholdCIVal.sh -R $@ $<



${FLIGHTGAIN_DACSLOPES_VAL} : ${FLIGHTGAIN_DACSLOPES_XML}
	dacSlopesVal.sh -R $@ $<

${FLIGHTGAIN_THOLDCI_VAL} : ${FLIGHTGAIN_THOLDCI_XML}
	tholdCIVal.sh -R $@ $<

.PHONY: flightgain_calibset_val
flightgain_calibset_val : ${FLIGHTGAIN_PED_XML} ${FLIGHTGAIN_INL_XML} ${FLIGHTGAIN_ASYM_XML} ${FLIGHTGAIN_MPD_XML} ${FLIGHTGAIN_THOLDCI_XML} 
	calibSetVal.sh -V ${FLIGHTGAIN_PED_XML} ${FLIGHTGAIN_INL_XML} ${FLIGHTGAIN_ASYM_XML} ${FLIGHTGAIN_MPD_XML} ${FLIGHTGAIN_THOLDCI_XML}

.PHONY: muongain_calibset_val
muongain_calibset_val : ${MUONGAIN_PED_XML} ${MUONGAIN_INL_XML} ${MUONGAIN_ASYM_XML} ${MUONGAIN_MPD_XML} ${MUONGAIN_THOLDCI_XML} 
	calibSetVal.sh -V ${MUONGAIN_PED_XML} ${MUONGAIN_INL_XML} ${MUONGAIN_ASYM_XML} ${MUONGAIN_MPD_XML} ${MUONGAIN_THOLDCI_XML}