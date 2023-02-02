# FAT_in_C_Project
## Anforderungen
Abzugeben ist eine eigene FAT-Implementierung basierend auf dem Projekt aus dem Unterricht.

- ADD File (File in die Partition schreiben; schon im Unterricht realisiert)
- DEL File (File löschen)
- PUT File (File von der Partition auf Dateisystem des Hostcomputers zurückschreiben)
- DIR (Files anzeigen)
- SHOW File (die von einer Datei belegten Blöcke ausgeben)  

Das Programm soll folgendermaßen getestet werden:
1) Viele kleine Dateien auf die Partition schreiben
2) Jede zweite Datei löschen
3) Eine sehr große Datei schreiben und sicherstellen, dass die in (2) entstanden Lücken korrekt genutzt werden 

## Anleitung
1. Erstellen eine File *.bin 
  + `streams -partition {partitionName}.bin`
  + Bsp: `streams -partition C.bin`
  2. Einlesen metadata
  + `streams -stats {partitionName}`
  + Bsp:  `streams -stats C.bin`
  3. Einfügen Datei in {partitionName}
  + `streams -partition {partitionName} -add {userfile}`
  + Bsp:  `streams -partition C.bin -add p1.png`
  4. Löschen ein File
   + `streams -partition {partitionName} -del {userfile}`
 + Bsp:  `streams -partition C.bin -del p1.png`
  5. File von der Partition auf Dateisystem des Hostcomputers zurückschreiben
  + `streams -partition {partitionName} -put {userfile}`
  + Bsp:  `streams -partition C.bin -put p1.png`
  + Schreiben Sie dieselbe Datei mit der Option, sie umzubenennen
  + `streams -partition {partitionName} -put {userfile} {newName}`
  6. Files anzeigen  
  + `streams -partition {partitionName} -dir`
  + Bsp: `streams -partition C.bin -dir`
  7. Anzeigen die Datein in einem File
+ `streams -partition {partitionName} -show {userfile}`
+ Bsp: `streams -partition C.bin -show p1.png`

 
