#include "../include/myaudio.hpp"
#include <iostream>

using namespace std;

int main() {
  RtAudio rtaudio;

  audio::myaudio audio;
  auto the_enumerator = audio.enumerator();
  cout << "There are a total of: " << the_enumerator.apis().size()
       << " host apis compiled for this system" << endl
       << endl;
  for (const auto &api : the_enumerator.apis()) {
    cout << "Have api, with name: " << api.name
         << ", and display name: " << api.displayName << endl;
    cout << "This api has " << api.systemDevices.size() << " active devices"
         << endl;
    for (const auto &d : api.systemDevices) {
      cout << d.info.name << endl;
    }
    cout << endl << "----------------------------" << endl;
  }

  return 0;
}
