#!/bin/bash

archivos=("../testsFinal/memoria_configs/memoria.config")  # Lista de archivos
linea_a_modificar=7                      # Línea a modificar (pasada como argumento)
nuevo_contenido="$1"                         # Nuevo contenido a asignar

for archivo in "${archivos[@]}"; do
  # Verificar si el archivo existe
  if [ -f "$archivo" ]; then
    # Copiar el archivo a un archivo temporal
    cp "$archivo" "$archivo.tmp"

    # Calcular el número de línea basado en la línea a modificar
    numero_linea=$((linea_a_modificar))

    # Obtener el contenido antes del símbolo '=' en la línea específica
    contenido_anterior=$(awk -F'=' -v num_linea="$numero_linea" 'NR == num_linea {print $1}' "$archivo.tmp")

    # Crear la nueva línea con el contenido anterior y el nuevo contenido
    nueva_linea="$contenido_anterior=$nuevo_contenido"

    # Reemplazar la línea específica del archivo temporal con la nueva línea
    sed -i "${numero_linea}s/.*/${nueva_linea}/" "$archivo.tmp"

    # Reemplazar el archivo original con el archivo temporal
    mv "$archivo.tmp" "$archivo"
    echo "Se ha modificado la línea $linea_a_modificar del archivo $archivo."
  else
    echo "El archivo $archivo no existe."
  fi
done
