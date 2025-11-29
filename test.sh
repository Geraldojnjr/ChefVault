#!/bin/bash
# Script para executar testes com saída detalhada
# Uso: ./test.sh ou make test-detailed

cd build
ctest --output-on-failure --verbose

