**Tema 3**

Scopul acestei teme a fost trimiterea si primirea, in mod corect, a mesajelor de la un client implementat la un server remote. Pentru asta, codul primeste, intr-un while loop continuu comenzile de la tastatura ale user-ului pe care le compara cu comenzile oferite de functionalitatea server-ului pentru a sti exact ce functionalitate trebuie adusa in discutie. Inaintea fiecarei comenzi oferite se deschide un nou canal de comunicatie TCP cu server-ul, deschidere ce se face cu ajutorul functiilor oferite la dispozitie din cadrul laboratorului si la final, se va inchide conexiunea respectiva, dupa rezolvarea cerintei in cauza. Am luat din cadrul laboratorului doar helpers.c(pe care am translatat-o in cpp, doar schimbandu-i numele) si request.cpp in care am modificat functionalitatea din C si am adaptat-o pentru C++(vector, string, alte semnaturi), iar buffer ajuta helpers in functionalitate. De asemenea, am creat doua variabile globale, una pentru id si una pentru token, pentru a nu fi fost nevoit sa le parsez in functii de fiecare data cum am facut cu alte variabile.
Am folosit implementarea bibliotecii nlohmann intrucat am lucrat cu C++. A fost foarte ajutatoare sa parsez mai ales vectorii din raspunsurile primite in get_collections/movies si numai. De asemenea, am folosit si pentru a crea mesajul la fiecare trimitere a cererii, meesaj care se putea crea simplu si fara, dar am tinut sa fie implementat si asa, dar mai pe larg voi explica in cele ce urmeaza. Astfel, voi detalia continutul fiecarei comenzi implementate de mine:

1. send_http_request -> aceasta functie creeaza un buffer pe lungimea meesajului pe care il primeste si il trimite la server cu ajutorul functiei dezvoltate din helpers.cpp.

2. valid_id -> valideaza daca un id este gol si daca este cifra, returnand false in caz contrar.

3. login_admin -> primeste de la tastatura username-ul si parola si verifica validaitea lor(gol, spatii goale). Daca au fost credentialele valide, creez cu ajutorul bibliotecii un json si il trimit cu ajutorul functiei care creeaza mesajul de POST in mod corect. Astept raspuns, extrag json-ul din raspuns si, cu ajutorul functiilor lui string(find), caut daca exista match pe anumitele raspunsuri si afizez mesajul potrivit. De asemenea, daca server-ul trimite success, voi primit automat si un cookie pe care il extrag cu ajutorul strtok-ului si il pun in vectorul de cookie-uri. Initial, am crezut ca vor fi mai multe, asteptand sa introduc mai multe string-uri intr-un vector, nu am stiut ca va fi doar unul pe sesiune, de aceea am implementat cu vector si nu cu string. De asemenea, setez variabila care-mi spune ca admin-ul este conectat pe true.

4. login -> este identica cu cea de sus, in sensul ca mai intai introduc datele, verific validatea lor, compun mesajul, trimit mesajul, astept raspuns pe socket-ul respectiv si interpretez mesajul. La fel, in caz de succes, voi pune cookie-ul de sesiune in vector si voi seta ca utilizatorul este conectat.

5. add_user -> nu este altceva decat reluarea pasilor de mai sus : introducere, validare, trimitere cerere, asteptare raspuns, interpretarea acesteia.

6. get_users -> aici, la interpretarea mesajului, m-am folosit de biblioteca json pentru a parsa meesajul. Voi crea un obiect de tip json si voi itera prin vectorul de users din "users". Initial, doar afisam toate campurile, insa biblioteca puea inainte si dupa string caracterul de ". Asa ca, folosindu-ma de functia substr am facut escape primului si ultimului caracter din fiecare string, ramand astfel doar cu string-ul corect.

7. delete_user -> primeste un id de la tastatura care este validat cu ajutorul functiei prezentate mai sus(valid_id), iar daca totul este in regula, trimite cererea la url ul respectiv, verifica ce primeste si afiseaza in consecinta.

8. get_access -> in implementarea din laborator exista si query, un char**, pe care l-am transformat intr un vector de perechi de string-uri, insa, in implementarea temei nu am avut niciodata nevoie de el, dar nu l-am mai sters, crezand mereu ca poate va veni randul lui. Asa ca, server-ul trimite un get request la URL ul respectiv si daca primeste success, in json i se va returna un token, pe care l parsez cu ajutorul lui strtok si il pun in variabila globala pentru a ma folosi de el la fiecare actiune necesara.

9. get_movies -> pe langa cerere si primirea de raspuns, am folosit la fel, un obiect json pentru parsarea in vector, tocmai pentru asta fiind si implementata biblioteca, nefiind nevoit sa parsez in alte feluri. Asa ca, daca un utilizator nu are nimic in vectorul de movies, afisez succes, dar niciun film, iar daca are cel putin un element, iterez cu un for prin elementele vectorului si reiau felul in care am afisat(cu substr, pentru a elimina caracterul " de la inceput si final).

10. get_movie -> citesc id-ul, il verific, trimit cererea, primesc raspuns, parsez cu ajutorul bibliotecii json fiecare camp si afisez.

11. logout_admin -> doar daca utliziatorul este conectat se trimite cererea, setandu-se admin_connected pe false, in caz de succes, iar de asemenea, se sterge token_jwt-ul.

12. add_movie -> se citesc datele de intrare si se valideaza fiecare camp astfel incat el sa fie corect. Am gasit ca cin.fail() nu lasa sa treaca introducerea unui alt tip de date in locul celui oferit in declarere, iar cin.ignore() ia '\n' de la sfarsitul citirii int-ului. Daca vreuna a fost invalida, dupa citirea datelor, nu se va trimite cererea. Aceeasi poveste, se creeaza un obiect json, se pun datele in el, se trimite si se asteapta un raspuns pentru a fi interpretat.

13. delete_movie -> primeste un id pe care-l verifica si urmeaza aceeasi pasi hipercunoscuti.

14. update_movie -> acelasi lucru

15. get_collections -> dupa trimiterea cererii, se verifica daca exista colectii sau nu in vectorul de colectii, iar daca exista, le afisez conform rationamentului de mai sus.

16. get_collection -> acelasi lucru

17. add_movie_to_collection -> functie ce primeste un bool in plus pentru a sti daca este apelata din main, pentru a adauga un film intr-o colectie deja existenta, sau este apelata din functia add_collection, care adauga la randul ei, n filme in colectie. Daca este apelata din main, primeste un collection id si un movie id si verifica validatea acestora. Pune in json doar movie-id-ul, si trimit pe url-ul cu collection id.

18. add_collection -> initial, se va citi titlul colectiei si numarul de filme care vor fi adaugate. Mai apoi, se trimite un request pentru a vedea daca se poate crea colectia, iar in caz afirmativ, se citesc cele n filme care se vor adauga cu functia anterioara, dandui-se ca argumente id-ul colectiei si id-ul filmului de adaugat. Daca cel putin un film nu reuseste sa fie adaugat, functia va da fail total. Daca toate au fost adaugate, se va afisa un mesaj de succes.

19. delete_colection -> se trimite un request de delete la server si se asteapta raspuns

20. delete_movie_from_collection -> am creat URL-ul doar cu collection_id, iar movie_id-ul va fi adaugat in functie, la sfarsitul URL-ului. Se asteapta raspuns care va fi interpretat.

21. logout -> utiliztorul se delogheaza, la fel ca in admin_logout.

22. main -> citim intr-un while toate comenzile.
