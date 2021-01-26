CONFIG -= qt
CONFIG += staticlib # uncomment me if you want a shared library (a dll on 'doze)
TEMPLATE = lib

unix{
    #QMAKE_CXXFLAGS += -Wpedantic
    # TODO: Fix VLA in Jack integration code. Then enable -Wpedantic
}

linux{
    # linux requires these things to be installed on command-line, thusly:
    # sudo apt-get install libpulse-dev libasound-dev libjack-jackd2-dev
    DEFINES += __LINUX_ALSA__ \
                __LINUX_PULSE__ \
                __UNIX_JACK__ \
                __RTAUDIO_DUMMY__

}

win32{
    # NOTE: I suggest using MSVC compiler inside QtCreator on 'Doze. Not Mingw.
    DEFINES += __WINDOWS_DS__ \
                __RT_DUMMY__
                __WINDOWS_WASAPI__ \
                #__WINDOWS_ASIO__
                # uncomment the above lines if you have the Steinberg SDK and _really_ want ASIO
                # (remember, it has SEVERE limitations, like you can only open ONE ASIO device
                # driver at a time) -- do you really need it? WASAPI is now just about as good.
                # And the below line, too. Remember to put the asio SDK files in the right place!
                # INCLUDEPATH += "../asio"
}
macos{
    DEFINES += __MACOSX_CORE__ \
                __RT_DUMMY__
}
CONFIG += c++17

SOURCES += \
    ../rtAudio/RtAudio.cpp

HEADERS += \
    ../include/myaudio.hpp \
    ../rtAudio/RtAudio.h

TARGET = rtAudiocpp

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target


