CONFIG -= qt
CONFIG += staticlib # comment me out if you want a shared library (a dll on 'doze)
TEMPLATE = lib
CONFIG += c++17
QMAKE_CFLAGS += -std=c99

unix{
    QMAKE_CXXFLAGS += -Wpedantic -Wall -Wodr #-Wlifetime
}

linux{
    # linux requires these things to be installed on command-line, thusly:
    # sudo apt-get install libpulse-dev libasound-dev libjack-jackd2-dev
    DEFINES += __LINUX_ALSA__ \
                __LINUX_PULSE__ \
                __UNIX_JACK__ \
                __RTAUDIO_DUMMY__

}

contains(QT_ARCH, i386) {
    #message("32-bit")
} else {
    #message("64-bit")
}



win32{
   INCLUDEPATH += ../rtAudio/asio
   DEFINES -= UNICODE
   LIBS += -lole32 -loleaut32

DEFINES += __WINDOWS_DS__ \
            __RT_DUMMY__ \
            __WINDOWS_WASAPI__ \
            __WINDOWS_ASIO__
   # NOTE: I suggest using MSVC compiler inside QtCreator on 'Doze. Not Mingw.

                #__WINDOWS_ASIO__
                # uncomment the above lines if you have the Steinberg SDK and _really_ want ASIO
                # (remember, it has SEVERE limitations, like you can only open ONE ASIO device
                # driver at a time) -- do you really need it? WASAPI is now just about as good.
                # And the below line, too. Remember to put the asio SDK files in the right place!
                # INCLUDEPATH += "../asio"

}
macx{
    DEFINES += __MACOSX_CORE__  __RTAUDIO_DUMMY__

    CONFIG += sdk_no_version_check
}


SOURCES += \
    ../rtAudio/RtAudio.cpp
win32{
SOURCES += ../rtAudio/asio/asio.cpp \
            ../rtAudio/asio/asiodrivers.cpp \
            ../rtAudio/asio/asiolist.cpp \
            ../rtAudio/asio/iasiothiscallresolver.cpp
}


HEADERS += \
    ../include/myaudio.hpp
    win32{
    SOURCES += ../rtAudio/RtAudio.h \
    ../rtAudio/asio/asio.h \
    ../rtAudio/asio/asiodrivers.h \
    ../rtAudio/asio/asiolist.h \
    ../rtAudio/asio/asiosys.h \
    ../rtAudio/asio/ginclude.h \
    ../rtAudio/asio/iasiodrv.h \
    ../rtAudio/asio/iasiothiscallresolver.h
}

TARGET = rtAudiocpp

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target



