Dies ist der Einstiegspunkt für die ADTF-Filterentwicklung. Die Ordnerstruktur baut sich wie folgt auf.

Folders:
* _build_user: wird bei Verwendung des Buildsktipts (build_user.sh) angelegt und enthält die temporären Buildartefakte.
* _build_base: wird bei Verwendung des Buildsktipts (build_base.sh) angelegt und enthält die temporären Buildartefakte.
* adtfUser: Enthält unter demo Beispielfilter. In diesem Ordner sollten alle Entwicklungen vom User gemacht werdeb.
* adtfBase: Enthält die Sourcen der vorkompilierten Basismodule zur Ansteuerung des AADC-Fahrzeugs. Änderungen sind lokal möglich, werden aber zum Wettbewerb auf einen einheitlichen Stand gebracht. Siehe hierzu auch die Wettbewerbsregeln.

Files:
* build_user.sh: baut die im Verzeichnis adtfUser befindlichen Sourcen mit Hilfe von CMake.
* build_base.sh: baut die im Verzeichnis adtfBase befindlichen Sourcen mit Hilfe von CMake.