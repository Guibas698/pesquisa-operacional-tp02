#!/bin/bash
# Script para compilar e executar o Algoritmo Genetico em todas as 13 instancias ATSP

make compila

INST_DIR="instancias/Completos e Assimetricos nos Arcos"

for arq in "$INST_DIR"/*.atsp; do
    ./AG "$arq"
done
