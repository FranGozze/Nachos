# PLANCHA I

## EJERCICIOS

1. ¿Por qué se prefiere emular una CPU en vez de utilizar directamente la CPU existente?
2. ¿Cuánta memoria tiene la maquina simulada para Nachos?
3. ¿Qué modificaria para cambiar la cantidad de memoria?
4. ¿De qué tamaño es un disco?
5. ¿Cuántas instrucciones de MIPS simula la máquina virtual de Nachos?
6. ¿En qué archivos está definida la función main? ¿En qué archivo está definida la función main del ejecutable nachos del directorio userprog?
7. Nombre los archivos fuente en los que figuran las funciones y métodos llamados por el main de Nachos al ejecutarlo en el directorio threads, hasta dos niveles de profundidad.
   Por ejemplo: main llama a Initialize, que está en tal archivo; e Initialize llama a ASSERT, que está en tal otro archivo.
8. ¿Qué efecto hacen las macros ASSERT y DEBUG definidas en lib/utility.hh?
9. Comente el efecto de las distintas banderas de depuración.
10. ¿Dónde están definidas las constantes USER PROGRAM, FILESYS NEEDED, FILESYS STUB?
11. ¿Qué argumentos de linea de comandos admite Nachos? ¿Que efecto tiene la opcion-rs?
12. Al ejecutar nachos -i, se obtiene informaci´on del sistema. Sin embargo está incompleta. Modifique el código para que se muestren los datos que faltan.
13. ¿Cuál es la diferencia entre las clases List y SynchList?
14. Modifique el caso de prueba simple del directorio threads para que se generen 5 hilos en lugar de 2.
15. Modifique el caso de prueba para que estos cinco hilos utilicen un semaforo inicializado en 3. Esto debe ocurrir solo si se define la macro de compilacion SEMAPHORE TEST.
16. Agregue al caso anterior una linea de depuracion que diga cuando cada hilo hace un P() y cuando un V(). La salida debe verse por pantalla solamente si se activa la bandera
    de depuracion correspondiente.
17. En threads se provee un caso de prueba que implementa el jardin ornamental. Sin embargo, el resultado es erroneo. Corrijalo de forma que se mantengan los cambios de
    contexto, sin agregar nuevas variables.
18. Replique el jardin ornamental en un nuevo caso de prueba. Revierta la solucion anterior y solucione el problema usando semaforos esta vez

## RESOLUCIONES

1. NACHOS prefiere la utilización de una CPU emulada en vez que usar directamente la CPU existente por varias razones:

- Permite crear un entorno controlado, lo que permite establecer los parametros y configuraciones necesarios para la simulación que se quiera dar.
- Facilita el proceso de debugear al proveer de ciertas herramientas para rastrear la ejecución de su código, comprender su comportamiento e identificar y corregir errores con mayor eficiencia.
- Da flexibilidad con respecto al hardware donde se encuentra NACHOS, el sistema no necesita adaptarse a cada tipo de computadora y se mantiene consistente en diferentes equipos.
- Simplifica el proceso de aprendisaje al no tener que lidiar con como hace el sistema operativo para adaptarse a cada hardware.

2. La maquina simulada se representa a partir de la clase Machine definida en el archivo machine.hh, donde se define a la memoria principal de la maquina como un array de
   caracteres
   ```
   char *mainMemory;
   ```
   Durante la ejecución de machine.cc se define al array de caracteres con el siguente tamaño
   ```
   unsigned memory_size = aNumPhysicalPages * PAGE_SIZE;
   mainMemory = new char [memory_size];
   ```
   En el archivo mmu.hh obtenemos el tamaño de las páginas es igual al tamaño de los sectores
   ```
   const unsigned PAGE_SIZE = SECTOR_SIZE;
   ```
   En el archivo disk.hh obtenemos que el tamaño de los sectores es de 128 bytes
   ```
   const unsigned SECTOR_SIZE = 128;
   ```
   Ahora queda obtener el valor de aNumPhysicalPages, el mismo se usa para definir el número de páginas físicas, el cual es una variable privada de la clase Machine
   ```
   unsigned numPhysicalPages;
   ```
   El valor se inicializa en el archivo system.cc
   ```
   int numPhysicalPages = DEFAULT_NUM_PHYS_PAGES;
   ```
   La macro se define en el archivo mmu.hh
   ```
   const unsigned DEFAULT_NUM_PHYS_PAGES = 32;
   ```
   Por lo que la cantidad de memoria dispoble para la máquina virtual es aNumPhysicalPages \* PAGE_SIZE = 4096 caracteres, que representarian 4096 bytes
3. Para cambiar modificar la cantidad de memoria de la maquina virtual, cambiaria la cantidad de páginas físicas a un valor diferente al default
   ```
   int numPhysicalPages = 40;
   ```
4. En el archivo disk.cc se define al tamaño del disco de la siguiente forma
   ```
   static const unsigned DISK_SIZE = MAGIC_SIZE + NUM_SECTORS * SECTOR_SIZE;
   ```
   A partir de los comentarios encritos en el archivo "disk.hh" podemos conocer que cada disco se representa separandolo en distintas pistas que a la vez se separan en sectors.
   ```
   /// The disk has a single surface, split up into “tracks”, and each track
   /// split up into "sectors" (the same number of sectors on each track, and
   /// each sector has the same number of bytes of storage).
   ```
   En el mismo archivo se declaran las siguientes variables y sus respectivos valores, que representarian las características de los discos de la máquina virtual
   ```
   SECTOR_SIZE = 128;
   SECTORS_PER_TRACK = 32;
   NUM_TRACKS = 32;
   NUM_SECTORS = SECTORS_PER_TRACK * NUM_TRACKS;
   ```
   Por lo que tendriamos que la cantidad de bytes de un sector es 128 bytes y que el número de sectores de un disco es la cantidad de sectores por pista por el número de pistas, osea 1024 sectores en total por disco. \
   Ahora queda el número mágico, el cual su tamañó se define como
   ```
   static const unsigned MAGIC_SIZE = sizeof (int);
   ```
   Por lo que finalmente tendriamos que el tamaño de un disco es el tamaño del número mágico más el número de sectores por el tamaño de cada sector, que daria 131076 bytes.
5. Para el manejo de las instrucciones, NACHOS simula las instrucciones de MIPS utilizando la clase Intructions definida en instruction.hh, la cual usa la variable opCode para
   diferenciarlas.

   ```
   class Instruction {
   public:

        /// Decode the binary representation of the instruction.
        void Decode();

        /// Retrieve the register number referred to in an instruction.
        int RegFromType(RegType reg) const;

        unsigned value;  //< Binary representation of the instruction.

        unsigned char opCode;  ///< Type of instruction.  This is NOT the same as
                            ///< the opcode field from the instruction: see
                            ///< defs in `encoding.hh`.
        unsigned char rs, rt, rd;  ///< Three registers from instruction.
        int extra;  ///< Immediate or target or shamt field or offset.
                    ///< Immediates are sign-extended.
   };
   ```

   En el archivo encoding.hh se enumaran todos los posibles valores de opCode, en total se hallan 63

   ```
   enum {
    OP_ADD      =  1,
    OP_ADDI     =  2,
    OP_ADDIU    =  3,
    OP_ADDU     =  4,
    OP_AND      =  5,
    OP_ANDI     =  6,
    OP_BEQ      =  7,
    OP_BGEZ     =  8,
    OP_BGEZAL   =  9,
    OP_BGTZ     = 10,
    OP_BLEZ     = 11,
    OP_BLTZ     = 12,
    OP_BLTZAL   = 13,
    OP_BNE      = 14,

    OP_DIV      = 16,
    OP_DIVU     = 17,
    OP_J        = 18,
    OP_JAL      = 19,
    OP_JALR     = 20,
    OP_JR       = 21,
    OP_LB       = 22,
    OP_LBU      = 23,
    OP_LH       = 24,
    OP_LHU      = 25,
    OP_LUI      = 26,
    OP_LW       = 27,
    OP_LWL      = 28,
    OP_LWR      = 29,

    OP_MFHI     = 31,
    OP_MFLO     = 32,

    OP_MTHI     = 34,
    OP_MTLO     = 35,
    OP_MULT     = 36,
    OP_MULTU    = 37,
    OP_NOR      = 38,
    OP_OR       = 39,
    OP_ORI      = 40,
    OP_RFE      = 41,
    OP_SB       = 42,
    OP_SH       = 43,
    OP_SLL      = 44,
    OP_SLLV     = 45,
    OP_SLT      = 46,
    OP_SLTI     = 47,
    OP_SLTIU    = 48,
    OP_SLTU     = 49,
    OP_SRA      = 50,
    OP_SRAV     = 51,
    OP_SRL      = 52,
    OP_SRLV     = 53,
    OP_SUB      = 54,
    OP_SUBU     = 55,
    OP_SW       = 56,
    OP_SWL      = 57,
    OP_SWR      = 58,
    OP_XOR      = 59,
    OP_XORI     = 60,
    OP_SYSCALL  = 61,

    OP_UNIMP    = 62,
    OP_RES      = 63,

    MAX_OPCODE  = 63
    };
   ```

   Pero se aclara que dos no pertenecen a las instrucciones de MIPS: OP_UNIMP y OP_RES, por lo que en realidad tendriamos 61

6. Podemos encontrar un función main() en los siguientes archivos, a recomendación del profesor ignoramos todos los archivos de la carpeta bin/

   - /code/threads/main.cc
   - code/userland/echo.c
   - code/userland/filetest.c
   - code/userland/halt.c
   - code/userland/matmult.c
   - code/userland/shell.c
   - code/userland/sort.c
   - code/userland/tinyshell.c
   - code/userland/touch.c
     COMPLETAR

7. Nombramos los archivos fuente en los que figuran las funciones y métodos llamados por el main de Nachos al ejecutarlo en el directorio threads, hasta dos niveles de profundidad:

   - En la función main se llama a la función Initialize, el cual se encuentra en el archivo system.cc
     - En la función Initialize se llama a la función ASSERT, el cual se encuentra en el archivo assert.hh
     - En la función Initialize se llama a la función ParseDebugOpts, el cual se encuentra en el archivo system.cc
     - En la función Initialize se llama a la función RandomInit, el cual se encuentra en el archivo system_dep.cc
     - En la función Initialize se llama a la función SetFlags, el cual se encuentra en el archivo debug.cc
     - En la función Initialize se llama a la función SetOpts, el cual se encuentra en el archivo debug.cc
     - En la función Initialize se llama al constructor Timer, el cual se encuentra en el archivo timer.cc
     - En la función Initialize se llama al constructor Thread, el cual se encuentra en el archivo thread.cc
     - En la función Initialize se llama a la función SetStatus, el cual se encuentra en el archivo thread.cc
     - En la función Initialize se llama a la función Enable, el cual se encuentra en el archivo interrupt.cc
     - En la función Initialize se llama a la función CallOnUserAbort, el cual se encuentra en el archivo interrupt.cc
     - En la función Initialize se llama al constructor Machine, el cual se encuentra en el archivo machine.cc
     - En la función Initialize se llama a la función SetExceptionHandlers, el cual se encuentra en el archivo exception.cc
     - En la función Initialize se llama al constructor SynchDisk, el cual se encuentra en el archivo synch_disk.cc
     - En la función Initialize se llama al constructor FileSystem, el cual se encuentra en el archivo fyle_system.cc
   - En la función main se llama a la función DEBUG, el cual se encuentra en el archivo utility.hh
     - En la función DEBUG se llama a la función Print, el cual se encuentra en el archivo debug.cc
   - En la función main se llama a la función SysInfo, el cual se encuentra en el archivo sys_info.cc
   - En la función main se llama a la función PrintVersion, el cual se encuentra en el archivo main.cc
   - En la función main se llama a la función ThreadTest, el cual se encuentra en el archivo thread_test.cc
     - En la función ThreadTest se llama a la función DEBUG, el cual se encuentra en el archivo utility.cc
     - En la función ThreadTest se llama a la función Choose, el cual se encuentra en el archivo thread_test.cc
     - En la función ThreadTest se llama a la función Run, el cual se encuentra en el archivo thread_test.cc
   - En la función main se llama a la función Halt, el cual se encuentra en el archivo interrupt.cc
   - En la función main se llama a la función StartProcess, el cual se encuentra en el archivo prog_test.cc
     - En la función StartProcess se llama a la función ASSERT, el cual se encuentra en el archivo assert.cc
     - En la función StartProcess se llama a la función Open, el cual se encuentra en el archivo file_system.cc
     - En la función StartProcess se llama al constructor AddressSpace, el cual se encuentra en el archivo address_space.cc
     - En la función StartProcess se llama a la función InitRegisters, el cual se encuentra en el archivo address_space.cc
     - En la función StartProcess se llama a la función RestoreState, el cual se encuentra en el archivo address_space.cc
     - En la función StartProcess se llama a la función Run, el cual se encuentra en el archivo thread_test.cc
   - En la función main se llama a la función ConsoleTest, el cual se encuentra en el archivo prog_test.cc
     - En la función ConsoleTest se llama al constructor Console, el cual se encuentra en el archivo console.cc
     - En la función ConsoleTest se llama al constructor Semaphore, el cual se encuentra en el archivo semaphore.cc
     - En la función ConsoleTest se llama a la función P, el cual se encuentra en el archivo semaphore.cc
     - En la función ConsoleTest se llama a la función GetChar, el cual se encuentra en el archivo console.cc
     - En la función ConsoleTest se llama a la función PutChar, el cual se encuentra en el archivo console.cc
   - En la función main se llama a la función ASSERT, el cual se encuentra en el archivo assert.hh
   - En la función main se llama a la función Copy, el cual se encuentra en el archivo fs_test.cc
     - En la función Copy se llama a la función ASSERT, el cual se encuentra en el archivo assert.hh
     - En la función Copy se llama a la función DEBUG, el cual se encuentra en el archivo debug.hh
     - En la función Copy se llama a la función Create, el cual se encuentra en el archivo fyle_system.hh
     - En la función Copy se llama a la función Open, el cual se encuentra en el archivo fyle_system.hh
     - En la función Copy se llama a la función Write, el cual se encuentra en el archivo open_file.cc
   - En la función main se llama a la función Print, el cual se encuentra en el archivo scheduler.cc
     - En la función Print se llama a la función Apply, el cual se encuentra en el archivo list.hh
   - En la función main se llama a la función Remove, el cual se encuentra en el archivo directory.cc
     - En la función Remove se llama a la función ASSERT, el cual se encuentra en el archivo assert.hh
     - En la función Remove se llama a la función FindIndex, el cual se encuentra en el archivo directory.cc
   - En la función main se llama a la función List, el cual se encuentra en el archivo directory.cc
   - En la función main se llama a la función Print \*, el cual se encuentra en el archivo directory.cc
     - En la función Print \* se llama a la función FetchFrom, el cual se encuentra en el archivo file_header.cc
   - En la función main se llama a la función Check, el cual se encuentra en el archivo fyle_system.cc
     - En la función Check se llama a la función DEBUG, el cual se encuentra en el archivo debug.hh
     - En la función Check se llama al constructor Bitmap, el cual se encuentra en el archivo bitmap.cc
     - En la función Check se llama a la función Mark, el cual se encuentra en el archivo file_header.cc
     - En la función Check se llama a la función GetRaw, el cual se encuentra en el archivo file_header.cc
     - En la función Check se llama a la función FetchFrom, el cual se encuentra en el archivo file_header.cc
     - En la función Check se llama a la función CheckForError, el cual se encuentra en el archivo fyle_system.cc
     - En la función Check se llama a la función CheckFileHeader, el cual se encuentra en el archivo fyle_system.cc
     - En la función Check se llama al constructor Directory, el cual se encuentra en el archivo directory.cc
     - En la función Check se llama a la función CheckDirectory, el cual se encuentra en el archivo fyle_system.cc
     - En la función Check se llama a la función CheckBitmaps, el cual se encuentra en el archivo fyle_system.cc
   - En la función main se llama a la función PerformanceTest, el cual se encuentra en el archivo fs_test.cc
     - En la función PerformanceTest se llama a la función Print, el cual se encuentra en el archivo statistics.cc
     - En la función PerformanceTest se llama a la función FileWrite, el cual se encuentra en el archivo fs_test
     - En la función PerformanceTest se llama a la función FileRead, el cual se encuentra en el archivo fs_test
     - En la función PerformanceTest se llama a la función Remove, el cual se encuentra en el archivo file_system.cc
   - En la función main se llama a la función Finish, el cual se encuentra en el archivo thread.cc
     - En la función Finish se llama a la función SetLevel, el cual se encuentra en el archivo interrupt.cc
     - En la función Finish se llama a la función ASSERT, el cual se encuentra en el archivo assert.hh
     - En la función Finish se llama a la función DEBUG, el cual se encuentra en el archivo debug.hh
     - En la función Finish se llama a la función GetName, el cual se encuentra en el archivo thread.cc
     - En la función Finish se llama a la función Sleep, el cual se encuentra en el archivo thread.cc

8. La macro ASSERT tiene un funcionamiento similar al definido en la libreria "assert.h" de c, es decir, chequea si la condicion es verdadera (en cuyo caso no hace nada), y en caso contrario corta el programa, muestra un mensaje con el lugar donde fallo

   Por otra parte, la macro DEBUG muestra en consola un mensaje junto a su flag (la cual debe estar prendida). Esto es util ya que se pueden poner mensajes en ciertas partes del codigo con flags especificas, las cuales solo se mostraran si al correr nachos se activan dichas flags (cosa util a la hora de debuggear)

9. Comente el efecto de las distintas banderas de depuración.

- `+` : Prende todos los mensajes de debug
- `t` : usada en el sistema de threads
- `s` : usada en semaforos, locks y variables de condicion
- `i` : usado en simulacion de interrupciones
- `m` : usada en simulacion de la maquina (requiere la bandera _USER_PROGRAM_).
- `d` : usada en simulacion de disco (requiere la bandera _FILESYS_).
- `f` : usada en el sistema de archivos (requiere la bandera _FILESYS_).
- `a` : usada en direcciones de memoria (requiere la bandera _USER_PROGRAM_).
- `e` : usada en handleo de excepciones (requiere la bandera _USER_PROGRAM_).

10. ¿Dónde están definidas las constantes USER PROGRAM, FILESYS NEEDED, FILESYS STUB? \
    Dichas constantes se definen en el makefile de userprog ("userprog/Makefile") en la variable DEFINE
11. En la función main principal, la que se encuentra en /code/threads/main.cc se definen todas las flags que admite NACHOS

- Opciones Generales
  - `-d <debugflags>` : hace que se impriman determinados mensajes de depuración.
  - `-do <debugopts>` : habilita opciones que modifican el comportamiento al imprimir mensajes de depuración
  - `-rs <random seed #>` : hace que el `Yield` se produzca en puntos aleatorios, pero repetibles.
  - `-z` : imprime la información sobre la versión y el copyright, y sale.
  - `-m <num phys pages>` : tamaño de la memoria física emulada en páginas.
  - `-i` : muestra la información del sistema.
- Opciones de Hilos
  - `-tt` : prueba el subsistema de subprocesos; se pide al usuario que elija una prueba a ejecutar de entre una colección de pruebas disponibles.
  - `-tn` : ejecuta la enésima prueba.
- Opciones de Programas de Usuario
  - `-s` : hace que los programas de usuario se ejecuten en modo de un solo paso.
  - `-x <nachos file>` : ejecuta un programa de usuario.
  - `-tc <consoleIn> <consoleOut>` : prueba la consola.
- Opciones del Sistema de Archivos
  - `-f` : hace que se formatee el disco físico.
  - `-cp <unix file> <nachos file>` : copia un fichero de UNIX a NACHOS.
  - `-pr <nachos file>` : imprime un archivo Nachos en la salida estándar.
  - `-rm <nachos file>` : elimina un archivo Nachos del sistema de archivos.
  - `-ls` : muestra el contenido del directorio NACHOS.
  - `-D` : imprime el contenido de todo el sistema de archivos.
  - `-c` : comprueba la integridad del sistema de archivos.
  - `-tf` : comprueba el rendimiento del sistema de archivos NACHOS.

12. Las modificaciones que deberemos hacer para obtener la informacion que falta sera utilizar las siguientes constantes:

- PAGE_SIZE, TLB_SIZE: definidas en "machine/mmu.hh" para el tamaño de pagina y el tamaño de la TLB
- numPhysicalPages: el cual si esta la bandera USER_PROGRAM obtendremos el dato del metodo GetNumPhysicalPages y en caso contrario de la constante DEFAULT_NUM_PHYS_PAGESd definida en "machine/mmu.hh"
- El tamaño de la memoria lo obtendremos de multiplicar numPhysicalPages \* PAGE_SIZE
- SECTOR_SIZE, SECTORS_PER_TRACK, NUM_TRACKS, NUM_SECTORS: declaradas en "machine/disk.hh"
- DISK_SIZE:

13. ¿Cuál es la diferencia entre las clases List y SynchList? \
    Antes de mencionar la diferencia entre ambas clases, veamos que ambas se comportan como listas simplemente enlazadas (para ser mas concreto, List es una lista simplemente enlazada, y luego SynchList utiliza un elemento de dicho tipo). Dicho esto, la diferencia entre ambas es que SynchList es una lista que soporta concurrencia.

14. Modifique el caso de prueba simple del directorio threads para que se generen 5 hilos en lugar de 2.

15. La razón por la cual el resultado es erroneo es debido a un problema de concurrencia entre los dos hilos que representan a los molinetes. El error agrede se puede ver en la función Turnstile:

```
Turnstile(void *n_)
{
  unsigned *n = (unsigned *)n_;

  for (unsigned i = 0; i < ITERATIONS_PER_TURNSTILE; i++)
  {
    int temp = count;
    currentThread->Yield();
    printf("Turnstile %u yielding with temp=%u.\n", *n, temp);
    printf("Turnstile %u back with temp=%u.\n", *n, temp);
    count = temp + 1;
    currentThread->Yield();
  }
  printf("Turnstile %u finished. Count is now %u.\n", *n, count);
  done[*n] = true;
}
```

Al momento de ejecutarse el jardín ornamental tenemos que el tramo de ejecución es:

- [Thread 1] int temp = count; / Si count = n, entonces ahora temp del Thread 1 es n
- [Thread 1] currentThread->Yield(); / la función simula el cambio de hilo que realiza el CPU
- [Thread 2] int temp = count; / Como count sigue siendo n, entonces temp del Thread 2 es n
- [Thread 2] currentThread->Yield(); / se vuelve a cambiar de hilo
- [Thread 1] printf("Turnstile %u yielding with temp=%u.\n", \*n, temp);
- [Thread 1] printf("Turnstile %u back with temp=%u.\n", \*n, temp);
- [Thread 1] count = temp + 1; / Se cambia count por n+1
- [Thread 1] currentThread->Yield(); / se vuelve a cambiar de hilo
- [Thread 2] printf("Turnstile %u yielding with temp=%u.\n", \*n, temp);
- [Thread 2] printf("Turnstile %u back with temp=%u.\n", \*n, temp);
- [Thread 2] count = temp + 1; / Se vuelve a guardar en count n+1
- [Thread 2] currentThread->Yield(); / se vuelve a cambiar de hilo

De tal forma el valor de count terminará siendo la mitad, debido a que en cada paso ambos molinetes obtienen el mismo valor de count y por lo tanto terminan guardando el mismo valor \
Para arreglar el problema sin cambiar el entorno, simplemente movemos el Yield una linea arriba, de tal forma el cambio de hilo se realiza antes de la declaración de temp y por lo tanto se obtiene el valor modificado por el otro hilo

```
Turnstile(void *n_)
{
  unsigned *n = (unsigned *)n_;

  for (unsigned i = 0; i < ITERATIONS_PER_TURNSTILE; i++)
  {
    currentThread->Yield();
    int temp = count;
    printf("Turnstile %u yielding with temp=%u.\n", *n, temp);
    printf("Turnstile %u back with temp=%u.\n", *n, temp);
    count = temp + 1;
    currentThread->Yield();
  }
  printf("Turnstile %u finished. Count is now %u.\n", *n, count);
  done[*n] = true;
}
```

- [Thread 1] currentThread->Yield(); / se vuelve a cambiar de hilo
- [Thread 2] currentThread->Yield(); / se vuelve a cambiar de hilo
- [Thread 1] int temp = count; / Si count = n, entonces ahora temp del Thread 1 es n
- [Thread 1] printf("Turnstile %u yielding with temp=%u.\n", \*n, temp);
- [Thread 1] printf("Turnstile %u back with temp=%u.\n", \*n, temp);
- [Thread 1] count = temp + 1; / Se cambia count por n+1
- [Thread 1] currentThread->Yield(); / se vuelve a cambiar de hilo
- [Thread 2] printf("Turnstile %u yielding with temp=%u.\n", \*n, temp);
- [Thread 2] printf("Turnstile %u back with temp=%u.\n", \*n, temp);
- [Thread 2] int temp = count; / Como count ahora es n+1, entonces temp del Thread 2 es n+1
- [Thread 2] count = temp + 1; / Ahora se guarda en count n+2
- [Thread 2] currentThread->Yield(); / se vuelve a cambiar de hilo

18.
