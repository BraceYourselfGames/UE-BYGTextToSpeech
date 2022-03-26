@echo off

if not [%1]==[] goto main
if not [%2]==[] goto main

:usage
@echo Usage: PackagePlugin.bat {PluginName} {UE4 version} [{UE4 dir}] [{Temp Dir}]
exit /B 1 


:main

set plugin_name=%1
set ue4_version=%2
set ue4_path=%3
set temp_dir=%4

if [%3]==[] set ue4_path=C:\Program Files\Epic Games\UE_%ue4_version%
if [%4]==[] set temp_dir=C:\Temp\%plugin_name%-%ue4_version%

@echo Building plugin %plugin_name% for Unreal version %ue4_version%

set command="%ue4_path%\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin ^
-Plugin="%~dp0%plugin_name%.uplugin" ^
-Package="%temp_dir%" ^
-%plugin_name% ^
-VS2019

@echo Running command:
@echo %command%

%command%

goto :eof




exit /B 1
