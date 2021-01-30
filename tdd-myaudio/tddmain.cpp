#include "../include/myaudio.hpp"
#include <algorithm> // all_of
#include <chrono>
#include <iostream>
#include <set>
#include <thread>
using namespace std::chrono_literals;

using namespace std;

void test_opening_output_stream()
{
    audio::myaudio all_audio;
    auto &all_enum = all_audio.enumerator();
    auto &first_api = all_enum.apis().at(0);
    const auto api = first_api;

    audio::myaudio audio_api(api);
    auto default_output_device = audio_api.DefaultOutputDevice();
    assert(default_output_device);
    auto instance = audio::DeviceInstance(*default_output_device);
    audio::Stream stream;
    stream.OpenAndRun(&audio_api, &instance, 44100, audio::Direction::output,
                      []() { return 0; });
}

void test_creating_devices()
{
    audio::myaudio audio_all;
    bool tests_hit[3] = {};
    std::string errstr;
    try
    {
        audio_all.openRtApi(RtAudio::Api::WINDOWS_DS);
    }
    catch (const std::exception &e)
    {
        errstr = e.what();
    }
    assert(!errstr.empty() && "Expected openRtApi to always fail");

    for (const auto &api : audio_all.enumerator().apis())
    {
        int c = 0;

        for (const auto &sdr : api.systemDevices())
        {
            const auto &sd = sdr.get();
            auto myDevice = audio::DeviceInstance(sd);
            assert(myDevice.systemDevice().isValid());
            const auto &inParms =
                myDevice.streamParameters(audio::Direction::input);
            const auto &outParms =
                myDevice.streamParameters(audio::Direction::output);

            if (myDevice.systemDevice().deviceType() ==
                audio::DeviceTypes::input)
            {

                assert((int)inParms.deviceId == myDevice.DeviceId());
                assert(inParms.isValid());
                assert(!outParms.isValid());
                tests_hit[0] = true;
            }
            else if (myDevice.systemDevice().deviceType() ==
                     audio::DeviceTypes::output)
            {
                assert((int)outParms.deviceId == myDevice.DeviceId());
                assert(outParms.isValid());
                assert(!inParms.isValid());
                tests_hit[1] = true;
            }
            else if (myDevice.systemDevice().isDuplex())
            {
                assert((int)outParms.deviceId == myDevice.DeviceId());
                assert((int)inParms.deviceId == myDevice.DeviceId());
                assert(outParms.isValid());
                assert(inParms.isValid());
                tests_hit[2] = true;
            }
            assert(myDevice.hostApi() == &api);
            assert(myDevice.DeviceId() == c);
            ++c;
        }
    }

    assert(std::all_of(std::begin(tests_hit), std::end(tests_hit),
                       [](auto &test) { return test; }) &&
           "Not all test scenarios for test_creating_devices hit");
}

void test_non_existent(const audio::DeviceEnumerator &e)
{
    // sanity:
    auto found =
        e.findDevice(e.apis().at(0), e.systemDevices().at(0).info.name);
    assert(found != nullptr);
    auto ptr = e.apiFromDisplayName("xyx");
    assert(ptr == nullptr);
    ptr = e.apiFromName("abc");
    assert(ptr == 0);
    const auto pd = e.findDevice("meh", "moo");
    assert(pd == nullptr);
    auto pdd = e.findDevice(e.apis().at(0), "na na na");
    assert(pdd == nullptr);
}

void create_specific_audio(const audio::HostApi &api)
{
    audio::myaudio audio(api);
    auto &e = audio.enumerator();
    cout << "******************************************************************"
            "*********\n";
    std::cerr << "Enumerating devices ONLY in hostApi: " << api.displayName
              << std::endl;
    cout << "Using api: " << api.displayName << " There are "
         << e.systemDevices().size() << " devices" << endl;
    for (const auto &d : e.systemDevices())
    {
        assert(d.Host_Api() == audio.currentApi());
    }
    cout << "******************************************************************"
            "*********\n";
    cout << "******************************************************************"
            "*********\n";
}

int main()
{
    RtAudio rtaudio;
    cout << "RtAudio detail: " << RtAudio::getVersion() << endl << endl;

    audio::myaudio audio;
    auto &the_enumerator = audio.enumerator();
    cout << "There are a total of: " << the_enumerator.apis().size()
         << " host apis compiled for this system" << endl
         << endl;

    for (const auto &api : the_enumerator.apis())
    {
        cout << "Have api, with name: " << api.name
             << ", and display name: " << api.displayName << endl;
        cout << "This api has " << api.systemDevices().size()
             << " active devices:" << endl;

        for (const auto &d : api.systemDevices())
        {
            cout << "***************************" << endl;

            std::string s{audio::deviceToString(d)};
            const auto &dev = d.get();
            assert(dev.isValid());
            const auto dbak =
                the_enumerator.findDevice(api.displayName, d.get().info.name);
            assert(dbak && dbak->info.name == dev.info.name);
            assert(dev.Host_Api() == &api);
            assert(dev.Host_Api()->displayName == api.displayName);
            assert(dev.deviceInfo().name == dbak->deviceInfo().name);
            assert(dev.DeviceId() >= 0);
            cout << "This device's global device id is: " << dev.DeviceId()
                 << endl;
            cout << s << endl;
        }
        cout << endl << "---------------------------------" << endl;
        const auto found_api =
            the_enumerator.apiFromDisplayName(api.displayName);
        assert(found_api && found_api->displayName == api.displayName);
        const auto found_api2 = the_enumerator.apiFromName(api.name);
        assert(found_api2 && found_api2->name == api.name);
    }

    cout << "Api and device enumeration 'complet'" << endl;

    cout << "Total Devices on system "
         << audio.enumerator().systemDevices().size() << endl;
    for (const auto &d : audio.enumerator().systemDevices())
    {
        assert(the_enumerator.findHostApi(d.rtapi) != nullptr);
    }

    cout << "Duplex devices on system: "
         << audio.enumerator().duplexDevices().size() << endl;
    for (const auto &d : audio.enumerator().duplexDevices())
    {
        cout << audio::deviceToString(d) << endl;
    }
    cout << "---------------------------------------" << endl << endl;

    cout << "Input devices on system: "
         << audio.enumerator().inputDevices().size() << endl;
    for (const auto &d : audio.enumerator().inputDevices())
    {
        cout << audio::deviceToString(d) << endl;
    }
    cout << "---------------------------------------" << endl << endl;

    cout << "Output devices on system: "
         << audio.enumerator().outputDevices().size() << endl;
    for (const auto &d : audio.enumerator().outputDevices())
    {
        cout << audio::deviceToString(d) << endl;
    }
    cout << "---------------------------------------" << endl << endl;

    for (const auto &api : the_enumerator.apis())
        create_specific_audio(api);
    cout.flush();

    auto &e = the_enumerator;
    test_non_existent(e);
    test_creating_devices();

    cout << flush;

    test_opening_output_stream();

#ifdef MAC
    int i = 0;
    while (i++ < 1000)
    {
        std::this_thread::sleep_for(1000000ns);
        std::cerr << "\r";
    }
#endif

    return 0;
}
