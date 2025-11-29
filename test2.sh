#!/bin/bash
# ==========================================================
# TEST 2: Manejo de señales SIGINT (Ctrl+C) y SIGCHLD
# Objetivo: verificar que procman se cierra limpiamente al recibir Ctrl+C
#           y que no quedan procesos zombie
# ==========================================================

echo "==> TEST 2: Manejo de señales"

# Ejecuta procman y crea un proceso "sleep 10"
# Después de 2 segundos, le envía SIGINT (Ctrl+C)
{
    echo "create sleep 10"
    sleep 2
} | ./procman > test2_output.txt 2>&1 &

PROC_PID=$!
sleep 3
kill -SIGINT $PROC_PID  # Simula presionar Ctrl+C
wait $PROC_PID

# Verifica que se haya mostrado el mensaje de apagado limpio
if grep -q "Shutting down" test2_output.txt; then
    echo "[OK] Señal SIGINT manejada correctamente"
else
    echo "[FAIL] No se manejó correctamente SIGINT"
    cat test2_output.txt
    exit 1
fi

# Verifica que no haya zombies
if ps aux | grep '[s]leep 10' | grep -q Z; then
    echo "[FAIL] Hay procesos zombie después de SIGINT"
    exit 1
else
    echo "[OK] No hay procesos zombie"
fi
