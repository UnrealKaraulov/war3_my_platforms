@echo off
:: -----------------------------------------------
:: Language:    Spanish
:: Author:      Joaquin Cabrera (jmcc3001@gmail.com)
:: -----------------------------------------------
:: Code Page:   850
:: Encoding:    OEM 850


set PHRASE_0_0=^^!^^!^^! PELIGRO ^^!^^!^^!
set PHRASE_0_1=Magic Builder tiene que estar ubicado en una ruta sin espacios, letras unicode y caracteres especiales (_.- est�n permitidos).
set PHRASE_0_2=Por ejemplo: %2

set PHRASE_1_0=  Visual Studio no est� instalado
set PHRASE_1_1=Seleccione una versi�n de Visual Studio para construir PvPGN:
set PHRASE_1_2=%2 ha sido seleccionado como entorno de construcci�n
set PHRASE_1_3=�Le gustar�a descargar/reemplazar la ultima fuente de PvPGN desde GIT (dentro del directorio %2)?
set PHRASE_1_4=   El c�digo de fuente PvPGN ser� reemplazado desde GIT
set PHRASE_1_5=   El c�digo de fuente PvPGN no ser� actualizado
set PHRASE_1_6=Seleccione interfaz PvPGN: 
set PHRASE_1_7=   1) Consola (Predeterminado)
set PHRASE_1_8=   2) Interfaz de Usuario Gr�fica
set PHRASE_1_9=Seleccione un n�mero
set PHRASE_1_10=   Establecer interfaz PvPGN como Consola
set PHRASE_1_11=   Establecer interfaz PvPGN como IUG (Interfaz de Usuario Gr�fica)
set PHRASE_1_12=Seleccione un tipo de base de datos: 
set PHRASE_1_13=   1) Simple / CDB (Predeterminado)
set PHRASE_1_14=
set PHRASE_1_15=   PvPGN ser� construido sin soporte de base de datos
set PHRASE_1_16=Configuraci�n CMake ha fallado

set PHRASE_2_1=%2 versiones disponibles (Puedes a�adir las tuyas en %3):
set PHRASE_2_2=   Ingrese un n�mero
set PHRASE_2_3=   N�mero incorrecto... Intente de nuevo
set PHRASE_2_4=   PvPGN ser� compilado con %2 soporte v%3 
set PHRASE_2_5=�Quieres configurar ajustes para %2 ahora (bnetd.conf ^> ruta_de_almacenamiento)?
set PHRASE_2_6=    Anfitri�n de Conexi�n
set PHRASE_2_7=    Usuario de Conexi�n
set PHRASE_2_8=    Contrase�a de Conexi�n
set PHRASE_2_9=    Nombre de Base de Datos
set PHRASE_2_10=    Prefijo de tabla (Predeterminado es %2)
set PHRASE_2_11=La configuraci�n de %2 ser� guardada en %2.conf.bat

set PHRASE_3_1=Buscando actualizaci�n ...
set PHRASE_3_2="v%2" es tu versi�n
set PHRASE_3_3="v%2" es la versi�n remota
set PHRASE_3_4= Tienes el �ltimo PvPGN Magic Builder
set PHRASE_3_5=La versi�n remota de PvPGN Magic Builder no es igual a la tuya. �Quieres actualizar a la �ltima version automaticamente?
set PHRASE_3_6= La actualizaci�n ha sido cancelada por el usuario
set PHRASE_3_6_1= No hay conexi�n con el servidor de actualizaci�n
set PHRASE_3_7=Comenzando actualizaci�n ...
set PHRASE_3_8= Descargando archivo %2 ...
set PHRASE_3_9=Actualizaci�n completada
set PHRASE_3_10=Por favor, revise el archivo %2 para m�s informaci�n sobre los cambios

set PHRASE_4_1=�Habilitar soporte Lua scripting ?
set PHRASE_4_2=   PvPGN ser� compilado con Lua
set PHRASE_4_3=   PvPGN ser� compilado sin Lua

set PHRASE_9_1=Seleccione su versi�n de D2GS:
set PHRASE_9_2=Establezca contrase�a administrativa para conexi�n Telnet (Escuchando en puerto 8888), ser� guardado en d2gs.reg
set PHRASE_9_3=Contrase�a hash ser� guardada en %2
set PHRASE_9_4=�Quieres descargar archivos MPQ esenciales y originales? (size 1GB)
set PHRASE_9_9=^^!^^!^^! Para terminar configurando D2GS editar d2gs.reg y ejecutar install.bat ^^!^^!^^!
