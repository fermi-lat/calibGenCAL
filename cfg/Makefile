# $Header: /nfs/slac/g/glast/ground/cvs/calibGenCAL/cfg/Makefile,v 1.8 2007/04/12 21:11:34 fewtrell Exp $
# @file calibGenCAL analysis Makefile - manage generation of all calibGenCAL outputs and accessory files
# @requires GNU Make
# @requires calibGenCAL package CMT environment setup - this means source your $CALIBGENCALROOT/cmt/setup.(csh|sh|bat)


##################
## INPUT FILES ###
##################

## SECTION A: PEDESTALS ##

### A1: MUON GAIN PEDESTALS
MUPED_DIGI_FILES = muon_digi_files.txt
# options 'PERIODIC' 'EXTERNAL' 'ALL'
MUPED_EVENT_FILTER     = PERIODIC
# 1000 events minimum per hist is sufficient
MUPED_NENTRIES_PERHIST = 1000

### A2: FLIGHT GAIN PEDESTALS
FLIGHTPED_DIGI_FILES = flightped_digi_files.txt
# options 'PERIODIC' 'EXTERNAL' 'ALL'
FLIGHTPED_EVENT_FILTER     = PERIODIC
# 1000 events minimum per hist is sufficient
FLIGHTPED_NENTRIES_PERHIST = 1000


## SECTION B: INT-NONLIN (CIDAC2ADC) ##
# set to '-c' if inl data is in column mode, blank if not
#INL_COLMODE    = -c

### B1: MUON GAIN INT-NONLIN
# input LE singlex16 digi root event file (muon gain, calibGain=on, calibGen element#201)
INL_MUON_DIGI_LE = /nfs/farm/g/glast/u41/Integration/rootData/077014468/v7r0913p10/digi/digitization-licos-v3r9p2_077014468_digi_DIGI.root
# input HE singlex16 digi root event file (muon gain), calibGain=off, calibGen element#204)
INL_MUON_DIGI_HE = /nfs/farm/g/glast/u41/Integration/rootData/077014471/v7r0913p10/digi/digitization-licos-v3r9p2_077014471_digi_DIGI.root

### B2: FLIGHT GAIN INT-NONLIN
# input LE singlex16 digi root event file (flight gain, calibGain=on, fle=nominal, calibGen element#101)
INL_FLIGHT_DIGI_LE = /nfs/farm/g/glast/u41/Integration/rootData/077014460/v7r0913p10/digi/digitization-licos-v3r9p2_077014460_digi_DIGI.root
# input HE singlex16 digi root event file (flight gain, calibGain=on, fle=nominla, calibGen element#102)
INL_FLIGHT_DIGI_HE = /nfs/farm/g/glast/u41/Integration/rootData/077014461/v7r0913p10/digi/digitization-licos-v3r9p2_077014461_digi_DIGI.root

### B3: 'NEIGHBOR' XTALK
# input 'NeighborXtalk' singlex16 digi root event file (flight gain, le_pulse_only, columnwise)
INPUT_NEIGHBOR_XTALK = /nfs/slac/g/glast/users/glground/bregeon/DIGICALCI/700002700.root



## SECTION C: OPTICAL RESPONSE (MUON) ##
### C1: MEVPERDAC / ASYM COMBINED USING TRACKER TRACKS
MUOPT_DIGI_FILES = muon_digi_files.txt
MUOPT_SVAC_FILES = muon_svac_files.txt
MUOPT_NENTRIES_PERHIST = 10000

## SECTION D: FLIGHT GAIN EXTRAPOLATION ##
### D1: CALIBGAIN ##
# input HE singlex16 digi root event file (flight_gain, calibGain=on, fle=127, calibGen element #106)
CG_DIGI_CGON  = /nfs/farm/g/glast/u41/Integration/rootData/077014465/v7r0913p10/digi/digitization-licos-v3r9p2_077014465_digi_DIGI.root
# input HE singlex16 digi root event file (flight_gain, calibGain=off, fle=127, calibGen element #108)
CG_DIGI_CGOFF = /nfs/farm/g/glast/u41/Integration/rootData/077014467/v7r0913p10/digi/digitization-licos-v3r9p2_077014467_digi_DIGI.root

## SECTION E: DAC SETTINGS GENERATION ##
DACSLOPES_MUON_CFG = 0407.MUON_GAIN.dacSlopes.cfg
THOLDCI_MUON_CFG   = 0407.MUON_GAIN.tholdci.cfg

DACSLOPES_FLIGHT_CFG = 0407.FLIGHT_GAIN.dacSlopes.cfg
THOLDCI_FLIGHT_CFG   = 0407.FLIGHT_GAIN.tholdci.cfg

# most thresholds in MeV
FLE_FLIGHT_THRESH = 100
FHE_FLIGHT_THRESH = 1000
LAC_FLIGHT_THRESH = 2
# uld threshold in percent of adc range
ULD_FLIGHT_THRESH = 1.5

# most thresholds in MeV
FLE_MUON_THRESH = 100
FHE_MUON_THRESH = 1000
LAC_MUON_THRESH = 2
# uld threshold in percent of adc range
ULD_MUON_THRESH = 1.5

#########################
### AUTOGEN_FILENAMES ###
#########################
INL_SFX         = .cidac2adc
PED_SFX         = .calPed
ASYM_SFX        = .calAsym
MPD_SFX         = .calMPD
MUSLOPE_SFX     = .muSlope
ADC2NRG_SFX     = .adc2nrg
MUOPT_SFX       = .muonOptical
CG_SFX          = .calibGain
FLIGHT_GAIN_SFX = .FLIGHT_GAIN
MUON_GAIN_SFX   = .MUON_GAIN
DACSLOPES_SFX   = .dacSlopes
THOLDCI_SFX     = .tholdCI

INL_MUON_BASE = $(addsuffix ${INL_SFX}, $(addsuffix ${MUON_GAIN_SFX}, $(basename $(notdir ${INL_MUON_DIGI_LE}))))
INL_MUON_TXT  = $(addsuffix .txt, ${INL_MUON_BASE})
INL_MUON_XML  = $(addsuffix .xml, ${INL_MUON_BASE})
INL_MUON_VAL  = $(addsuffix .val.root, ${INL_MUON_BASE})

INL_FLIGHT_BASE = $(addsuffix ${INL_SFX}, $(addsuffix ${FLIGHT_GAIN_SFX}, $(basename $(notdir ${INL_FLIGHT_DIGI_LE}))))
INL_FLIGHT_TXT  = $(addsuffix .txt, ${INL_FLIGHT_BASE})
INL_FLIGHT_XML  = $(addsuffix .xml, ${INL_FLIGHT_BASE})
INL_FLIGHT_VAL  = $(addsuffix .val.root, ${INL_FLIGHT_BASE})

CG_BASE    = $(basename $(notdir ${CG_DIGI_CGON}))
CG_TXT = $(addsuffix .txt, $(addsuffix ${CG_SFX}, ${CG_BASE}))

CG_ON_BASE = $(addsuffix ${INL_SFX}, $(addsuffix .calibGainOn, ${CG_BASE}))
CG_ON_TXT = $(addsuffix .txt, ${CG_ON_BASE})
CG_ON_XML = $(addsuffix .xml, ${CG_ON_BASE})

CG_OFF_BASE = $(addsuffix ${INL_SFX}, $(addsuffix .calibGainOff, $(basename $(notdir ${CG_DIGI_CGOFF}))))
CG_OFF_TXT = $(addsuffix .txt, ${CG_OFF_BASE})
CG_OFF_XML = $(addsuffix .xml, ${CG_OFF_BASE})


NEIGHBOR_XTALK_BASE = $(addsuffix ${NBR_XTALK_SFX}, $(basename $(notdir ${INPUT_NEIGHBOR_XTALK})))
NEIGHBOR_XTALK_TXT  = $(addsuffix .txt, ${NEIGHBOR_XTALK_BASE})

FIRST_MUPED_DIGI = $(shell head -n 1 ${MUPED_DIGI_FILES})
PED_MUON_BASE = $(addsuffix ${PED_SFX}, $(addsuffix ${MUON_GAIN_SFX}, $(basename $(notdir ${FIRST_MUPED_DIGI}))))
PED_MUON_TXT  = $(addsuffix .txt, ${PED_MUON_BASE})
PED_MUON_XML  = $(addsuffix .xml, ${PED_MUON_BASE})
PED_MUON_VAL  = $(addsuffix .val.root, ${PED_MUON_BASE})

FIRST_MUOPT_DIGI = $(shell head -n 1 ${MUOPT_DIGI_FILES})
MUOPT_BASE = $(addsuffix ${MUOPT_SFX}, $(addsuffix ${MUON_GAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
ASYM_MUON_BASE = $(addsuffix ${ASYM_SFX}, ${MUOPT_BASE})
ASYM_MUON_TXT  = $(addsuffix .txt, ${ASYM_MUON_BASE})
ASYM_MUON_XML  = $(addsuffix .xml, ${ASYM_MUON_BASE})
ASYM_MUON_VAL  = $(addsuffix .val.root, ${ASYM_MUON_BASE})

MPD_MUON_BASE = $(addsuffix ${MPD_SFX}, ${MUOPT_BASE})
MPD_MUON_TXT  = $(addsuffix .txt, ${MPD_MUON_BASE})
MPD_MUON_XML  = $(addsuffix .xml, ${MPD_MUON_BASE})
MPD_MUON_VAL  = $(addsuffix .val.root, ${MPD_MUON_BASE})

MUSLOPE_MUON_BASE = $(addsuffix ${MUSLOPE_SFX}, $(addsuffix ${MUON_GAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
MUSLOPE_MUON_XML  = $(addsuffix .xml, ${MUSLOPE_MUON_BASE})
MUSLOPE_MUON_VAL  = $(addsuffix .val.root, ${MUSLOPE_MUON_BASE})

ADC2NRG_MUON_BASE = $(addsuffix ${ADC2NRG_SFX}, $(addsuffix ${MUON_GAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
ADC2NRG_MUON_XML  = $(addsuffix .xml, ${ADC2NRG_MUON_BASE})
ADC2NRG_MUON_VAL  = $(addsuffix .val.root, ${ADC2NRG_MUON_BASE})

MUSLOPE_FLIGHT_BASE = $(addsuffix ${MUSLOPE_SFX}, $(addsuffix ${FLIGHT_GAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
MUSLOPE_FLIGHT_XML  = $(addsuffix .xml, ${MUSLOPE_FLIGHT_BASE})
MUSLOPE_FLIGHT_VAL  = $(addsuffix .val.root, ${MUSLOPE_FLIGHT_BASE})

ADC2NRG_FLIGHT_BASE = $(addsuffix ${ADC2NRG_SFX}, $(addsuffix ${FLIGHT_GAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
ADC2NRG_FLIGHT_XML  = $(addsuffix .xml, ${ADC2NRG_FLIGHT_BASE})
ADC2NRG_FLIGHT_VAL  = $(addsuffix .val.root, ${ADC2NRG_FLIGHT_BASE})

FIRST_FLIGHTPED_DIGI = $(shell head -n 1 ${FLIGHTPED_DIGI_FILES})
PED_FLIGHT_BASE = $(addsuffix ${PED_SFX}, $(addsuffix ${FLIGHT_GAIN_SFX}, $(basename $(notdir ${FIRST_FLIGHTPED_DIGI}))))
PED_FLIGHT_TXT  = $(addsuffix .txt, ${PED_FLIGHT_BASE})
PED_FLIGHT_XML  = $(addsuffix .xml, ${PED_FLIGHT_BASE})
PED_FLIGHT_VAL  = $(addsuffix .val.root, ${PED_FLIGHT_BASE})


MUOPT_FLIGHT_BASE = $(addsuffix ${MUOPT_SFX}, $(addsuffix ${FLIGHT_GAIN_SFX}, $(basename $(notdir ${FIRST_MUOPT_DIGI}))))
ASYM_FLIGHT_BASE = $(addsuffix ${ASYM_SFX}, ${MUOPT_FLIGHT_BASE})
ASYM_FLIGHT_XML  =  $(addsuffix .xml, ${ASYM_FLIGHT_BASE})
ASYM_FLIGHT_VAL  = $(addsuffix .val.root, ${ASYM_FLIGHT_BASE})
 
MPD_FLIGHT_BASE = $(addsuffix ${MPD_SFX}, ${MUOPT_FLIGHT_BASE})
MPD_FLIGHT_XML  = $(addsuffix .xml, ${MPD_FLIGHT_BASE})
MPD_FLIGHT_VAL  = $(addsuffix .val.root, ${MPD_FLIGHT_BASE})

DACSLOPES_FLIGHT_BASE = $(basename $(notdir ${DACSLOPES_FLIGHT_CFG}))
DACSLOPES_MUON_BASE = $(basename $(notdir ${DACSLOPES_MUON_CFG}))
DACSLOPES_FLIGHT_XML = $(addsuffix .xml, ${DACSLOPES_FLIGHT_BASE})
DACSLOPES_FLIGHT_VAL = $(addsuffix .val.root, ${DACSLOPES_FLIGHT_BASE})
DACSLOPES_MUON_XML = $(addsuffix .xml, ${DACSLOPES_MUON_BASE})
DACSLOPES_MUON_VAL = $(addsuffix .val.root, ${DACSLOPES_MUON_BASE})

THOLDCI_FLIGHT_BASE =  $(basename $(notdir ${THOLDCI_FLIGHT_CFG}))
THOLDCI_MUON_BASE =  $(basename $(notdir ${THOLDCI_MUON_CFG}))
THOLDCI_FLIGHT_XML = $(addsuffix .xml, ${THOLDCI_FLIGHT_BASE})
THOLDCI_FLIGHT_VAL = $(addsuffix .val.root, ${THOLDCI_FLIGHT_BASE})
THOLDCI_MUON_XML = $(addsuffix .xml, ${THOLDCI_MUON_BASE})
THOLDCI_MUON_VAL = $(addsuffix .val.root, ${THOLDCI_MUON_BASE})

FLE_FLIGHT_XML = ${DACSLOPES_FLIGHT_BASE}.${FLE_FLIGHT_THRESH}MeV.FLIGHT_GAIN.fle.xml
FHE_FLIGHT_XML = ${DACSLOPES_FLIGHT_BASE}.${FHE_FLIGHT_THRESH}MeV.FLIGHT_GAIN.fhe.xml
LAC_FLIGHT_XML = ${DACSLOPES_FLIGHT_BASE}.${LAC_FLIGHT_THRESH}MeV.FLIGHT_GAIN.lac.xml
ULD_FLIGHT_XML = ${DACSLOPES_FLIGHT_BASE}.${ULD_FLIGHT_THRESH}pct.FLIGHT_GAIN.uld.xml

FLE_MUON_XML = ${DACSLOPES_MUON_BASE}.${FLE_MUON_THRESH}MeV.MUON_GAIN.fle.xml
FHE_MUON_XML = ${DACSLOPES_MUON_BASE}.${FHE_MUON_THRESH}MeV.MUON_GAIN.fhe.xml
LAC_MUON_XML = ${DACSLOPES_MUON_BASE}.${LAC_MUON_THRESH}MeV.MUON_GAIN.lac.xml
ULD_MUON_XML = ${DACSLOPES_MUON_BASE}.${ULD_MUON_THRESH}pct.MUON_GAIN.uld.xml

FLE_FLIGHT_VAL = ${DACSLOPES_FLIGHT_BASE}.${FLE_FLIGHT_THRESH}MeV.FLIGHT_GAIN.fle.val.root
FHE_FLIGHT_VAL = ${DACSLOPES_FLIGHT_BASE}.${FHE_FLIGHT_THRESH}MeV.FLIGHT_GAIN.fhe.val.root
LAC_FLIGHT_VAL = ${DACSLOPES_FLIGHT_BASE}.${LAC_FLIGHT_THRESH}MeV.FLIGHT_GAIN.lac.val.root
ULD_FLIGHT_VAL = ${DACSLOPES_FLIGHT_BASE}.${ULD_FLIGHT_THRESH}pct.FLIGHT_GAIN.uld.val.root

FLE_MUON_VAL = ${DACSLOPES_MUON_BASE}.${FLE_MUON_THRESH}MeV.MUON_GAIN.fle.val.root
FHE_MUON_VAL = ${DACSLOPES_MUON_BASE}.${FHE_MUON_THRESH}MeV.MUON_GAIN.fhe.val.root
LAC_MUON_VAL = ${DACSLOPES_MUON_BASE}.${LAC_MUON_THRESH}MeV.MUON_GAIN.lac.val.root
ULD_MUON_VAL = ${DACSLOPES_MUON_BASE}.${ULD_MUON_THRESH}pct.MUON_GAIN.uld.val.root

####################
## GENERIC RULES ###
####################
%${INL_SFX}.val.root : %${INL_SFX}.xml
	intNonlinVal.sh -R $@ $<

%${INL_SFX}.xml : %${INL_SFX}.txt
	inlTXT2XML.sh $< $@

# %${INL_SFX}.txt : %${ADCMEAN_SFX}.txt
# 	smoothINL.exe $< $@

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

%${ADC2NRG_SFX}.val.root : %${ADC2NRG_SFX}.xml
	adc2nrgVal.sh -R $@ $<

##########################
### TARGET OUTPUTS #######
##########################
.PHONY: all
all: ped inl muopt flight_opt muslope dac_settings tholdci

.PHONY: inl
inl : inl_muon inl_flight

.PHONY : inl_muon
inl_muon: inl_muon_val

.PHONY: inl_muon_val
inl_muon_val : ${INL_MUON_VAL}

.PHONY: inl_muon_xml
inl_muon_xml : ${INL_MUON_XML} 

.PHONY: inl_muon_txt
inl_muon_txt : ${INL_MUON_TXT} 

${INL_MUON_TXT} : ${INL_MUON_DIGI_LE} ${INL_MUON_DIGI_HE}
	genCIDAC2ADC.exe ${INL_COLMODE} -l ${INL_MUON_DIGI_LE} -h  ${INL_MUON_DIGI_HE} ${INL_MUON_BASE}

.PHONY: inl_flight
inl_flight : inl_flight_val

.PHONY: inl_flight_val
inl_flight_val : ${INL_FLIGHT_VAL}

.PHONY: inl_flight_xml
inl_flight_xml : ${INL_FLIGHT_XML} 

.PHONY: inl_flight_txt
inl_flight_txt : ${INL_FLIGHT_TXT} 

${INL_FLIGHT_TXT} : ${INL_FLIGHT_DIGI_LE} ${INL_FLIGHT_DIGI_HE}
	genCIDAC2ADC.exe ${INL_COLMODE} -l ${INL_FLIGHT_DIGI_LE} -h  ${INL_FLIGHT_DIGI_HE} ${INL_FLIGHT_BASE}

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


.PHONY: neighbor_xtalk
neighbor_xtalk : ${NEIGHBOR_XTALK_TXT}

${NEIGHBOR_XTALK_TXT} : ${INPUT_NEIGHBOR_XTALK}
	genNeighborXtalk.exe $< ${NEIGHBOR_XTALK_BASE}


.PHONY: peds
peds : ped

.PHONY: ped
ped : ped_muon_val

.PHONY: ped_muon_val
ped_muon_val : ${PED_MUON_VAL}

.PHONY: ped_muon_xml
ped_muon_xml : ${PED_MUON_XML}

.PHONY: ped_muon_txt
ped_muon_txt : ${PED_MUON_TXT}

${PED_MUON_TXT} : ${MUPED_DIGI_FILES}
	genMuonPed.exe ${MUPED_DIGI_FILES} ${PED_MUON_BASE} --entriesPerHist=${MUPED_NENTRIES_PERHIST} --triggerCut=${MUPED_EVENT_FILTER}




.PHONY : muopt
muopt : muopt_val
.PHONY: muopt_val 
muopt_val : asym_muon_val mpd_muon_val
.PHONY: asym_muon_val 
asym_muon_val : ${ASYM_MUON_VAL}
.PHONY: mpd_muon_val 
mpd_muon_val : ${MPD_MUON_VAL}



.PHONY: muopt_xml
muopt_xml : ${ASYM_MUON_XML} ${MPD_MUON_XML}
.PHONY: asym_muon_xml 
asym_muon_xml : ${ASYM_MUON_XML}
.PHONY: mpd_muon_xml 
mpd_muon_xml : ${MPD_MUON_XML}

.PHONY: asym_muon_txt
asym_muon_txt : ${ASYM_MUON_TXT}

.PHONY: mpd_muon_txt 
mpd_muon_txt : ${MPD_MUON_TXT}

.PHONY: muopt_txt 
muopt_txt : ${ASYM_MUON_TXT} ${MPD_MUON_TXT}


# asym.txt & mpd.txt are created simultaneously
${ASYM_MUON_TXT} : ${MPD_MUON_TXT}

${MPD_MUON_TXT} : ${PED_MUON_TXT} ${INL_MUON_TXT} ${MUOPT_DIGI_FILES} ${MUOPT_SVAC_FILES} 
	genMuonCalibTkr.exe ${PED_MUON_TXT} ${INL_MUON_TXT}  ${MUOPT_DIGI_FILES} ${MUOPT_SVAC_FILES} ${MUOPT_BASE} --entriesPerHist=${MUOPT_NENTRIES_PERHIST}


.PHONY: ped_flight_val
ped_flight_val : ${PED_FLIGHT_VAL}
 
.PHONY: ped_flight_xml
ped_flight_xml : ${PED_FLIGHT_XML}
 
.PHONY: ped_flight_txt
ped_flight_txt : ${PED_FLIGHT_TXT}
 
${PED_FLIGHT_TXT} : ${FLIGHTPED_DIGI_FILES}
	genMuonPed.exe ${FLIGHTPED_DIGI_FILES} ${PED_FLIGHT_BASE} --entriesPerHist=${FLIGHTPED_NENTRIES_PERHIST} --triggerCut=${FLIGHTPED_EVENT_FILTER}
 

.PHONY: flight_opt
flight_opt : flight_asym flight_mpd
 
.PHONY: flight_asym
flight_asym : ${ASYM_FLIGHT_XML}
${ASYM_FLIGHT_XML} : ${CG_TXT} ${ASYM_MUON_XML}
	asymApplyCalibGain.sh ${CG_TXT} ${ASYM_MUON_XML} ${ASYM_FLIGHT_XML}
 
.PHONY: flight_mpd
flight_mpd : ${MPD_FLIGHT_XML}
${MPD_FLIGHT_XML} : ${CG_TXT} ${MPD_MUON_XML}
	mpdApplyCalibGain.sh ${CG_TXT} ${MPD_MUON_XML} ${MPD_FLIGHT_XML}


.PHONY: onboard_calib
onboard_calib : onboard_ped onboard_gain

 
.PHONY: onboard_ped
onboard_ped : cal_pedestals.h
cal_pedestals.h : ${PED_FLIGHT_XML}
	genFlightPed.sh ${PED_FLIGHT_XML}
 
.PHONY: onboard_gain
onboard_gain : cal_gains.h
cal_gains.h: ${MUSLOPE_FLIGHT_XML}
	genFlightGain.sh ${FLIGHT_MUSLOPE_XML}


.PHONY : muslope
muslope : muslope_muon_val  muslope_flight_val

.PHONY: muslope_muon_val
muslope_muon_val : ${MUSLOPE_MUON_VAL}
.PHONY: adc2nrg_muon_val 
adc2nrg_muon_val : ${ADC2NRG_MUON_VAL}

.PHONY: muslope_muon_xml 
muslope_muon_xml : ${MUSLOPE_MUON_XML}

# adc2nrg.xml & muslope.xml are created simultaneously
${ADC2NRG_MUON_XML}: ${MUSLOPE_MUON_XML}

${MUSLOPE_MUON_XML} :  ${INL_MUON_XML} ${ASYM_MUON_XML} ${MPD_MUON_XML}
	genADC2NRG.sh ${INL_MUON_XML} ${ASYM_MUON_XML} ${MPD_MUON_XML} ${ADC2NRG_MUON_XML} ${MUSLOPE_MUON_XML}


.PHONY: muslope_flight_val
muslope_flight_val : ${MUSLOPE_FLIGHT_VAL}
.PHONY: adc2nrg_flight_val 
adc2nrg_flight_val : ${ADC2NRG_FLIGHT_VAL}

.PHONY: muslope_flight_xml 
muslope_flight_xml : ${MUSLOPE_FLIGHT_XML}

# adc2nrg.xml & muslope.xml are created simultaneously
#${MUSLOPE_FLIGHT_XML} :${ADC2NRG_FLIGHT_XML}

${MUSLOPE_FLIGHT_XML} :  ${INL_FLIGHT_XML} ${ASYM_FLIGHT_XML} ${MPD_FLIGHT_XML}
	genADC2NRG.sh ${INL_FLIGHT_XML} ${ASYM_FLIGHT_XML} ${MPD_FLIGHT_XML} ${ADC2NRG_FLIGHT_XML} ${MUSLOPE_FLIGHT_XML}


.PHONY: dacslopes
dacslopes : dacslopes_val
.PHONY: dacslopes_val
dacslopes_val : dacslopes_flight_val dacslopes_muon_val

.PHONY: dacslopes_flight
dacslopes_flight : dacslopes_flight_val
.PHONY: dacslopes_flight_val
dacslopes_flight_val: ${DACSLOPES_FLIGHT_VAL}


.PHONY: dacslopes_muon
dacslopes_muon : dacslopes_muon_val
.PHONY: dacslopes_muon_val
dacslopes_muon_val: ${DACSLOPES_MUON_VAL}

.PHONY: tholdci
tholdci : tholdci_val
.PHONY: tholdci_val
tholdci_val : tholdci_flight_val tholdci_muon_val

.PHONY: tholdci_flight
tholdci_flight : tholdci_flight_val
.PHONY: tholdci_flight_val
tholdci_flight_val: ${THOLDCI_FLIGHT_VAL}

.PHONY: tholdci_muon
tholdci_muon : tholdci_muon_val
.PHONY: tholdci_muon_val
tholdci_muon_val: ${THOLDCI_MUON_VAL}



.PHONY: dacslopes_flight_xml
dacslopes_flight_xml : ${DACSLOPES_FLIGHT_XML}
${DACSLOPES_FLIGHT_XML} : ${DACSLOPES_FLIGHT_CFG} ${MUSLOPE_FLIGHT_XML}
	dacSlopesGen.sh ${DACSLOPES_FLIGHT_CFG} ${DACSLOPES_FLIGHT_XML}

.PHONY: dacslopes_muon_xml
dacslopes_muon_xml : ${DACSLOPES_MUON_XML}
${DACSLOPES_MUON_XML} : ${DACSLOPES_MUON_CFG} ${MUSLOPE_MUON_XML}
	dacSlopesGen.sh ${DACSLOPES_MUON_CFG} ${DACSLOPES_MUON_XML}

.PHONY: tholdci_flight_xml
tholdci_flight_xml : ${THOLDCI_FLIGHT_XML}
.PHONY: dac_settings_flight
${THOLDCI_FLIGHT_XML} : ${THOLDCI_FLIGHT_CFG} 
	tholdCIGen.sh ${THOLDCI_FLIGHT_CFG} ${THOLDCI_FLIGHT_XML}

.PHONY: tholdci_muon_xml
tholdci_muon_xml : ${THOLDCI_MUON_XML}
.PHONY: dac_settings_muon
${THOLDCI_MUON_XML} : ${THOLDCI_MUON_CFG} 
	tholdCIGen.sh ${THOLDCI_MUON_CFG} ${THOLDCI_MUON_XML}



.PHONY : dac_settings
dac_settings : dac_settings_val
.PHONY : dac_settings_val
dac_settings_val : dac_settings_muon_val dac_settings_flight_val

.PHONY: dac_settings_muon
dac_settings_muon : dac_settings_muon_val

.PHONY: dac_settings_muon_val
dac_settings_muon_val : fle_muon_val fhe_muon_val uld_muon_val lac_muon_val

.PHONY: fle_muon_val
fle_muon_val : ${FLE_MUON_VAL}

.PHONY: fhe_muon_val
fhe_muon_val : ${FHE_MUON_VAL}

.PHONY: uld_muon_val
uld_muon_val : ${ULD_MUON_VAL}

.PHONY: lac_muon_val
lac_muon_val : ${LAC_MUON_VAL}


${FLE_MUON_XML} : ${DACSLOPES_MUON_XML}
	genDACsettings.sh FLE ${FLE_MUON_THRESH} ${DACSLOPES_MUON_XML} ${FLE_MUON_XML}

${FHE_MUON_XML} : ${DACSLOPES_MUON_XML}
	genDACsettings.sh FHE ${FHE_MUON_THRESH} ${DACSLOPES_MUON_XML} ${FHE_MUON_XML}

${LAC_MUON_XML} : ${DACSLOPES_MUON_XML}
	genDACsettings.sh LAC ${LAC_MUON_THRESH} ${DACSLOPES_MUON_XML} ${LAC_MUON_XML}

${ULD_MUON_XML} : ${DACSLOPES_MUON_XML}
	genDACsettings.sh ULD ${ULD_MUON_THRESH} ${DACSLOPES_MUON_XML} ${ULD_MUON_XML}


${FLE_MUON_VAL}  : ${FLE_MUON_XML} 
	valDACsettings.sh -R ${FLE_MUON_VAL} FLE ${FLE_MUON_THRESH} ${DACSLOPES_MUON_XML} ${FLE_MUON_XML}

${FHE_MUON_VAL} : ${FHE_MUON_XML} 
	valDACsettings.sh -R ${FHE_MUON_VAL} FHE ${FHE_MUON_THRESH} ${DACSLOPES_MUON_XML} ${FHE_MUON_XML}

${LAC_MUON_VAL} : ${LAC_MUON_XML} 
	valDACsettings.sh -R ${LAC_MUON_VAL} LAC ${LAC_MUON_THRESH} ${DACSLOPES_MUON_XML} ${LAC_MUON_XML}

${ULD_MUON_VAL} : ${ULD_MUON_XML} 
	valDACsettings.sh -R ${ULD_MUON_VAL} ULD ${ULD_MUON_THRESH} ${DACSLOPES_MUON_XML} ${ULD_MUON_XML}


.PHONY : dac_settings_flight
dac_settings_flight: dac_settings_flight_val

.PHONY: dac_settings_flight_val
dac_settings_flight_val : fle_flight_val fhe_flight_val uld_flight_val lac_flight_val

.PHONY: fle_flight_val
fle_flight_val : ${FLE_FLIGHT_VAL}

.PHONY: fhe_flight_val
fhe_flight_val : ${FHE_FLIGHT_VAL}

.PHONY: uld_flight_val
uld_flight_val : ${ULD_FLIGHT_VAL}

.PHONY: lac_flight_val
lac_flight_val : ${LAC_FLIGHT_VAL}


${FLE_FLIGHT_XML} : ${DACSLOPES_FLIGHT_XML}
	genDACsettings.sh FLE ${FLE_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${FLE_FLIGHT_XML}

${FHE_FLIGHT_XML} : ${DACSLOPES_FLIGHT_XML}
	genDACsettings.sh FHE ${FHE_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${FHE_FLIGHT_XML}

${LAC_FLIGHT_XML} : ${DACSLOPES_FLIGHT_XML}
	genDACsettings.sh LAC ${LAC_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${LAC_FLIGHT_XML}

${ULD_FLIGHT_XML} : ${DACSLOPES_FLIGHT_XML}
	genDACsettings.sh ULD ${ULD_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${ULD_FLIGHT_XML}


${FLE_FLIGHT_VAL}  : ${FLE_FLIGHT_XML} 
	valDACsettings.sh -R ${FLE_FLIGHT_VAL} FLE ${FLE_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${FLE_FLIGHT_XML}

${FHE_FLIGHT_VAL} : ${FHE_FLIGHT_XML} 
	valDACsettings.sh -R ${FHE_FLIGHT_VAL} FHE ${FHE_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${FHE_FLIGHT_XML}

${LAC_FLIGHT_VAL} : ${LAC_FLIGHT_XML} 
	valDACsettings.sh -R ${LAC_FLIGHT_VAL} LAC ${LAC_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${LAC_FLIGHT_XML}

${ULD_FLIGHT_VAL} : ${ULD_FLIGHT_XML} 
	valDACsettings.sh -R ${ULD_FLIGHT_VAL} ULD ${ULD_FLIGHT_THRESH} ${DACSLOPES_FLIGHT_XML} ${ULD_FLIGHT_XML}


${DACSLOPES_MUON_VAL} : ${DACSLOPES_MUON_XML}
	dacSlopesVal.sh -R $@ $<

${THOLDCI_MUON_VAL} : ${THOLDCI_MUON_XML}
	tholdCIVal.sh -R $@ $<



${DACSLOPES_FLIGHT_VAL} : ${DACSLOPES_FLIGHT_XML}
	dacSlopesVal.sh -R $@ $<

${THOLDCI_FLIGHT_VAL} : ${THOLDCI_FLIGHT_XML}
	tholdCIVal.sh -R $@ $<
