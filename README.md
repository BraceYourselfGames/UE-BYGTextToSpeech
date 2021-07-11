# BYG Text to Speech

A plugin for Unreal Engine 4 that supports text-to-speech on Windows, using
Windows SAPI.


## Features

* Blueprint functions for creating USoundWave instances from text.
* Automatically read text under the cursor using UE4's built-in `GetAccessibleText()`


## Installation

1. Download the zip or clone the repository to `ProjectName/Plugins/BYGTextToSpeech`.
2. Add `BYGTextToSpeech` to `.uproject` file
3. Add `BYGTextToSpeech` to `PrivateDependencyModuleNames` inside `ProjectName.Build.cs`.
4. Compile and run. If there are errors with "atlbase.h" not being found:
	* Make sure that ATL is installed through Visual Studio installer
	* Check the include path in `BYGTextToSpeech.Build.cs` is pointing to
	  a valid location where `atlbase.h` is installed




## Limitations

* Windows only.
* Requires users to have voice packs installed for the language they wish to
  use.


## Unreal Version Support

* Checked with 4.25 and 4.26
* Compiles from 4.25 to 5.0EA but not tested, sorry!


## License

* [3-clause BSD license](LICENSE)
* Based on [yar3333/text-to-speech-ue4](https://github.com/yar3333/text-to-speech-ue4) and [indomitusgames/FMRTTSLib](https://github.com/indomitusgames/FMRTTSLib)


## Contact

* Created and maintained by [@_benui](https://twitter.com/_benui) at [Brace Yourself Games](https://braceyourselfgames.com/)
* Please report bugs through [GitHub](https://github.com/BraceYourselfGames/UE4-BYGTextToSpeech/issues)

## Future Work

* Async/threaded calls for large amounts of text
