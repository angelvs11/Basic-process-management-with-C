#!/bin/bash
# ==========================================================
# TEST 3: Visualización del árbol de procesos
# Objetivo: probar la función print_process_tree() con procesos anidados
# ==========================================================

echo "==> TEST 3: Árbol de procesos"

# Crea un proceso bash con varios hijos (sleep 5 y sleep 10)
{
    echo "create bash -c \"sleep 5 & sleep 10 & wait\""
    sleep 2
    echo "tree"
    echo "wait"
    echo "quit"
} | ./procman > test3_output.txt 2>&1

# Verifica que se muestre el árbol correctamente
if grep -q "procman" test3_output.txt && grep -q "sleep" test3_output.txt; then
    echo "[OK] Árbol de procesos mostrado correctamente"
else
    echo "[FAIL] Error al mostrar el árbol de procesos"
    cat test3_output.txt
    exit 1
fi
