
PXG F1 Telemetry is a tool to record, review and analyse UDP telemetry data from Codemasters F1 2019.

## Features

F1 Telemetry is able to analyse laps, stints (multiple consecutive laps) and races of every drivers in the session (human or a.i.).

It allows you to :

- Compare your laps with the A.I. or other drivers to improve your driving
- Refine your setups
- Prepare your own practice programmes 
- Analyse what happend during a race

Data of the selected drivers is recorded on the hard drive during the sessions and can be reviewed later. There is no real time display !

![Tracked drivers sélection](Screenshots/tracking.png)

### Analysis available for laps

- Speed
- Throttle input
- Brake input
- Steering input
- Gear
- Time differences
- Tyre wear
- Tyre temperatures
- Tyre degradation
- Suspension
- ERS balance
- Tyres lockup
- Car balance i.e. oversteer / understeer
- Multiple laps comparison

![Comparaison of laps](Screenshots/Laps.png)

### Analysis available for stints

- Lap times
- Average tyre wear per lap
- Max tyre temperature
- Fuel consumption
- ERS energy
- Multiple stints comparison

![Comparaison of stints](Screenshots/Stints.png)

### Analysis available for races

- Race position
- Race time
- Lap times
- Tyre life
- Fuel consumption
- ERS energy
- Weather changes
- Air / track temperature
- Front wing damage
- Pitstop duration
- Penalities received

![Analysis of races](Screenshots/Race.png)


## Compilation

- Download and install Qt 5.14.1 or higher from www.qt.io
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

## Author
Thibaud Rabillard (pxgf1telemetry@laposte.net)
