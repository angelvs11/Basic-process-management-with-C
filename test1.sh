#!/bin/bash
# ==========================================================
# TEST 1: Creación y terminación de procesos
# Objetivo: verificar que se pueden crear y finalizar procesos correctamente
# ==========================================================

echo "==> TEST 1: Creación y terminación de procesos"

# Inicia procman en modo interactivo y ejecuta comandos automáticamente
# El símbolo '|' manda las líneas como si las escribieras manualmente
{
    echo "create sleep 5"
    sleep 1   # Espera a que el proceso se cree
    echo "list"
    echo "wait"
    echo "quit"
} | ./procman > test1_output.txt 2>&1

# Verifica si la salida contiene palabras clave esperadas
if grep -q "Created process" test1_output.txt && grep -q "Terminated" test1_output.txt; then
    echo "[OK] Procesos creados y finalizados correctamente"
else
    echo "[FAIL] Error en la creación o terminación de procesos"
    cat test1_output.txt
    exit 1
fi
