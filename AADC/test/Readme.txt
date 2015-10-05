Dieser Ordner enthält Tests, mit hilfe deren die Funktionalität des Aktor-, Sensor- und Communication Filter abgeprüft werden kann.
Dabei sind die Tests in Modul Tests und Integration Tests aufgeteilt.
Modul Test testen jeweils einen einzelnen Filter. Der Integration Test testet aktors-communication-sensors im verbund.
Die Sourcen dazu befinden sich in den gleichnamigen Ordnern.
Um einen Test auszuführen kann man die Datei do_test.sh ausführen.
Dadurch werden alle Tests nacheinander ausgeführt.
Möchte mann nur einen einzelnen Test ausführen, so gibt man im Konsolenfenster vorher den folgenden Befehl ein:
export TEST_PATTERN=(hier kommt der Name des Tests)
Testnamen:
- tester_aadc_aktors
- tester_aadc_arduino_comm
- tester_aadc_sensors
- tester_aadc_aktors_comm_sensors


