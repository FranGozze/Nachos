# Ejercicio 2

### Tabla Hitrate

| Entradas | matmult                        | sort                               |
| -------- | ------------------------------ | ---------------------------------- |
| 4        | 719330 of 781960 (91.990639 %) | 21559270 of 22880818 (94.224205 %) |
| 32       | 690887 of 690994 (99.984512 %) | 21027323 of 21030253 (99.986076 %) |
| 64       | 690837 of 690883 (99.993340 %) | 21024776 of 21024815 (99.999809 %) |

Donde, como se puede apreciar, a mayor tamaño de la TLB, mejor nos da un mejor hit ratio, es por eso que sugirimos usar un tamaño de 64 entradas

# Ejercicio 6

### Tabla Hitrate Sort

|                       | Clock                              | FIFO                               | Random                             |
| --------------------- | ---------------------------------- | ---------------------------------- | ---------------------------------- |
| Unico proceso         | 10889123 of 12204521 (89.222046 %) | 10805969 of 12121851 (89.144547 %) | 11155276 of 12471267 (89.447815 %) |
| Procesos Sucesivos    | 12415838 of 13732631 (90.411209 %) | 11561751 of 12877850 (89.780136 %) | 11170164 of 12505448 (89.322380 %) |
| Procesos Concurrentes | 12416702 of 13733542 (90.411499 %) | 11562615 of 12878761 (89.780495 %) | 11171028 of 12506359 (89.322784 %) |

### Tabla Hitrate Matmult

|                       | Clock                          | FIFO                           | Random                         |
| --------------------- | ------------------------------ | ------------------------------ | ------------------------------ |
| Unico proceso         | 737549 of 801914 (91.973579 %) | 911731 of 991193 (91.983192 %) | 719758 of 782427 (91.990433 %) |
| Procesos Sucesivos    | 737215 of 801599 (91.968056 %) | 737287 of 801672 (91.968658 %) | 720058 of 782816 (91.983047 %) |
| Procesos Concurrentes | 738916 of 803362 (91.977959 %) | 738988 of 803435 (91.978569 %) | 721759 of 784579 (91.993156 %) |

Donde, como se puede apreciar, en ambos problemas el mejor resultado es el de aplicar el algoritmo Clock mejorado en terminos generales.
