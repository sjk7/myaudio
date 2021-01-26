TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    tddmain.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Release/release/ -lrtAudiocpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/debug/ -lrtAudiocpp
else:unix: LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/ -lrtAudiocpp

INCLUDEPATH += $$PWD/../rtAudio
DEPENDPATH += $$PWD/../rtAudio

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/release/librtAudiocpp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/debug/librtAudiocpp.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/release/rtAudiocpp.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/debug/rtAudiocpp.lib
else:unix: PRE_TARGETDEPS += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_GCC_64bit-Debug/librtAudiocpp.a

win32-msvc{
    LIBS += -ldsound -lole32 -luser32
}

linux{
    LIBS += -ljack -lpthread -lasound -lpulse -lpulse-simple
}

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/release/ -lrtAudiocpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug/ -lrtAudiocpp

INCLUDEPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug
DEPENDPATH += $$PWD/../build-rtAudioLib-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/debug
