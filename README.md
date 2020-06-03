# StepTimer
Doorstep Timer for campervans

ATTiny13, C compiled with AVR Studio

Wohnmobil - Automatische Trittstufe

Warum ?
Mein Womo hat eine 2 stufige elektrische Trittstufe. Man kann auch ohne sie einsteigen, aber es ist sehr mühsam. Daher wird sie oft benutzt. Bei Starten des Motors mit ausgefahrener Trittstufe ertönt ein lautes Signal. Dann muß ich aber erstmal den Gurt lösen, nach hinten gehen, die Stufe hochfahren und wieder zurück. Da ich es doch immer wieder mal vergessen habe, wurde mir das zu blöd.

Wie ist die bestehende Schaltung ?
Der Summer ist in der Nähe des EBL befestigt. Sein Plus hängt an einem Verteiler am KEY-ON Signal des Ducato (600mA). Sein Minus liegt an einem grauen Kabel, das unter dem Herd am Boden über eine Klemme an ein weiteres graues Kabel geht, das an einen Kontakt des Endschalters der Stufe geht. Der andere Kontakt des Schalters wird über ein braunes Kabel mit der Masse oben am Bedienschalter der Stufe verbunden. Der Summer ertönt also, wenn der Schlüssel in die erste Position gedreht wird und die Stufe unten ist.

Der Tastschalter der Stufe ist mehr oder weniger ein Polwendeschalter. D.h. im Ruhezustand liegen die Motoranschlüsse beide auf dem gleichen Pol (Minus), also fließt kein Strom. Jede der Tasten verbindet nun einen der Motoranschlüsse mit einem anderen Pol (Plus), sodaß er läuft. Dabei zieht er etwa 5A.
Es gibt keine Endschalter (abgesehen vom oben beschriebenen Schalter, der aber nichts mit dem Motorstrom zu tun hat). Daher wird der Strom am Ende auch nicht unterbrochen, sondern steigt auf den Blockierstrom von etwa 20A an. Die Sicherung im EBL hat 25A.
Relais:
Es gibt eine Automatiklösung, die im wesentlichen aus einem Relais und einem Elko besteht. Das Relais wird statt des Summers betätigt und legt wie der AUF Taster einen Motorpol auf Plus. Wenn der Endschalter betätigt wird, fällt das Relais ab und der Motor steht.

Der Motor soll nach Erreichen des Endschalters noch einen Moment weiterlaufen (Nachlauf), damit die Stufe nicht knapp an der Schaltschwelle stehen bleibt und beim Fahren wieder rausrutscht und wieder Strom bekommt...
Dazu wird das Relais mit dem Elko noch etwas länger eingeschaltet gelassen. Der Elko liegt einfach parallel zur Wicklung.

Ein Schaltungsbeispiel und eine Einbauanleitung gab es mal im Netz, leider funktionieren die Links nicht mehr.

Meine Variante:
Ich habe den Summer abgeklemmt und statt dessen eine Sicherung eingebaut, wodurch ich in der Nähe des Stufenschalters das KEY-ON liegen habe. Das graue Kabel zum Endstellungssensor mußte ich nicht auftrennen, weil es schon unten im Küchenkasten mit Klemmen verbunden war. Dort und am Schalter würde das Automatikrelais eingeschleift. Allerdings gefiel mir die Lösung mit dem Elko nicht besonders. Außerdem wurde immer wieder die Frage gestellt, was passiert, wenn gerade jemand auf der Stufe steht.

Ich hab statt dessen eine etwas intelligentere Elektronik gebaut:
Schaltplan: https://github.com/rkopka42/StepTimer/blob/master/Step02.jpg

Mittels uController kann diese etwas mehr als das einfache Relais. Zum einen hat sie einen Piezo Tongeber, der die unterschiedlichen Phasen mit Tönen anzeigt. Diese Phasen sind:

    2sek erstmal nur Warnton
    Treppe einfahren, maximal 3sek. Motorlaufzeit
    wenn die Treppe den Sensor betätigt 0,5sek Nachlaufzeit (das macht sonst der Elko parallel zum Relais)
    die Stufe kann danach auch bei KEY-ON ausgefahren werden, aber es gibt einen erträglichen Warnton
    falls die Stufe nicht in 3sek einfährt, wird der Motor abgeschaltet und ein eindringlicher Warnton ertönt für 10sek.

Das Relais muß recht groß sein, da der normale Strom schon etwas 5A(auf) beträgt. Am Ende läuft der Motor (ohne Endschalter) in den Anschlag und zieht bis zu 20A !!!

Der Schaltplan entstand mit einem recht einfachen freien Programm und war mein erster Versuch damit. Leider gibt es noch ein paar Fehler. Die Out In Bezeichnungen am Spannungsregler sind verkehrt herum. VREG U 5V ist etwas nach rechts verrutscht und bezeichnet den Spannungsregler. Die antiparallele Diode an der Relaisspule fehlt. Es gab kein Buzzer Symbol, daher steht dort eine Wechselstromquelle. Die ZD 5,6V würde ich eher auf 5,1V ändern.

Ich verwende einen ATTiny13 (die Angabe im Schaltplan stimmt nicht, obwohl ein 45er genauso geht), weil ich dafür ein Entwicklerboard habe und die Chips gut erhältlich sind. Da die Schaltung sehr einfach ist, habe ich alles incl. Relais auf einer Lochrasterplatine mit einer Schraubklemmenleiste untergebracht. Besser wäre es aber, das Relais extern zu halten und für die Anschlüsse 6,3mm Flachstecker vorzusehen.
Ursprünglich war auch ein Programmieranschluß auf der Platine. Allerdings habe ich den dann aus Platzgründen entfernt. Ich mußte auch keine Änderungen am Code vornehmen, der vorher schon getestet war. Der einzige Fehler war ein fehlender Widerstand (Pull-Up) an der Resetleitung.

Probleme:
Leider fiel der Praxistest nicht so gut aus. Die Stufe fuhr nur kurz an, dann war der Strom weg. Ich hatte die schwache KEY-ON Leitung mit dem Stufenstrom belastet. Die 2A Sicherung bei der Karosserieausbaubuchse in der rechten B-Säule hat schlimmeres verhindert. Dummerweise findet man diese Infos nicht in den normalen Unterlagen sondern erst nach längerem Suchen im Netz oder nach Rückfrage beim Hersteller.

Korrekt würde der alternative Stromkreis über eine starkes ACC gehen, damit sich nur bei Motor An etwas tun kann. Da KEY-ON zu schwach ist, nehme ich die normale Stufenversorgung. Man könnte das noch etwas sicherer machen, indem man sie über ein Relais führt, das durch KEY-ON oder D+ gesteuert wird.

Lösung:
Für weitere Basteleien habe ich mir über ein Relais eine stärkere Version des D+/KeyOn Signals verschafft. Trotzdem hängt das Trittstufenrelais jetzt direkt an der Stufenversorgung. Komischerweise war im EBL nur eine 20A Sicherung statt einer 25A (evt. war da schon mal was und die richtige Sicherung war nicht bei der Hand). Die hat bei den ersten Versuchen ausgelöst. Inzwischen mit 25A geht es recht gut.
Die Elektronik ist in einem kleinen Kästchen unter dem Herd eingebaut.

Gedanken
Ich frage mich aber schon, warum die Hersteller so ein Geheimnis um die Verkabelung ihrer Womos machen. Mit etwas mehr Infos täte man sich leichter. Außerdem ist das ja wirklich kein Hightec. Mit etwas Mühe kann jeder die Kabelführung verfolgen. Und mehr als ein paar Relais findet man normalerweise auch nicht vor.
