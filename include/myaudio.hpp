#pragma once
#define _USE_MATH_DEFINES
#include "../rtAudio/RtAudio.h"
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <functional> // std::reference_wrapper
#include <thread>
#include <vector>

namespace audio
{

namespace dsp
{
#ifdef _MSC_VER
#define PA_FORCE_INLINE __forceinline
#else
#define PA_FORCE_INLINE __attribute__((always_inline))
#endif
static inline float next_sine_sample(unsigned int &sample_num,
                                     unsigned int samplerate,
                                     unsigned int freq = 440)
{
    if (sample_num >= samplerate) sample_num = 0;

    return (float)sin(freq * 2 * M_PI * sample_num++ / samplerate);
}

template <typename T> class fader
{
    T m_destValue;
    float m_secToDest;
    std::atomic<int> m_steps;
    float m_samplerate;
    float m_startValue = 0;
    float m_step = 0;
    float m_vol = 0;
    int m_nch = 2;
    void calc()
    {
        if (m_secToDest <= 0 || m_samplerate <= 0) return;
        const float fsteps = (m_secToDest * m_samplerate) * m_nch;
        m_steps = (int)(fsteps + 0.5f);
        m_step = (m_destValue - m_startValue) / m_steps;
        m_vol = m_startValue;
    }

  public:
    fader() : m_destValue(0), m_secToDest(0), m_steps(0), m_samplerate(0) {}
    fader(T destValue, float secToDest, float samplerate, int nch = 2)
        : m_destValue(destValue), m_secToDest(secToDest), m_steps(0),
          m_samplerate(samplerate), m_nch(nch), m_vol(0)
    {
        calc();
    }

    T volume() const noexcept { return m_vol; }
    void arm(T destval, float samplerate, float secToDest)
    {
        m_startValue = m_vol;
        m_destValue = destval;
        m_samplerate = samplerate;
        m_secToDest = secToDest;
        calc();
    }

    PA_FORCE_INLINE void processSample(T &sample)
    {
        if (m_steps > 0)
        {
            sample *= m_vol;
            m_vol += m_step;
            if (m_vol < 0) m_vol = 0;
            --m_steps;
            if (m_steps <= 0)
            {
                m_vol = this->m_destValue;
                return;
            }
        }
    }

    void processSamples(int nFrames, T *samples, const int nch)
    {
        while (nFrames > 0)
        {
            if (active())
            {
                for (auto ch = 0; ch < nch; ++ch)
                {
                    T &val = *samples++;
                    processSample(val);
                }
                nFrames--;
            }
            else
            {
                if (!is_almost_equal(1.0f, m_destValue))
                {
                    for (auto ch = 0; ch < nch; ++ch)
                    {
                        *samples = *samples * m_vol;
                        ++samples;
                    }
                    nFrames--;
                }
                else
                {
                    return; // nothing to do here: just multiply by one.
                }
            }
        };
    }

    bool active() const noexcept { return m_steps > 0; }
};
} // namespace dsp

template <typename T> struct no_copy
{
    no_copy() = default;
    no_copy(const no_copy &rhs) = delete;
    no_copy &operator=(const no_copy &rhs) = delete;
};
enum class DeviceTypes : unsigned int
{
    unknown = 0,
    input = 1,
    output = 2,
    duplex = input | output
};

[[maybe_unused]] static inline std::string
deviceTypeToString(const DeviceTypes dt)
{
    static std::array<std::string, 4> strings{"unknown", "input", "output",
                                              "duplex"};
    if (dt == DeviceTypes::duplex) return "duplex";
    const auto idt = static_cast<unsigned int>(dt);
    assert(idt > 0 && idt < strings.size() &&
           "bad index in deviceTypeToString" != nullptr);
    return strings.at(idt);
}

[[maybe_unused]] static inline bool
isDeviceType(const DeviceTypes wanted, const DeviceTypes check_for) noexcept
{
    unsigned int uwanted = static_cast<unsigned int>(wanted);
    unsigned int ucheck = static_cast<unsigned int>(check_for);
    return (uwanted & ucheck) != 0;
}

using DeviceInfo = RtAudio::DeviceInfo;

[[maybe_unused]] static inline DeviceTypes
deviceTypeFromInfo(const DeviceInfo &info)
{
    if (info.duplexChannels > 0) return DeviceTypes::duplex;
    if (info.outputChannels > 0) return DeviceTypes::output;
    if (info.inputChannels > 0) return DeviceTypes::input;
    assert("unexpected deviceTypeFromInfo" == nullptr);
    return DeviceTypes::unknown;
}

struct HostApi;
struct DeviceEnumerator;
using Api = RtAudio::Api;
using StreamOptions = RtAudio::StreamOptions;
using StreamParameters = RtAudio::StreamParameters;

struct SystemDevice
{

    friend struct DeviceEnumerator;

    SystemDevice(const int deviceId, const DeviceInfo &info, const Api api)
        : info(info), rtapi(api), m_DeviceType(deviceTypeFromInfo(info)),
          m_DeviceId(deviceId)
    {
    }

    ~SystemDevice() {}
    const DeviceInfo &deviceInfo() const noexcept { return info; }

    const DeviceInfo info;
    const Api rtapi;
    const DeviceTypes m_DeviceType = DeviceTypes::unknown;
    DeviceTypes deviceType() const noexcept { return m_DeviceType; }

    bool isInput() const noexcept
    {
        return isDeviceType(DeviceTypes::input, m_DeviceType);
    }
    bool isOutput() const noexcept
    {
        return isDeviceType(DeviceTypes::output, m_DeviceType);
    }
    bool isDuplex() const noexcept
    {
        return isDeviceType(DeviceTypes::duplex, m_DeviceType);
    }
    const HostApi *Host_Api() const noexcept { return hostApi; }
    // corresponds to index in RtAudio::getDeviceCount()
    int DeviceId() const noexcept { return m_DeviceId; }
    bool isValid() const noexcept
    {
        return m_DeviceId > StreamParameters::BAD_DEVICE_ID;
    }

  private:
    // corresponds to index in RtAudio::getDeviceCount()
    int m_DeviceId = StreamParameters::BAD_DEVICE_ID;
    const HostApi *hostApi = nullptr;
};

using SysDevList = std::vector<SystemDevice>;
using SystemDeviceRef = std::reference_wrapper<const SystemDevice>;
using SysDevListRef = std::vector<SystemDeviceRef>;

enum class AudioFormat : RtAudioFormat
{
    SINT8 = 0x1,
    SINT16 = 2,
    SINT24 = 0x4,
    FLOAT32 = 0x10,
    FLOAT64 = 0x20
};

static inline unsigned long AudioFormatToBits(const AudioFormat &fmt)
{
    switch (fmt)
    {
    case AudioFormat::SINT8:
        return 8;
    case AudioFormat::SINT16:
        return 16;
    case AudioFormat::SINT24:
        return 24;
    case AudioFormat::FLOAT32:
        return 32;
    case AudioFormat::FLOAT64:
        return 64;
    default:
        return 0;
    }
}

static inline std::string formatsToString(const RtAudioFormat f)
{
    std::string ret;
    if (f & RTAUDIO_SINT8) ret += "Signed 8-bit integer\t";
    if (f & RTAUDIO_SINT16) ret += "Signed 16-bit integer\t";
    if (f & RTAUDIO_SINT24) ret += "Signed 24-bit integer\t";
    if (f & RTAUDIO_SINT32) ret += "Signed 32-bit integer\t";
    if (f & RTAUDIO_FLOAT32) ret += "Signed 32-bit float\t";
    if (f & RTAUDIO_FLOAT64) ret += "Signed 64-bit float\t";
    return ret;
}

[[maybe_unused]] std::string deviceToString(const SystemDevice &d)
{
    std::stringstream ss;
    ss << "Device:" << d.info.name
       << " [type=" << deviceTypeToString(d.deviceType()) << "]"
       << ", using api: " << RtAudio::getApiDisplayName(d.rtapi) << '\n';

    ss << "Preferred samplerate: " << d.info.preferredSampleRate << '\n'
       << "All samplerates: ";
    for (const auto &sr : d.info.sampleRates)
    {
        ss << '|' << sr;
    }

    ss << "\nSupported bit depth formats: ";
    const std::string fmts{formatsToString(d.info.nativeFormats)};
    ss << fmts << std::endl;

    std::string ret{ss.str()};
    ss << '\n' << '\n';
    return ret;
}

struct HostApi
{
    const Api api;
    const std::string name;
    const std::string displayName;
    HostApi(const Api api, std::string_view name, std::string_view displayname,
            SysDevListRef &devices)
        : api(api), name(name), displayName(displayname),
          m_systemDevices(devices)
    {
    }
    const SysDevListRef &systemDevices() const { return m_systemDevices; }

  private:
    SysDevListRef m_systemDevices;
};

[[maybe_unused]] static inline StreamOptions StreamOptionsDefault()
{
    StreamOptions ret = StreamOptions(RTAUDIO_SCHEDULE_REALTIME, 1024, 0);
    return ret;
}
enum class Direction : unsigned int
{
    not_specified = StreamParameters::BAD_DEVICE_IDU,
    input = 0,
    output = 1,
    duplex = input | output
};

/*!
 * \brief StreamParamsDefault: NOTE I can throw std::runtime_error!
 * \param sd
 * \param dir
 * \return
 */
[[maybe_unused]] static inline StreamParameters
StreamParamsDefault(const SystemDevice &sd,
                    const Direction dir = Direction::not_specified)
{
    const auto first_channel = 0;
    auto def_chans = 2;
    auto device_id = sd.DeviceId();
    if (dir == Direction::not_specified)
    {
        if (sd.isDuplex())
        {
            def_chans = (std::min)((int)sd.info.duplexChannels, 2);
        }
        else if (sd.isInput())
        {

            def_chans = (std::min)((int)sd.info.inputChannels, 2);
        }
        else
        {
            def_chans = (std::min)((int)sd.info.outputChannels, 2);
        }
    }
    else
    {
        if (dir == Direction::input)
        {
            if ((int)sd.deviceType() & (int)DeviceTypes::input)
            {
                def_chans = (std::min)((int)sd.info.inputChannels, 2);
            }
            else
            {
                def_chans = 0;
                device_id = StreamParameters::BAD_DEVICE_IDU;
            }
        }
        else
        {
            if ((int)sd.deviceType() & (int)DeviceTypes::output)
            {
                def_chans = (std::min)((int)sd.info.outputChannels, 2);
            }
            else
            {
                def_chans = 0;
                device_id = StreamParameters::BAD_DEVICE_IDU;
            }
        }
    }

    StreamParameters ret(device_id, def_chans, first_channel);
    return ret;
}
struct FormatType
{
    AudioFormat Format = AudioFormat::FLOAT32;
    unsigned int Channels = 2;
    unsigned int SamplesPerSec = 44100;
    unsigned int BitsPerSample() const { return AudioFormatToBits(Format); }
};

[[maybe_unused]] static inline FormatType defaultAudioFormat()
{
    return FormatType{};
}

struct DeviceInstance
{
    DeviceInstance(const SystemDevice &sd,
                   FormatType fmt = defaultAudioFormat(),
                   StreamOptions opts = {}, StreamParameters inParams = {},
                   StreamParameters outParams = {})
        : m_sysDevice(sd), m_StreamOptions(opts), m_format(fmt)

    {
        m_StreamParams[0] = inParams;
        m_StreamParams[1] = outParams;

        for (int i = 0; i < 2; ++i)
        {
            auto &params = m_StreamParams[i];
            Direction dir = static_cast<Direction>(i);
            if (params.nChannels == 0)
            {
                params = StreamParamsDefault(sd, dir);
            }
            if (opts.numberOfBuffers == 0)
            {
                m_StreamOptions = StreamOptionsDefault();
            }
        };
    }
    const SystemDevice &systemDevice() const { return m_sysDevice; }
    // corresponds to index in RtAudio::getDeviceCount()
    int DeviceId() const noexcept { return m_sysDevice.DeviceId(); }
    const HostApi *hostApi() const noexcept
    {
        assert(m_sysDevice.Host_Api());
        return m_sysDevice.Host_Api();
    }
    StreamOptions &streamOptions() noexcept { return m_StreamOptions; }
    StreamParameters &streamParameters(Direction direction) noexcept
    {
        unsigned int index = static_cast<unsigned int>(direction);
        return m_StreamParams[index];
    }
    FormatType &Format()
    {
        m_format.Channels = m_StreamParams[1].nChannels;
        return m_format;
    }

    DeviceInstance(const DeviceInstance &rhs)
        : m_sysDevice(rhs.m_sysDevice), m_StreamOptions(rhs.m_StreamOptions),
          m_format(rhs.m_format)

    {
        m_StreamParams[0] = rhs.m_StreamParams[0];
        m_StreamParams[1] = rhs.m_StreamParams[1];
    }

  private:
    const SystemDevice &m_sysDevice;
    StreamOptions m_StreamOptions = {};
    StreamParameters m_StreamParams[2] = {{}, {}};
    mutable FormatType m_format;
};

using ApiDevList = std::vector<DeviceInstance>;
using ApiList = std::vector<HostApi>;

struct DeviceEnumerator : public no_copy<DeviceEnumerator>
{

  public:
    DeviceEnumerator() { enum_all(); }
    DeviceEnumerator(RtAudio *prt) { enum_apis(prt); }

    // Care! This will invalidate any references that you may have to apis and
    // devices.
    void Refresh() { enum_all(); }
    const ApiList &apis() const noexcept { return m_apis; }
    const SysDevList &systemDevices() const noexcept { return m_sysDevs; }

    const HostApi *apiFromDisplayName(std::string_view name) const noexcept
    {
        return findApiByDisplayName(name);
    }
    const HostApi *apiFromName(std::string_view name) const noexcept
    {
        return this->findApiByName(name);
    }
    const SystemDevice *findDevice(const HostApi &a,
                                   std::string_view deviceName) const noexcept
    {
        for (const auto &d : a.systemDevices())
        {
            const auto &dev = d.get();
            if (dev.deviceInfo().name == deviceName) return &dev;
        }
        return nullptr;
    }
    const SystemDevice *findDevice(std::string_view apiDisplayName,
                                   std::string_view deviceName) const noexcept
    {
        auto api = apiFromDisplayName(apiDisplayName);
        if (!api) return nullptr;
        return findDevice(*api, deviceName);
    }
    const HostApi *findHostApi(Api rtapi)
    {
        auto ptr = std::find_if(m_apis.begin(), m_apis.end(),
                                [&](const auto &a) { return a.api == rtapi; });
        assert(ptr != m_apis.end());
        if (ptr == m_apis.end()) return nullptr;
        return &(*ptr);
    }

    const SysDevListRef &duplexDevices() const { return m_sysDevsDuplex; }
    const SysDevListRef &inputDevices() const { return m_sysDevsInput; }
    const SysDevListRef &outputDevices() const { return m_sysDevsOutput; }

  private:
    // all devices
    SysDevList m_sysDevs;
    SysDevListRef m_sysDevsOutput;
    SysDevListRef m_sysDevsInput;
    SysDevListRef m_sysDevsDuplex;
    ApiList m_apis;
    const HostApi *
    findApiByDisplayName(std::string_view displayName) const noexcept
    {

        for (const auto &api : apis())
        {
            if (api.displayName == displayName)
            {
                return &api;
            }
        }
        return nullptr; // not found
    }
    const HostApi *findApiByName(std::string_view name) const noexcept
    {
        for (const auto &api : apis())
        {
            if (api.name == name)
            {
                return &api;
            }
        }
        return nullptr; // not found
    }

    void clear()
    {
        m_sysDevs.clear();
        m_apis.clear();
        m_sysDevsOutput.clear();
        m_sysDevsInput.clear();
        m_sysDevsDuplex.clear();
    }
    static constexpr size_t TOTAL_MAX_DEVICES = 150;
    SysDevListRef enum_api_devices(RtAudio *prt)
    {
        assert(prt);
        if (!prt)
            throw std::runtime_error(
                "enum_api_devices: pointer to audio object required");
        auto a = prt->getCurrentApi();
        auto sname = RtAudio::getApiDisplayName(a);
        SysDevListRef refs;
        SysDevList &sysDevs = this->m_sysDevs;
        for (auto i = 0u; i < prt->getDeviceCount(); ++i)
        {
            const auto &info = prt->getDeviceInfo(i);

            const auto &dsys = sysDevs.emplace_back(SystemDevice(i, info, a));
            if (sysDevs.size() >= TOTAL_MAX_DEVICES)
            {
                std::cerr << "Too Many Devices!" << std::endl;
                std::terminate();
            }
            refs.push_back(dsys);
            if (info.duplexChannels > 0) m_sysDevsDuplex.push_back(dsys);
            if (info.outputChannels > 0) m_sysDevsOutput.push_back(dsys);
            if (info.inputChannels > 0) m_sysDevsInput.push_back(dsys);

        }; // devices

        return refs;
    }

    void enum_api(RtAudio *rta)
    {
        assert(rta);
        if (!rta)
            throw std::runtime_error("enum_api: no audio object pointer!");
        auto a = rta->getCurrentApi();
        auto refs = enum_api_devices(rta);
        auto api_name = RtAudio::getApiName(a);
        m_apis.emplace_back(
            HostApi{a, api_name, RtAudio::getApiDisplayName(a), refs});
    }

    void enum_apis(RtAudio *prt = nullptr)
    {
        clear();
        using rtapiList = std::vector<Api>;
        rtapiList myapiList;
        RtAudio::getCompiledApi(myapiList);
        std::string api_name;
        // must reserve, else references to devices will be invalided.
        m_sysDevs.reserve(TOTAL_MAX_DEVICES);
        if (prt)
        {
            enum_api(prt);
        }
        else
        {
            try
            {
                for (const auto &a : myapiList)
                {
                    RtAudio rta(a);
                    enum_api(&rta);

                }; // apis
            }
            catch (std::runtime_error &e)
            {
                std::cerr << "Failed to instantiate an instance of the backend "
                             "using hostApi: "
                          << api_name << ", with error: " << e.what()
                          << std::endl;
            }
        }

        // back-pointers to rich api:
        for (auto &d : this->m_sysDevs)
        {
            auto *hostapi = findHostApi(d.rtapi);
            assert(hostapi);
            d.hostApi = hostapi;
        }
    }

    void enum_all()
    {
        clear();
        enum_apis();
    }
};

enum class AudioCallbackStatus : unsigned int
{
    none = 0,
    RTAUDIO_INPUT_OVERFLOW = 0x1, // Input data was discarded because of an
    // overflow condition at the
    // driver.
    RTAUDIO_OUTPUT_UNDERFLOW = 0x2 // The output buffer ran low, likely
    // causing a gap in the output sound.
};
[[maybe_unused]] inline AudioCallbackStatus operator|(AudioCallbackStatus &lhs,
                                                      AudioCallbackStatus &rhs)
{
    unsigned int l = (unsigned int)lhs;
    unsigned int r = (unsigned int)rhs;
    unsigned int ret = l | r;
    return (AudioCallbackStatus)ret;
}
struct StreamCallbackInfo
{
    const void *outputBuffer = nullptr;
    const void *inputBuffer = nullptr;
    unsigned int frames = 0;
    double streamTime = 0.0;
    AudioCallbackStatus status = AudioCallbackStatus::none;
    FormatType format = {};
};

namespace detail
{
static void static_error_callback(RtAudioError::Type type,
                                  const std::string_view s)
{
    std::cerr << "error_callback, with type: " << type << std::endl;
    throw std::runtime_error(s.data());
}

} // namespace detail

// Override this in your own class to accept the callback
struct AudioCallback
{
    virtual int OnAudioCallback(const StreamCallbackInfo &info) = 0;
    FormatType format = {};
};

class Stream
{
  private:
    Stream(DeviceInstance &deviceOut, RtAudio &rta, AudioCallback *cb,
           FormatType &fmt)
        : m_deviceInstance(deviceOut), m_rta(rta), m_pcb(cb), m_format(fmt)

    {
        assert(m_pcb);
        puts("constructor with just an rtAudio instance");
    }
    DeviceInstance &m_deviceInstance;
    RtAudio &m_rta;
    AudioCallback *m_pcb = nullptr;
    FormatType &m_format;

    static int
    static_callback(const void *outputBuffer, const void *inputBuffer,
                    const unsigned int frames, const double streamTime,
                    const RtAudioStreamStatus status, const void *userdata)
    {
        StreamCallbackInfo info{outputBuffer, inputBuffer, frames, streamTime,
                                AudioCallbackStatus(status)};

        auto *pcb = (AudioCallback *)userdata;
        info.format = pcb->format;
        return pcb->OnAudioCallback(std::forward<StreamCallbackInfo>(info));
    };

  public:
    Stream(RtAudio &rta, DeviceInstance &deviceOut, AudioCallback *cb,
           FormatType &fmt)
        : Stream(deviceOut, rta, cb, fmt)
    {
        OpenForOutput();
    }
    Stream(const Stream &rhs)
        : m_deviceInstance(rhs.m_deviceInstance), m_rta(rhs.m_rta),
          m_pcb(rhs.m_pcb), m_format(rhs.m_format),
          m_latencyFrames(rhs.m_latencyFrames)
    {
        puts("copy constructor");
    }

    Stream(Stream &&rhs) = default;

    ~Stream()
    {
        puts("Stream Destructor");
        puts("\n");
    }

    // throws std::runtime_error if problems.
    void Start() { m_rta.startStream(); }
    long GetStreamLatency() const { return m_rta.getStreamLatency(); }
    bool HasCallback() const noexcept { return m_pcb; }

  private:
    int OnAudioCallback(StreamCallbackInfo &&info)
    {
        return m_pcb->OnAudioCallback(std::forward<StreamCallbackInfo>(info));
    }

  public:
    bool isRunning() const { return m_rta.isStreamRunning(); }

    FormatType Format() const { return m_format; }

  private:
    Stream OpenForOutput()
    {
        using namespace std;
        using namespace chrono_literals;
        assert(m_pcb);
        if (!m_pcb)
        {
            throw std::runtime_error(
                "Stream::OpenAndRun: a callback is REQUIRED");
        }

        if (!this->m_deviceInstance.systemDevice().isValid())
        {
            throw std::runtime_error("Stream::OpenForOutput a DeviceInstance "
                                     "is REQUIRED to be valid");
        }

        StreamOptions *opts = nullptr;
        StreamParameters *inParams = nullptr;
        const auto dir = Direction::output;
        StreamParameters *outParams = &m_deviceInstance.streamParameters(dir);
        if (dir == Direction::duplex)
        {
            inParams = &m_deviceInstance.streamParameters(Direction::output);
            if (!inParams || !inParams->isValid())
            {
                throw std::runtime_error("Input parameters must be set and "
                                         "valid for a duplex stream");
            }
        }
        if (!outParams || !outParams->isValid())
        {
            throw std::runtime_error(
                "Output parameters must be set and valid for a duplex stream");
        }

        opts = &m_deviceInstance.streamOptions();
        unsigned int bufferFrames = 1024;

        m_format = m_deviceInstance.Format();

        m_pcb->format =
            m_format; // it's a copy, so its safe to access it from the callback
        unsigned int fmt = (unsigned int)m_format.Format;
        m_rta.openStream(outParams, nullptr, fmt, m_format.SamplesPerSec,
                         &bufferFrames, &static_callback, m_pcb, opts,
                         &detail::static_error_callback);

        m_latencyFrames = m_rta.getStreamLatency();

        this->Start();
        while (isRunning())
        {
            this_thread::sleep_for(10ms);
        }
        return *this;
    }

    long m_latencyFrames = 0;

  protected:
};

class myaudio : public RtAudio, public no_copy<myaudio>
{
    DeviceEnumerator m_enum;
    std::string m_sid;

  public:
    // create an instance of myaudio that can enumerate
    // all Host apis and devices:
    myaudio(std::string_view id = "") : m_enum(), m_sid(id) {}
    // create an instance of myaudio that specifically
    // targets one of the apis found by constructing one of the above.
    myaudio(const HostApi &api, std::string_view id = "")
        : RtAudio(api.api), m_enum(this), m_sid(id)
    {
    }
    virtual ~myaudio() {}

    void openRtApi(RtAudio::Api)
    {
        throw std::runtime_error(
            "note: openRtApi() function is not supported as I am worried about "
            "the"
            "enumerator references becoming invalid. Just create a different "
            "myaudio"
            "instead, and specify the api in the constructor.");
    }
    DeviceEnumerator &enumerator() { return m_enum; }
    std::string_view id() const noexcept { return m_sid; }
    const HostApi *currentApi()
    {
        auto a = m_enum.findHostApi(RtAudio::getCurrentApi());
        return a;
    }
    const SystemDevice *DefaultOutputDevice() const
    {
        unsigned int devId = RtAudio::getDefaultOutputDevice();
        for (const auto &d : m_enum.systemDevices())
        {
            if ((unsigned int)d.DeviceId() == devId && d.isOutput())
            {
                return &d;
            }
        }
        return nullptr;
    }

    const SystemDevice *DefaultInputDevice() const
    {
        unsigned int devId = RtAudio::getDefaultInputDevice();
        for (const auto &d : m_enum.systemDevices())
        {
            if ((unsigned int)d.DeviceId() == devId && d.isInput())
            {
                return &d;
            }
        }
        return nullptr;
    }

    auto OpenStream(DeviceInstance &deviceOut, AudioCallback *cb)
    {
        Stream s(*this, deviceOut, cb, deviceOut.Format());
        return s;
    }
};

namespace dsp
{

[[maybe_unused]] static inline void
fill_buffer_sine(unsigned int &nsample, const audio::StreamCallbackInfo &info,
                 int freq = 440)
{
    float *out = (float *)info.outputBuffer;
    const auto nch = info.format.Channels;
    for (unsigned int i = 0; i < info.frames; ++i)
    {
        auto v = next_sine_sample(nsample, info.format.SamplesPerSec, freq);
        for (unsigned int ch = 0; ch < nch; ++ch)
        {
            *out++ = v;
        }
    }
}
} // namespace dsp

namespace tests
{
}
} // namespace audio
