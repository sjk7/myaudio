TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    tddmain.cpp


macx{CONFIG += sdk_no_version_check}
unix{
    QMAKE_CXXFLAGS += -Wpedantic -Wall -Wodr
    CONFIG(release, debug | release):QMAKE_CXXFLAGS += -flto
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/release/ -lrtAudiocpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/debug/ -lrtAudiocpp
else:linux: LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/ -lrtAudiocpp

INCLUDEPATH += $$PWD/../rtAudio
DEPENDPATH += $$PWD/../rtAudio


win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/release/rtAudiocpp.lib
win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug/rtAudiocpp.lib
linux: PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/librtAudiocpp.a

win32-msvc{
    LIBS += -ldsound -lole32 -luser32 -lAdvapi32 -lkernel32
    INCLUDEPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug
    DEPENDPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug
}

win32:!win32-g++:CONFIG(debug, debug|release):INCLUDEPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug: DEPENDPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug
win32:!win32-g++:CONFIG(release, debug|release):INCLUDEPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/release: DEPENDPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/release
win32-g++{
    # note: I cannot get mingw to link to COM, so it's not supported!
    LIBS +=  -lkernel32 -luuid -lole32 -loleaut32 -ldsound -luser32 -lAdvapi32
}

linux{
    LIBS += -ljack -lpthread -lasound -lpulse -lpulse-simple

}
macx{
    LIBS += -framework CoreAudio -framework CoreFoundation
}

win32-msvc:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/release/ -lrtAudiocpp
else:win32-msvc:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug/ -lrtAudiocpp

win32-g++:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MinGW_64_bit-Debugelease -lrtAudiocpp
else:win32-g++:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/debug -lrtAudiocpp

macx:CONFIG(debug, debug|release) {
LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Debug/ -lrtAudiocpp
INCLUDEPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Debug
DEPENDPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Debug
PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Debug/librtAudiocpp.a
}
macx:CONFIG(release, debug|release){
LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Release/ -lrtAudiocpp
INCLUDEPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Release
DEPENDPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Release
PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_clang_64bit-Release/librtAudiocpp.a
}
