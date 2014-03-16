*******************************************************************************
					CIOTEC MARIAN-STEFAN 333CA
						TEMA 2 - APD
*******************************************************************************


*******************************************************************************
			INDEXAREA DOCUMENTELOR FOLOSIND PARADIGMA MAP-REDUCE
*******************************************************************************


*******************************************************************************
CUPRINS:
	1) CONTINUT ARHIVA
	2) SPECIFICATII SISTEM
	3) DETALII IMPLEMENTARE
	4) ANALIZA COMPLEXITATII
*******************************************************************************


*******************************************************************************
1) CONTINUT ARHIVA
	FISIERE SURSA:
		- ReplicatedWorkers.java
		- WorkPool.java - din laborator(cu mici modificari)
		- GlobalVars.java
	README
*******************************************************************************


*******************************************************************************
2) SPECIFICATII SISTEM
	SISTEM OPERARE : MICROSOFT WINDOWS XP SP3
	IDE : ECLIPSE KEPLER 4.3.0	
*******************************************************************************


*******************************************************************************
3) DETALII IMPLEMENTARE
	MAP : 
		- UTILITATE : PRELUCREAZA TASK-URI DE FORMA <FILE,READ_START,READ_STOP> 
		SI ASAMBLEAZA SOLUTIILE PARTIALE, REZULTAND ASOCIERI DE FORMA 
		<FILE, <CUVANT, NR. APARITII>>;
		
		-ASOCIERILE <CUVANT, NR.APARITII> SUNT RETINUTE SUB FORMA UNUI HASHTABLE
		PENTRU FIECARE FISIER;
		
		- STAREA FIECARUI FISIER(UN HASTABLE CARE CONTINE ASOCIERI 
		<CUVANT, NUMAR APARITII> SI NUMARUL DE CUVINTE DIN FISIER) SUNT RETINUTE
		IN CLASA FileStatistics;
		
		- FISIERUL GlobalVars.java CONTINE CLASA GlobalVars LA ALE CAREI CAMPURI
		AU ACCES TOATE CLASELE(VARIABILE GLOBALE). CONTINE VECTORUL DE 
		FileStatistics, UN VECTOR DE LOCKS(SEMAFOARE) PENTRU FIECARE FISIER
		(PENTRU A SINCRONIZA MODIFICAREA VARIBILELOR FileStatistics
		
		ALGORITM DE REZOLVARE : 
		
		- THREAD-UL MASTER CREEAZA TASK-URI PENTRU WORKERII MAP SI LE PUNE IN 
		WORKPOOL-UL MAP(variabila map_wp);
		
		- SUNT PORNITI NT (NUMBER OF THREADS) WORKERI, CARE PRELUCREAZA 
		TASK-URILE DIN WORKPOOL. UN TASK DIN WORKPOOL-UL MAP ESTE O INSTANTA A
		CLASEI PartialMapSolution, CONTINAND NUMELE FISIERULUI DIN CARE TREBUIE
		CITIT, IDENTIFICATORUL FISIERULUI, POZITIA DE UNDE TREBUIA SA INCEAPA 
		SA CITEASCA SI POZITIA PANA UNDE TREBUIE CITIT;
		
		- UN TASK ESTE PRELUCRAT DE CATRE UN WORKER PRIN FUNCTIA 
		processPartialSolution(PartialSolution). PENTRU FIECARE TASK, FISIERUL
		DIN CARE TREBUIE CITIT ESTE DESCHIS SI SE CALCULEAZA POZITIILE DE START
		SI DE FINAL(SARE CUVANTUL DACA POZITIA DE START E IN INTERIORUL
		CUVANTULUI SI CITESTE TOT CUVANTUL DACA POZITIA DE STOP ESTE IN INTERIORUL
		CUVANTULUI). PENTRU CITIRE FISIER AM UTILIZAT RandomAccessFile SI 
		FUNCTIILE seek(long), readByte() si read(byte[], offset, len);
		
		- WORKERII(THEAD-URILE) CITESC IN PARALEL DIN FISIERE, NUMAI MODIFICAREA
		ASUPRA VARIABILELOR CARE RETIN STAREA VARIABILELOR ESTE SINCRONIZATA, DAR
		PARALELA.
		DE EXEMPLU : PRESUPUNEM CA AVEM 5 THREAD-URI DINTRE CARE PRIMELE 2 AU DE 
		PRELUCRAT O PARTE DIN PRIMUL FISIER, URMATOARELE 2 DIN AL DOILEA FISIER SI 
		ULTIMUL DIN AL TREILEA FISIER. TOATE 5 VOR CITI IN PARALEL, IAR APOI
		THREAD-URILE 1 SI 2 SE VOR SINCRONIZA PENTRU MODIFICAREA VARIABILEI
		CORESPUNZATOARE PRIMULUI FISIER, THREAD-URILE 3 SI 4 PENTRU AL DOILEA
		FISIER IAR ULTIMUL VA MODIFICA VARIABILA PENTRU AL TREILEA FISIER.
		PENTRU CA SUNT LOCK-URI DIFERITE PENTRU FIECARE VARIABILA, MODIFICARILE
		ASUPRA A VARIABILE DIFERITE SUNT PARALELE, NUMAI MODIFICAREA ASUPRA
		ACELEIASI VARIABILE ESTE SINCRONIZATA;
		
		- PENTRU A REALIZA O DISTRIBUTIE CAT MAI BUNA FUNCTIA getWork() DIN
		CLASA WorkPool INTOARCE TASK-URI RANDOM, EVITAND ASTFEL O SITUATIE IN
		CARE TOATE THREAD-URILE AR FI PRELUCRAT TASK-URI PENTRU ACELASI FISIER,
		CA MAI APOI SA SE SIINCRONIZEZE TOATE PENTRU MODIFICAREA VARIBILEI.
		IN VARIANTA DIN LABORATOR INTORCEA NUMAI ELEMENTE CONSECUTIVE, DECI
		AR FI MERS MAI LENT;
		
	REDUCE : 
		- UTILITATE : PRELUCREAZA TASK-URI DE FORMA <FILE1, FILE2> SI INTOARCE
		GRADUL DE SIMILARITATE DINTRE CELE DOUA CALCULAT PE BAZA INFORMATIILOR
		RETINUTE IN VARIBILELE GLOBALE;
		
		- VECTORUL DOUBLE SIM[] DIN GlobalVars RETINE GRADELE DE SIMILITUDINE
		ALE TUTUROR FISIERELOR CU UN FISIER;
		
		ALGORITM DE REZOLVARE : 
		- SUNT PORNITI NT WORKERI, CARE PRELUCREAZA TASK-URILE DIN WORPOOL. 
		FIECARE WORKER CALCULEAZA CATE UN GRAD DE SIMILITUDINE INTRE 2 FISIERE
		PRIN REZOLVAREA UNUI TASK.
*******************************************************************************


*******************************************************************************
4) ANALIZA COMPLEXITATII
			NOTATII:
				NT - nr de thread-uri
				ND - nr de fisiere
				D - nr de bytes care este citit de catre un thread la o citire
				S - marimea medie a unui fisier in bytes
				NC - nr de cuvinte al fisierul de verificat impotriva plagiatului
				N - numarul mediu de bucati de citit dintr-un fisier
				
			COMPLEXITATE OPERATIA MAP:
			
				- pp ca un thread executa operatia de read a D bytes dintr-un
				fisier in O(1);
				
				=> citirea unui fisier de catre un thread se executa in O(S/D)
					
				=> citirea celor ND fisiere de catre un thread se executa in
					O(ND*(S/D)) => O(ND*N)
					
				Pentru ND foarte mare, cazurile in care mai multe thread-uri 
				trebuie sa se sincronizeze pentru modificarea unei varibile
				sunt foarte rari
				
				=> citirea fisierelor si modificarea varibilelor in mod paralel
				de catre NT thread-uri se executa in O((ND*N)/NT);
				
				=> COMPLEXITATE MAP : O((ND*N)/NT)
				
			COMPLEXITATE OPERATIA REDUCE:
				
					- sunt prelucrate ND-1 task-uri de forma <file_plag, file_i>
					in mod paralel de catre NT thread-uri
					- la prelucrarea unui task se itereaza prin NC cuvinte distince
					(cheile hashtable-ului corespunzator fisierului de verificat
					impotriva plagiatului)
					
					=> COMPLEXITATE REDUCE : O((ND*NC)/NT)
					
			
			COMPLEXITATE TOTALA : O((ND*N)/NT) + O((ND*NC)/NT)  = 
								  O((ND*(N+NC))/NT)
			PENTRU ND>>N si NC>>N => O(ND/NT)
				