
PXG F1 Telemetry is a tool to store, review and analyse UDP telemetry data from Codemasters F1 2018.

## Compilation

- Download and install Qt 5.12.0 or higher from www.qt.io
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

## Author
Thibaud Rabillard (trabillard@laposte.net)
