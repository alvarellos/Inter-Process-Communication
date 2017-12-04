#!/bin/bash 
#este archivo es un scrip que compila fuente1, fuente2 y fuente3 
# y luego ejecuta Ej1

# Elimina fichero1
cd Trabajo2

# CABECERA 
blue=$(tput setaf 4)
yellow=$(tput setaf 3)
normal=$(tput sgr0)

echo -e '\e[1m\e[33m-------------------------------------------------'
printf "%30s\n" "${blue}PED2 ${normal}Comunicacion de Procesos"

echo -e '\e[1m\e[33m-------------------------------------------------'
echo -e '\e[1m\e[34mAmpliacion de Sistemas Operativos Uned' 
echo -e 'Diego Diaz Alvarellos'
echo 'Curso 2016-2017'
echo -e '\e[1m\e[33m-------------------------------------------------'
printf "${normal}"

# fin cabecera

rm -f fichero1 >> /dev/null

# compila y genera ejecutables

gcc -o Ej1 fuente1.c
gcc -o Ej2 fuente2.c
gcc -o Ej3 fuente3.c

# Se ejecuta Ej1
./Ej1

# Se borran del directorio Ej1, Ej2, Ej3
rm Ej1
rm Ej2
rm Ej3

# Borrados con exito Ej1, Ej2, Ej3
echo ' Borrados Ej1, Ej2, Ej3'