/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "CImg.h"
#include <thread>
#include <stdlib.h>
#include <time.h>

using namespace cimg_library;

// Tipo de dato double
typedef double data_t;
// Imagen original
const char *SOURCE_IMG = "../Photos/normal/bailarina.bmp";
// Background
const char *SOURCE_IMG2 = "../Photos/backgrounds/background_V.bmp";
// Destino final de la imagen
const char *DESTINATION_IMG = "../Photos/processed/filter_multithread.bmp";
// Hilo de ejecución
pthread_t *thread;

int saturationControl(int x, int y);
void *thread_filter(void *arg);
// Filter argument data type
typedef struct {
    // Punteros a los componentes R, G y B de la imagen source
    data_t *pRsrc;
    data_t *pGsrc;
    data_t *pBsrc;

    // Punteros a los componentes R, G y B de la imagen destino
    data_t *pRdst;
    data_t *pGdst;
    data_t *pBdst;

    uint pixelCount; // Tamaño de la imagen en píxeles
} filter_args_t;

typedef struct { // Struct para pasar los datos a cada hilo
    filter_args_t args1;
    filter_args_t args2;
    uint start;
    uint end;
} ThreadData;

int main() {
    // Miramos cuántos hilos tiene el procesador antes de la ejecución del algoritmo
    unsigned int NUM_THREADS = std::thread::hardware_concurrency();

    // Abrir ficheros de imagen e inicializar objetos
    CImg<data_t> srcImage(SOURCE_IMG);
    CImg<data_t> srcImage2(SOURCE_IMG2);

    filter_args_t filter_args;
    filter_args_t filter_args2;
    data_t *pDstImage; // Puntero a los  píxeles de la imagen destino

    // Mostrar las imágenes de origen
    srcImage.display();
    srcImage2.display();
	
    // Datos de la primera imagen
    uint width = srcImage.width();
    uint height = srcImage.height();
    uint nComp = srcImage.spectrum();

    // Datos de la segunda imagen
    uint width2 = srcImage2.width();
    uint height2 = srcImage2.height();
    uint nComp2 = srcImage2.spectrum();

    // Comprobación sobre el tamaño de las imágenes
    if (width != width2 || height != height2 || nComp != nComp2) {
        printf("Error: Not same size images");
        return (-1);
    }

    // Calculando el tamaño de la imagen en píxeles
    filter_args.pixelCount = width * height;
    filter_args2.pixelCount = width2 * height2;

    // Reserva de espacio de memoria para los componentes de la imagen destino
    pDstImage = (data_t *)malloc(filter_args.pixelCount * nComp * sizeof(data_t));
    if (pDstImage == NULL) {
        perror("Allocating destination image");
        exit(-2);
    }

    // Punteros a los arrays de componentes de la imagen original
    filter_args.pRsrc = srcImage.data();
    filter_args.pGsrc = filter_args.pRsrc + filter_args.pixelCount;
    filter_args.pBsrc = filter_args.pGsrc + filter_args.pixelCount;

    // Punteros a los componentes de la segunda imagen
    filter_args2.pRsrc = srcImage2.data();
    filter_args2.pGsrc = filter_args2.pRsrc + filter_args2.pixelCount;
    filter_args2.pBsrc = filter_args2.pGsrc + filter_args2.pixelCount;

    // Punteros a los componentes de la imagen destino
    filter_args.pRdst = pDstImage;
    filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
    filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;

    // Variables para el tiempo
    struct timespec tStart, tEnd;
    double dElapsedTimeS;

    // Calculamos el número de píxeles que procesará cada hilo
    uint pixelsPerThread = filter_args.pixelCount / NUM_THREADS;

    // Creacion de variables para usar luego en el algoritmo
    ThreadData *thread_data;
    thread = (pthread_t *)malloc(NUM_THREADS * sizeof(pthread_t));
    thread_data = (ThreadData *)malloc(NUM_THREADS * sizeof(ThreadData));

    clock_gettime(CLOCK_REALTIME, &tStart); // Inicio del contador de tiempo de ejecución

    for (int i = 0; i < NUM_THREADS; i++) {
        // Copiamos los punteros originales
        thread_data[i].args1 = filter_args;
        thread_data[i].args2 = filter_args2;

        // Calculamos el rango de índices absolutos
        thread_data[i].start = i * pixelsPerThread;

        // Comprovamos por si la division no es exacta
        if (i == NUM_THREADS - 1) {
            thread_data[i].end = filter_args.pixelCount;
        } else {
            thread_data[i].end = (i + 1) * pixelsPerThread;
        }

        // Lanzamos el hilo y le decimos que ejecute la funcion
        pthread_create(
            &thread[i];             // ID del hilo
            NULL,                   // Atributos
            thread_filter,          // Función a ejecutar
            (void *)&thread_data[i] // Argumento
        );
    }

    // Esperamos a que acaben todos los hilos
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    clock_gettime(CLOCK_REALTIME, &tEnd); // Fin del contador de tiempo de ejecución

    // Cálculo del tiempo transcurrido
    dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;

    // Crear un nuevo objeto de imagen con los píxeles calculados
    // En caso de imágenes en con componentes RGB, usar nComp=3
    CImg<data_t> dstImage(pDstImage, width, height, 1, nComp);

    // Guardar la imagen de destino en disco
    dstImage.save(DESTINATION_IMG);

    // Mostrar la imagen de destino
    dstImage.display();

	//Imprimir tiempo total de procesamiento
	printf("Tiempo total de procesamiento: %.6f s\n", dElapsedTimeS);

    // Liberar memoria
    free(pDstImage);
    free(thread);
    free(thread_data);

    return 0;
}

void *thread_filter(void *arg) {
    // 1. Desempaquetar (Cast)
    ThreadData *data = (ThreadData *)arg;

    for (uint i = data->start; i < data->end; i++) {
        data->args1.pRdst[i] = 255 - saturationControl(data->args1.pRsrc[i], data->args2.pRsrc[i]);
        data->args1.pGdst[i] = 255 - saturationControl(data->args1.pGsrc[i], data->args2.pGsrc[i]);
        data->args1.pBdst[i] = 255 - saturationControl(data->args1.pBsrc[i], data->args2.pBsrc[i]);
    }

    pthread_exit(NULL); // Indica que el hilo ha terminado
}

/**
 * Algoritmo de control de saturación
 */
int saturationControl(int x, int y) {
    int res = ((256 * (255 - y)) / (x + 1));

    if (res > 255) {
        res = 255;
    }
    else if (res < 0) {
        res = 0;
    }

    return res;
}
