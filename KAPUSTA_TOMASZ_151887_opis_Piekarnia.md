https://github.com/xtomvs/projekt_sys_op
Temat 12 - Piekarnia
Piekarnia produkuje P różnych produktów (P>10), każdy w innej cenie i na bieżąco sprzedaje je w
samoobsługowym sklepie firmowym. Produkty bezpośrednio po wypieku (losowa liczba sztuk różnych
produktów co określony czas) trafiają do sprzedaży w sklepie – każdy rodzaj produktu Pi na oddzielny
podajnik. Każdy podajnik może przetransportować w danej chwili maksymalnie Ki sztuk pieczywa.
Pieczywo z danego podajnika musi być pobieranie w sklepie dokładnie w takiej kolejności jak zostało
położone na tym podajniku w piekarni.
Zasady działania piekarni przyjęte przez kierownika są następujące:
• Piekarnia/sklep jest czynny w godzinach od Tp do Tk.
• W sklepie w danej chwili może się znajdować co najwyżej N klientów (pozostali, jeżeli są
czekają przed wejściem);
• W sklepie są 3 stanowiska kasowe, zawsze działa min. 1 stanowisko kasowe.
• Na każdych K (K=N/3) klientów znajdujących się na terenie supermarketu powinno przypadać
min. 1 czynne stanowisko kasowe.
• Jeśli liczba klientów jest mniejsza niż 2*N/3, to jedna z 3 kas zostaje zamknięta.
• Jeśli w kolejce do kasy czekali klienci (przed ogłoszeniem decyzji o jej zamknięciu) to powinni
zostać obsłużeni przez tę kasę.
Klienci przychodzą do piekarni w losowych momentach czasu z losową listą zakupów (spośród P
produktów każdy klient wybiera min. dwa różne, np.: bułka kajzerka 3szt., chleb żytni 1 szt., …). Jeżeli
dany produkt jest niedostępny (podajnik jest pusty) klient nie kupuje tego towaru. Następnie klient
udaje się do kasy, a kasjer wystawia paragon na zakupy (kasjer zapamiętuje ile sztuk danego
produktu skasował).
Na komunikat (sygnał1) o ewakuacji – sygnał wysyła kierownik - klienci natychmiast przerywają
zakupy i opuszczają piekarnię omijając kasy – pobrany już z podajników towar odkładają do kosza
przy kasach.
Na komunikat (sygnał2) o inwentaryzacji – sygnał wysyła kierownik - klienci kontynuują zakupy
normalnie do zamknięcia piekarni/sklepu. Po zamknięciu sklepu, każda kasa robi podsumowanie
sprzedanych produktów (Pi – liczba szt.), kierownik sumuje towar na podajnikach, piekarz podaje ilość
wytworzonych produktów.
Napisz program klienta, kierownika, piekarza i kasjera