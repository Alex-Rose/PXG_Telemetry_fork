
PXG F1 Telemetry is a tool to record, review and analyse UDP telemetry data from Codemasters F1 2019. 
It was originally written by Thibaud Rabillard and the original repo is available at 
[https://bitbucket.org/Fiingon/pxg-f1-telemetry/src/master/](https://bitbucket.org/Fiingon/pxg-f1-telemetry/src/master/).

## Readme

Original [readme](README.original.md).

## License
This software is licensed under [GPLv3](http://www.gnu.org/licenses/gpl-3.0.html).

### Qt
This software uses Qt under [GPLv3](license.txt) license. More information about using
Qt for open source at [https://www.qt.io/download-open-source#obligations](https://www.qt.io/download-open-source#obligations).

## Compilation

- Download and install the open source version of Qt 5.14.1 or higher from www.qt.io
- Open "F1Telemetry.pro" with Qt creator and compile the project

## Deployment

### macOS
- Add a custom build step in Qt Creator:
 - Command: $QTDIR/bin/macdeployqt
 - Arguments: src/PXG\ F1\ Telemetry.app 
  
### Windows
- Add a custom build step in Qt Creator:
 - Command: %QTDIR%/bin/windeployqt.exe
 - Arguments: "src\release\PXG F1 Telemetry.exe" --dir package
- Copy "PXG F1 Telemetry.exe" in the package directory
- Install and copy the OpenSSL dll inside the package directory

## Original author
Thibaud Rabillard
