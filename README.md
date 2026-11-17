# Practica 3 de la asignatura de Sistemas Empotrados y Tiempo Real de 3 año de la carrera de Ingenieria de Robotica Software
##  Introducción 
Para este proyecto vamos ha hacer uso de una placa Elegoo y diferentes componentes. De componentes usaremos una pantalla LCD, un joystick, un sensor de temperatura y humedad, un sensor de ultra sonidos, un pulsador y dos LEDs.

El proyecto consiste en crear una especie de simulación de maquina expendedora, donde en un modo inicial estara en modo servicio y permitira al usuario navegar atraves de una interfaz y seleccionar el tipo de bebida que necesite. Si se presiona durante dos o tres segundos el boton la maquina se reiniciara, y en caso de que el tiempo presionando al boton sea 5 segundos la maquina entrara en modo administrador. En modo administrador podremos navegar por un menu especial para verificar estados de los sensores y modificar los precios de las bebidas.

<!--TODO Me gustaria poder añadir otros componentes, tales como un zumbador o un sensor laser para el ingreso de monedas, e incluso un motor para la ventilacion de la propia maquina.-->

## Funcionamiento
Para ello se crearon diferentes archivos para tener el proyecto mucho más ordenado como archivos .h y .cpp para evitar tener un main de demasiadas lineas y asi ayudar a la busqueda de errores y para facilitar la modificacion del código en el futuro.

### Inicio y Reinicio
El encargado de esto es la funcion `SystemOn()` que se encarga de mostrar el mensaje "Cargando..." en la pantalla LCD ademas del parpadeo del LED. Esta funcion sera llamada al inicializar la placa (para el arranque), y en el bucle inicial se la llamara dos veces. La primera seria en el caso de que el botón fuese presionado durante 2 o 3 segundos y la segunda vez seria en el caso en el que se produjera un bloque del sistema, que la maquina deberia de reiniciarse ella sola.

### Servicio


## Esquemas

## Fotos

## Componentes adicionales