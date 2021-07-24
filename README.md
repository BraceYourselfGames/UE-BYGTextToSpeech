# BYG Text to Speech

A plugin for Unreal Engine 4 that supports text-to-speech on Windows, using
Windows SAPI.


## Features

* Automatically read text under the cursor using UE4's built-in `GetAccessibleText()`.
* Blueprint functions for creating USoundWave instances from text.
* Settings for voice, speed, volume.
* Detects all installed voices. 


## Installation

1. Download the zip or clone the repository to `ProjectName/Plugins/BYGTextToSpeech`.
2. Add `BYGTextToSpeech` to `.uproject` file
3. Add `BYGTextToSpeech` to `PrivateDependencyModuleNames` inside `ProjectName.Build.cs`.
4. Install ATL dependencies through Visual Studio. Run the Visual Studio 2019
   installer and under Individual Modules make sure that "C++ ATL for latest
   v142 build tools (x86 & x64) is checked".


## Limitations

* Windows only.
* Requires users to have voice packs installed for the language they wish to
  use.


## Unreal Version Support

* Checked with 4.25 and 4.26.
* Compiles with 5.0EA but not tested, sorry!


## License

* [3-clause BSD license](LICENSE)
* Based on [yar3333/text-to-speech-ue4](https://github.com/yar3333/text-to-speech-ue4) and [indomitusgames/FMRTTSLib](https://github.com/indomitusgames/FMRTTSLib)


## Contact

* Created and maintained by [@_benui](https://twitter.com/_benui) at [Brace Yourself Games](https://braceyourselfgames.com/)
* Please report bugs through [GitHub](https://github.com/BraceYourselfGames/UE4-BYGTextToSpeech/issues)


## Future Work

* Async/threaded calls for large amounts of text.
* Limit voice choice by the player's current locale.

