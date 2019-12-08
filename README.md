# TITULO: AVENTURA 2
###### DESCRIPCION: MINISHELL
###### GRUPO: TryHards
###### AUTORES: FELIX AGUILAR FERRER, ADRIAN BENNASAR POLZIN Y ALVARO BUENO LOPEZ
###### FECHA: 15/12/2019

Este shell permite ejecutar un conjunto de comandos internos definidos en este 
y cualquier comando externo. El conjunto de comandos internos es: 
- cd: permite cambiar de directorio.
- export: permite cambiar el valor de una variable de entorno.
- source: permite la ejecución de comandos contenidos en un archivo.
- jobs: muestra los trabajos activos en segundo plano y detenidos.
- fg: permite ejecutar un trabajo en primer plano.
- bg: permite ejecutar un trabajo en segundo plano.

Además de estos comandos internos se puede modificar el estado de un trabajo 
utilizando Ctrl+C, este finaliza la ejecución del trabajo en primer plano,
Ctrl+Z, este detiene la ejecución del trabajo en primer plano y lo pone en la
cola de trabajos en segundo plano.

Para finalizar la ejecución del mini shell, se puede utilizar el comando "exit"
o la combinación de teclas Ctrl+D.

En la carpeta se encuentran todos los niveles que se han ido realizando a lo
largo del desarrollo del proyecto. Para poder ejecutar cualquiera de ellos 
se pueden compilar utilizando el Makefile con la estructura "make archivo sin 
extension". Para limpiar el directorio de los archivos compilados después de
su uso, se puede utilizar el comando "make clean".

Hay que tener en cuenta que los niveles del 1 al 6 pueden contener errores que
se han corregido en el nivel 7 y en my_shell, por ejemplo, a partir del nivel 5
cuando se implementan las funciones fg y bg, el Ctrl+Z puede dar probemas al 
detener el proceso. Además, pese a que todos los niveles están comentados, los
únicos que tienen comentarios revisados son el nivel 7 y my_shell, por lo que
puede haber faltas de otrografía o zonas sin comentar en los demás niveles.

Se han implementado todas las funciones adicionales, por lo tanto, se puede 
ejecutar el comando "cd" utilizando los carácteres ("",'' y \) para indicar 
directorios con espacios en blanco. Por ejemplo: cd "Aventura 2" o cd 
Aventura\ 2. 

También se han incluido las funciones fg y bg las cuales permiten
ejecutar un trabajo en primer plano el cual estaba ejecutándose en segundo 
plano o detenido y ejecutar un trabajo en segundo plano el cual estaba 
detenido respectivamente.


