# BYG Text to Speech Coverage


## Compilation

* If there are errors with "atlbase.h" not being found:
	* Make sure that ATL is installed through Visual Studio installer
	* Check the include path in `BYGTextToSpeech.Build.cs` is pointing to
	  a valid location where `atlbase.h` is installed

## Limitations

* Windows only.
* Requires users to have voice packs installed for the language they wish to
  use.
* 



## License

* [3-clause BSD license](LICENSE)
* Based on [yar3333/text-to-speech-ue4](https://github.com/yar3333/text-to-speech-ue4), but with Project Settings and easier wrappers for the Windows language API
* Uses [indomitusgames/FMRTTSLib](https://github.com/indomitusgames/FMRTTSLib)

## Future Work

* Async/threaded calls for large amounts of text

## Contact

* Created and maintained by [@_benui](https://twitter.com/_benui) at [Brace Yourself Games](https://braceyourselfgames.com/)
* Please report bugs through [GitHub](https://github.com/BraceYourselfGames/UE4-BYGTextToSpeech/issues)

