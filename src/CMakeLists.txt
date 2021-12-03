#####################################################################################################################
#
# Project Settings
#
#####################################################################################################################

if (NOT APPLE)
    set(CURL_DIR ../build_resources/libcurl_ssl_x64)
endif ()

set(WEX_SRC
        clipper/clipper.cpp
        codeedit.cpp
        csv.cpp
        dclatex.cpp
        dview/dvautocolourassigner.cpp
        dview/dvdcctrl.cpp
        dview/dvdmapctrl.cpp
        dview/dvfilereader.cpp
        dview/dvplotctrl.cpp
        dview/dvplotctrlsettings.cpp
        dview/dvplothelper.cpp
        dview/dvpncdfctrl.cpp
        dview/dvprofilectrl.cpp
        dview/dvscatterplotctrl.cpp
        dview/dvselectionlist.cpp
        dview/dvstatisticstablectrl.cpp
        dview/dvtimeseriesctrl.cpp
        dview/dvtimeseriesdataset.cpp
        easycurl.cpp
        extgrid.cpp
        exttext.cpp
        exttextstream.cpp
        exttree.cpp
        gleasy.cpp
        jsonreader.cpp
        jsonval.cpp
        jsonwriter.cpp
        label.cpp
        lkscript.cpp
        metro.cpp
        mswfatal.cpp
        pagelayout.cpp
        pageobjects.cpp
        plot/mplcontour.cpp
        plot/plannotation.cpp
        plot/plcontourplot.cpp
        plot/ploutdev.cpp
        plot/plplot.cpp
        plot/plsectorplot.cpp
        plot/pltext.cpp
        plot/plwindrose.cpp
        radiochoice.cpp
        diurnal.cpp
        numeric.cpp
        pdf/pdfannotation.cpp
        pdf/pdfbarcode.cpp
        pdf/pdfcffdecoder.cpp
        pdf/pdfcffindex.cpp
        pdf/pdfcolour.cpp
        pdf/pdfdc.cpp
        pdf/pdfdecode.cpp
        pdf/pdfdocument.cpp
        pdf/pdfencoding.cpp
        pdf/pdfencrypt.cpp
        pdf/pdffont.cpp
        pdf/pdffontdata.cpp
        pdf/pdffontdatacore.cpp
        pdf/pdffontdataopentype.cpp
        pdf/pdffontdatatruetype.cpp
        pdf/pdffontdatatype0.cpp
        pdf/pdffontdatatype1.cpp
        pdf/pdffontdescription.cpp
        pdf/pdffontdetails.cpp
        pdf/pdffontextended.cpp
        pdf/pdffontmanager.cpp
        pdf/pdffontparser.cpp
        pdf/pdffontparsertruetype.cpp
        pdf/pdffontparsertype1.cpp
        pdf/pdffontsubsetcff.cpp
        pdf/pdffontsubsettruetype.cpp
        pdf/pdffontvolt.cpp
        pdf/pdfform.cpp
        pdf/pdfgradient.cpp
        pdf/pdfgraphics.cpp
        pdf/pdfimage.cpp
        pdf/pdfkernel.cpp
        pdf/pdflayer.cpp
        pdf/pdfobjects.cpp
        pdf/pdfocg.cpp
        pdf/pdfparser.cpp
        pdf/pdfpattern.cpp
        pdf/pdfprint.cpp
        pdf/pdfrijndael.cpp
        pdf/pdftemplate.cpp
        pdf/pdfutility.cpp
        pdf/pdfxml.cpp
        plot/plaxis.cpp
        plot/plcolourmap.cpp
        plot/plbarplot.cpp
        plot/plhistplot.cpp
        plot/pllineplot.cpp
        plot/plplotctrl.cpp
        plot/plscatterplot.cpp
        registration.cpp
        snaplay.cpp
        tpdlg.cpp
        uiform.cpp
        utils.cpp
        freetype/src/base/ftbase.c
        freetype/src/base/ftbbox.c
        freetype/src/base/ftbitmap.c
        freetype/src/base/ftfntfmt.c
        freetype/src/base/ftgasp.c
        freetype/src/base/ftglyph.c
        freetype/src/base/ftinit.c
        freetype/src/base/ftstroke.c
        freetype/src/base/ftsynth.c
        freetype/src/base/ftsystem.c
        freetype/src/base/fttype1.c
        freetype/src/cff/cff.c
        freetype/src/cid/type1cid.c
        freetype/src/psaux/psaux.c
        freetype/src/pshinter/pshinter.c
        freetype/src/psnames/psnames.c
        freetype/src/raster/raster.c
        freetype/src/sfnt/sfnt.c
        freetype/src/smooth/smooth.c
        freetype/src/truetype/truetype.c
        freetype/src/type1/type1.c
        freetype/src/base/ftlcdfil.c
        plot/qhull/geom.c
        plot/qhull/geom2.c
        plot/qhull/global.c
        plot/qhull/io.c
        plot/qhull/libqhull.c
        plot/qhull/mem.c
        plot/qhull/merge.c
        plot/qhull/poly.c
        plot/qhull/poly2.c
        plot/qhull/qset.c
        plot/qhull/random.c
        plot/qhull/rboxlib.c
        plot/qhull/stat.c
        plot/qhull/user.c
        plot/qhull/usermem.c
        plot/qhull/userprintf.c
        plot/qhull/userprintf_rbox.c
        )

if (WIN32)
    list(APPEND WEX_SRC
            ole/activex.cpp
            ole/excelauto.cpp
            ole/iehtmlwin.cpp
            )
endif ()


#####################################################################################################################
#
# CMake Targets
#
#####################################################################################################################

add_library(wex STATIC ${WEX_SRC})
set_target_properties(wex
        PROPERTIES
        DEBUG_POSTFIX "d"
        PREFIX ""
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        )
if (MSVC)
    set_target_properties(wex
            PROPERTIES
            LINK_FLAGS /SUBSYSTEM:CONSOLE)
endif ()

if (NOT MSVC)
	file(GLOB DVIEW_SRC_FILES dview/*.cpp)
	file(GLOB QHULL_SRC_FILES plot/qhull/*.c)

	set_source_files_properties(freetype/src/base/ftbase.c
	  PROPERTIES COMPILE_FLAGS -Wno-unused-function)

    if (NOT APPLE)
        set_source_files_properties(${DVIEW_SRC_FILES} ${QHULL_SRC_FILES}
          PROPERTIES COMPILE_FLAGS -Wno-unused-but-set-variable)
    endif()
endif ()

target_include_directories(wex PRIVATE ../include freetype/include $ENV{LKDIR}/include ${CURL_DIR}/include)


#####################################################################################################################
#
# Link Libraries and Options
#
#####################################################################################################################

target_link_libraries(wex ${wxWidgets_LIBRARIES})
if (MSVC)
    target_compile_definitions(wex PUBLIC _LIB LK_USE_WXWIDGETS)
endif ()
if (${CMAKE_PROJECT_NAME} STREQUAL export_config)
    target_compile_definitions(wex PUBLIC _export_config_)
endif ()
