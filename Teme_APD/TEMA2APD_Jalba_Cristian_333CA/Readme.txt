Readme Tema 2 APD
@author: Jalba Cristian 333CA
scrisa in notepad++

	In implementarea temei am pornit de laboratorul 5 , acela cu Replicated Workers . 
In principiu folosesc 3 instante ale clasei Workpool , 3 tipuri de workeri 
(WorkerMAP ,  WokerREDUCE , WorkerCOMPARE) si un singur PartialSolution care 
are campuri pentru toti workerii. 

	Main-ul se afla in clasa ReplicatedWorkers 	, unde pregatesc taskurile pentru 
fiecare tip de workeri si le pun in workpool-urile respective . Pentru workpoolMAP 
pun in PartialSolution inceputul si sfarsitul fragmentului ce trebuie wrokerul de 
tip MAP sa citeasca din fisier . Citirea din fisier o fac in WorkerMAP si mai exact
o fac printr-un vector de bytes de lungime foarte mare ce e citit cu totul folosind
functia read() din RandomAccessFile . Dupa ce obtin vectorul , caut delimitatorii si 
ii inlocuiesc cu " " apoi convertesc la String vectorul de bytes si fac split pe el.
Am respectat conditiile din cerinta ce impune sa pun un cuvant intr-un fragment atunci
cand sfarsitul fragmetului pica pe el , respectiv celelalte cazuri . 

	In WokerREDUCE folosesc index_partial_cuvinte din PartialSolution , mai concret
combin toate hash-urile create de peoni (workerii de tip MAP) intr-un hash de hash-uri,
unde fiecare cheie reprezinta numele fisierului , iar valoarea cuvintele indexate din acel
document . 

	In WorkerCOMPARE folosesc celelalte 2 hash-uri , document1 si document2 . Ele reprezinta
acele 2 documente ce sunt comparate la un moment dat. Pentru comparare folosesc un comparator
si supraincarc metoda compareTo() . Formula e cea din cerinta. Ma bazez pe o clasa separata 
pentru a face adaugarile in hash ca sa evit probleme de sincronizare , asta se poate observa
si in ceilalti workeri . Sortarea o fac in main , dupa ce se termina toate thread-urile . 

Codul nu e complicat , am lasat cateva comentarii in el . Sper sa fie de ajuns explicatiile date
aici . A fost o tema interesanta , va multumesc pentru timpul acordat updatarii si perfectionarii 
temei . 

	
