### GOIDESCU Rares-Stefan 2023-2024

# Distributed Data Base - Tema 2 SD Seria CA

## Note despre tema

### Ce mi s-a parut interesant?

Ideea temei a fost foarte interesanta.
Aceasta mi-a dat o idee foarte buna despre cum sunt folosite diferite structuri de date in lumea reala, mai ales cand vorbim de centre de stocare a datelor.

### Care a fost cea mai dificila parte a temei?

As zice ca implementarea server-ului si a cache-ului a fost cea mai dificila din cauza multiplelor probleme cu management-ul memoriei.

A trebuit sa fiu foarte atent cum tratez rezultatele interogatiilor, zonele de memorie alocate in client-side si la lucrurile pe care le stocam in dictionarul de cache.

### Ce cred ca as fi putut face mai bine?

Exista o mica posibilitate sa fiu dependent de `goto`.
De asemenea, sunt de parere ca as fi putut face unele lucruri mai elegant, insa... examene, sarbatori :D.

---

## LOAD BALANCER

Ca structuri de date, am folosit un vector de pointeri catre servere, care a fost tratat ca circular in implementare (acum ca ma gandesc, poate ar fi fost mai eleganta o lista circulara).

### ADD SERVER

Se adauga un server in vectorul circular, se afla pozitia pe care a fost adaugat si se cauta posibilul donator de documente.
In caz ca exista, se da pop la coada de cereri pe serverul donator, iar mai apoi, se cauta documentele care ar putea fi donate, respectand principiul Consistent Hashing-ului.
Atunci cand se muta documente, acestea sunt eliminate din cache-ul serverului sursa.
In caz contrar, nu se intampla nimic.

### REMOVE SERVER

Se cauta pozitia pe inel a serverului care se vrea eliminat, in caz ca nu se gaseste, nu se intampla nimic, iar in caz ca exista, se trimite un request pentru a executa toate cererile din coada, apoi cauta posibilul succesor si se muta documentele, urmand ca serverul sa fie sters de pe inel si eliberat din memorie.

### FORWARD REQUEST

Se cauta serverul cel mai potrivit in inel tinand cont principiul consistent hashing-ului.
Daca nu este gasit niciun server cu un hash mai mare decat cel al documentului, cererea este preluata de primul server.

## SERVER

Ca structuri de date, am folosit un dictionar care tine documentele, o coada implementata cu o lista simplu inlantuita pentru request-uri si cache-ul despre care o sa vorbesc mai jos.

In mare parte, am urmat flow-ul descris in cerinta temei, insa am intampinat extrem de multe probleme cu valgrind-ul intrucat nu stiam exact unde si cand se elibereaza memoria
request-urilor, asa ca am facut copii ale acestora, apoi le-am adaugat in coada de cereri.

### HANDLE REQUEST

Un lucru care ar trebui mentionat la `server_handle_request()`, un request care are campul `doc_name` `NULL` va declansa un `goto` care da pop la request-urile din coada.

Ideal, client-side-ul ar trebui sa verifice daca acel camp este `NULL` pentru a nu da forward request-ului.

Daca request-ul nu are `doc_name` nul, atunci totul decurge in mod normal: request-urile de `EDIT` se adauga in coada, in modul *lazy*, pana cand un request de tip `GET` este primit, moment in care se da pop pe coada si se executa toate request-urile stocate pana atunci.

### EDIT & GET

Cele doua operatii au fost tratate in mod asemanator.
Ambele urmeaza flow-ul descris in cerinta:
1. Verificam cache-ul pentru existenta documentului dorit
    - In caz ca exista, preluam sau actualizam continutul si in cache, si in baza de date
    - In caz ca nu exista este adaugat/actualizata ordinea in cache
2. Verificam, de asemenea, inainte sa actualizam ordinea in cache pentru cazul de mai sus, daca fisierul exista in baza de date.

Astfel, cream, actualizam si/sau preluam documente cu usurinta.

## CACHE

Ca structuri de date, am folosit un dictionar care tine perechi `(doc_name, adresa_catre_document_in_lista)` si o lista dublu inlantuita in care tin documentele `(doc_name, doc_content)` ordonate dupa ordinea accesarii/adaugarii in cache.

Rolul principal al dictionarului in acest caz, este de a oferi acces in O(1) la orice nod din lista.

### PUT

Daca cache-ul este plin, scoatem din lista cel mai vechi nod (care se va afla la finalul listei (operatia se face in O(1) deoarece am acces la `tail`)), trimitem prin *referinta* numele documentului care a fost scos din cache, si este adaugat noul document, la inceputul listei (pozitia cea mai recenta).

In caz ca nu este plin, pur si simplu adaugam documentul si nu intoarcem nimic.

### GET

Daca documentul nu exista, nu intoarcem nimic, iar daca exista, documentul este pus la inceputul listei pentru a marca faptul ca este cea mai recenta actiune in cache, si intoarcem documentul.

### REMOVE

Daca nu exista documentul pe care il vrem sters din cache, ne oprim, iar daca exista, pur si simplu il scoatem din lista (folosind accesul rapid cu dictionarul), iar mai apoi il scoatem si din dictionar.

## BONUS

La urmatoarea tema, promit :D

---

## PS

Am schimbat cateva obiceiuri, luand in considerare feedback-ul de la tema trecuta:
- Am inceput sa folosesc formatul Doxygen (care imi place prea mult)
- Am inceput sa nu mai tin cont de cele 80 de coloane in README, intrucat am inteles ca este mai eficient sa primesc review cand fiecare propozitie ocupa un singur rand
- Am incercat sa mentin, in continuare, README-ul scurt si la obiect, fara a pierde din informatii
- Nu am mai eliminat `#ifdef DEBUG`-urile.
