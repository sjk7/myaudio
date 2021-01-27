#pragma once
#include "../rtAudio/RtAudio.h"
#include <array>
#include <cassert>
#include <functional> // std::reference_wrapper
#include <vector>

namespace audio
{

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

[[maybe_unused]] static inline DeviceTypes
deviceTypeFromInfo(const RtAudio::DeviceInfo &info)
{
    if (info.duplexChannels > 0) return DeviceTypes::duplex;
    if (info.outputChannels > 0) return DeviceTypes::output;
    if (info.inputChannels > 0) return DeviceTypes::input;
    assert("unexpected deviceTypeFromInfo" == nullptr);
    return DeviceTypes::unknown;
}

struct HostApi;
struct DeviceEnumerator;
struct SystemDevice
{
    friend struct DeviceEnumerator;

    SystemDevice(const RtAudio::DeviceInfo &info, const RtAudio::Api api)
        : info(info), rtapi(api), m_DeviceType(deviceTypeFromInfo(info))
    {
        // puts("constructor\n");
    }

    ~SystemDevice()
    {
    }

    const RtAudio::DeviceInfo info;
    const RtAudio::Api rtapi;
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

  private:
    const HostApi *hostApi = nullptr;
};
using SysDevList = std::vector<SystemDevice>;
using SystemDeviceRef = std::reference_wrapper<const SystemDevice>;
using SysDevListRef = std::vector<SystemDeviceRef>;
/*/
     * typedef unsigned long RtAudioFormat;
static const RtAudioFormat RTAUDIO_SINT8 = 0x1;  // 8-bit signed integer.
static const RtAudioFormat RTAUDIO_SINT16 = 0x2; // 16-bit signed integer.
static const RtAudioFormat RTAUDIO_SINT24 = 0x4; // 24-bit signed integer.
static const RtAudioFormat RTAUDIO_SINT32 = 0x8; // 32-bit signed integer.
static const RtAudioFormat RTAUDIO_FLOAT32 =
    0x10; // Normalized between plus/minus 1.0.
static const RtAudioFormat RTAUDIO_FLOAT64 =
    0x20; // Normalized between plus/minus 1.0.
/*/
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
    const RtAudio::Api api;
    const std::string name;
    const std::string displayName;
    SysDevListRef systemDevices;
};

struct DeviceInstance
{
  private:
    const SystemDevice &m_systemDevice;

  public:
    DeviceInstance(const SystemDevice &sd) : m_systemDevice(sd) {}
    const SystemDevice &systemDevice() const noexcept { return m_systemDevice; }
};

using ApiDevList = std::vector<DeviceInstance>;
using ApiList = std::vector<HostApi>;

struct DeviceEnumerator : public no_copy<DeviceEnumerator>
{
  public:
  private:
    SysDevList m_sysDevs;
    SysDevListRef m_sysDevsOutputOnly;
    SysDevListRef m_sysDevsInputOnly;
    SysDevListRef m_sysDevsDuplexOnly;
    ApiDevList m_apiDevs;
    ApiList m_apis;

    const HostApi *findHostApi(RtAudio::Api rtapi)
    {
        auto ptr = std::find_if(m_apis.begin(), m_apis.end(),
                                [&](const auto &a) { return a.api == rtapi; });
        assert(ptr != m_apis.end());
        if (ptr == m_apis.end()) return nullptr;
        return &(*ptr);
    }

    void clear()
    {
        m_sysDevs.clear();
        m_apis.clear();
        m_apiDevs.clear();
        m_sysDevsOutputOnly.clear();
        m_sysDevsInputOnly.clear();
        m_sysDevsDuplexOnly.clear();
    }

    void enum_apis()
    {
        clear();
        using rtapiList = std::vector<RtAudio::Api>;
        rtapiList myapiList;
        RtAudio::getCompiledApi(myapiList);
        std::string sname;
        static constexpr size_t TOTAL_MAX_DEVICES = 150;
        // must reserve, else references to devices will be invalided.
        const auto dev_size = sizeof(SystemDevice);
        const auto mem_used_k = (dev_size * TOTAL_MAX_DEVICES) / 1024;
        (void)mem_used_k;

        m_sysDevs.reserve(TOTAL_MAX_DEVICES);
        try
        {
            for (const auto &a : myapiList)
            {
                sname = RtAudio::getApiDisplayName(a);
                RtAudio rt(a);
                SysDevListRef refs;
                SysDevList &sysDevs = this->m_sysDevs;
                for (auto i = 0u; i < rt.getDeviceCount(); ++i)
                {
                    const auto &info = rt.getDeviceInfo(i);
                    const auto &dsys =
                        sysDevs.emplace_back(SystemDevice{info, a});
                    if (sysDevs.size() >= TOTAL_MAX_DEVICES)
                    {
                        std::cerr << "Too Many Devices!" << std::endl;
                        std::terminate();
                    }
                    refs.push_back(dsys);
                    if (info.duplexChannels > 0)
                        m_sysDevsDuplexOnly.push_back(dsys);
                    else if (info.outputChannels > 0)
                        m_sysDevsInputOnly.push_back(dsys);
                }; // devices

                m_apis.emplace_back(
                    HostApi{a, RtAudio::getApiName(a), sname, refs});
            } // apis

            // back-pointers to rich api:
            for (auto &d : this->m_sysDevs)
            {
                auto *hostapi = findHostApi(d.rtapi);
                assert(hostapi);
                d.hostApi = hostapi;
            }
        }
        catch (std::runtime_error &e)
        {
            std::cerr << "Failed to instantiate an instance of the backend "
                         "using hostApi: "
                      << sname << ", with error: " << e.what() << std::endl;
        }
    }
    void enum_all()
    {
        clear();
        enum_apis();
    }

  public:
    DeviceEnumerator() { enum_all(); }
    const ApiList &apis() const noexcept { return m_apis; }
    const SysDevList systemDevices() const noexcept { return m_sysDevs; }
};

class myaudio : public RtAudio
{
    DeviceEnumerator m_enum;
    std::string m_sid;

  public:
    myaudio(std::string_view id = "") : m_sid(id) {}
    DeviceEnumerator &enumerator() { return m_enum; }
    std::string_view id() const noexcept { return m_sid; }
};

namespace tests
{
}
} // namespace audio
