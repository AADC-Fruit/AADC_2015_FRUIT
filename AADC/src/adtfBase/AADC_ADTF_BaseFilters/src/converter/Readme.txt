In diesem Ordner sind die Sourcen fuer die Converter enthalten

Folders:
* AADC_WheelRpmConverter:	Mit diesem Filter k�nnen die Raddrehzahlen der R�der berechnet werden. Dabei wird ein Sliding-Window-Filter verwendet der aus den �bergebenen Z�hlerwerten die Raddrehzahlwerte berechnet.
* AADC_AttitudeConverter:	Dieser Filter konvertiert die Lage des Fahrzeugs von Quaternionen zu Euler Winkel.
Es existieren 24 verschiedene Euler Systeme. Momentan wird nur Yaw Pitch� Roll�� unterst�tzt


Files:
* CMakeLists.txt:	Datei der CMake kette
* Readme.txt:		Diese Datei