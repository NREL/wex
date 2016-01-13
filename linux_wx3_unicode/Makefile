VPATH = ../src/dview ../src/plot ../src/pdf ../src

LKDIR = ../../lk
LKLIB = lkuxwx3.a

CC = gcc
CXX = g++
CFLAGS = -fPIC -std=c++0x -Wall -O2 `wx-config-3 --cflags` -I../include -I$(LKDIR)
CXXFLAGS = $(CFLAGS)

LIBNAME = wexuxwx3
TARGET = ../$(LIBNAME).a
	
OBJECTS = \
	snaplay.o \
	pagelayout.o \
	pageobjects.o \
	csv.o \
	exttree.o \
	tpdlg.o \
	label.o \
	exttext.o \
	radiochoice.o \
	metro.o \
	uiform.o \
	diurnal.o \
	utils.o \
	lkscript.o \
	codeedit.o \
	extgrid.o \
	jsonreader.o \
	jsonval.o \
	jsonwriter.o \
	md5.o \
	numeric.o \
	dvstatisticstablectrl.o \
	dvautocolourassigner.o \
	dvcolourmap.o \
	dvdcctrl.o \
	dvdmapctrl.o \
	dvplotctrl.o \
	dvfilereader.o \
	dvplotctrlsettings.o \
	dvplothelper.o \
	dvpncdfctrl.o \
	dvprofilectrl.o \
	dvscatterplotctrl.o \
	dvselectionlist.o \
	dvtimeseriesctrl.o \
	dvtimeseriesdataset.o \
	plaxis.o \
	plbarplot.o \
	plhistplot.o \
	pllineplot.o \
	plplotctrl.o \
	plscatterplot.o \
	plwindrose.o \
	pdfannotation.o \
	pdfbarcode.o \
	pdfcffdecoder.o \
	pdfcffindex.o \
	pdfcolour.o \
	pdfdc.o \
	pdfdecode.o \
	pdfdocument.o \
	pdfencoding.o \
	pdfencrypt.o \
	pdffont.o \
	pdffontdata.o \
	pdffontdatacore.o \
	pdffontdataopentype.o \
	pdffontdatatruetype.o \
	pdffontdatatype0.o \
	pdffontdatatype1.o \
	pdffontdescription.o \
	pdffontdetails.o \
	pdffontextended.o \
	pdffontmanager.o \
	pdffontparser.o \
	pdffontparsertruetype.o \
	pdffontparsertype1.o \
	pdffontsubsetcff.o \
	pdffontsubsettruetype.o \
	pdffontvolt.o \
	pdfform.o \
	pdfgradient.o \
	pdfgraphics.o \
	pdfimage.o \
	pdfkernel.o \
	pdflayer.o \
	pdfobjects.o \
	pdfocg.o \
	pdfparser.o \
	pdfpattern.o \
	pdfprint.o \
	pdfrijndael.o \
	pdftemplate.o \
	pdfutility.o \
	pdfxml.o


$(TARGET) : $(OBJECTS)
	ar rs $@ $^

DEPENDS = $(OBJECTS:.o=.d)

clean:
	rm -rf $(OBJECTS) $(DEPENDS) $(TARGET) 

-include $(DEPENDS)

%.d: %.cpp
	$(CXX) -MM $(CFLAGS) $^ > $*.d
	@cp -f $*.d $*.d.tmp
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp

