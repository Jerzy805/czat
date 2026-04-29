Znajdują się tu pliki tworzące w całości program czat, poniżej krótka instrukcja instalacji i użytkowania:

1. Pobierz wszystkie pliki (z ewentualnym pominięciem .gitignore)
2. Uruchom polecenie make all
3. Po skompilowaniu, jeżeli jesteś zalogowany do spk poprzez ssh, to odpal ./call, w przeciwnym wypadku ./login, a potem już ./call*
4. Reszta programów musi być skompilowana, ale działają one jedynie jako moduły programu i są wywoływane z poziomu programu call
5. Obecnie przesyłanie plików jest dostępne jedynie w konwersacji 1-1, instrukcja znajduje się w menu call.



*Czat wymaga, żeby użytkownik odpalał go z poziomu spk, a nie lokalnego komputera, nawet w pracowni komputerowej, dlatego program ./login loguje cię na spk i automatycznie uruchamia stamtąd ./call, pozostając zalogowanym. Jeżeli jesteś zalogowany, wystarczy używać ./call, 