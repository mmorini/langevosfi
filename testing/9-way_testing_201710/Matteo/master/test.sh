(echo 10 15 40; echo -1 -1; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v1.txt
(echo 10 15 40; echo  0 -1; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v2.txt
(echo 10 15 40; echo  1 -1; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v3.txt

(echo 10 15 40; echo -1 0; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v4.txt
(echo 10 15 40; echo  0 0; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v5.txt
(echo 10 15 40; echo  1 0; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v6.txt

(echo 10 15 40; echo -1 1; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v7.txt
(echo 10 15 40; echo  0 1; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v8.txt
(echo 10 15 40; echo  1 1; echo 1 100; echo 100 1000 1000; echo 123) | ../../../main > v9.txt


