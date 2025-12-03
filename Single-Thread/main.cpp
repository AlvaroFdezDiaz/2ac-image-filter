/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include "CImg.h"
#include <time.h> //Añadido para medir el tiempo

using namespace cimg_library;

// Tipo de dato double
typedef double data_t;
//Imagen original
const char* SOURCE_IMG      = "../Photos/normal/bailarina.bmp";
//Imagen con degradado
const char* SOURCE_IMG2      = "../Photos/backgrounds/background_V.bmp";
//Destino final de la imagen 
const char* DESTINATION_IMG = "../Photos/processed/filter_monothread.bmp";

int saturationControl(int x, int y);

// Filter argument data type
typedef struct {
	// Punteros a las componentes R, G y B de la imagen source
	data_t *pRsrc;
	data_t *pGsrc;
	data_t *pBsrc;

	// Punteros a las componentes R, G y B de la imagen destino
	data_t *pRdst;
	data_t *pGdst;
	data_t *pBdst;

	uint pixelCount; // Número de píxeles de la imagen
} filter_args_t;

/***********************************************
 * 
 * Algoritmo. Filtro de imagen.
 * Problema 12 - Darken
 * Oscurece la imagen original en función de los valores de la segunda imagen
 * *********************************************/
void filter (filter_args_t args,filter_args_t args2) {
	for (uint i = 0; i < args.pixelCount; i++) {
		args.pRdst[i] = 255 - saturationControl(args.pRsrc[i], args2.pRsrc[i]);
		args.pGdst[i] = 255 - saturationControl(args.pGsrc[i], args2.pGsrc[i]);
		args.pBdst[i] = 255 - saturationControl(args.pBsrc[i], args2.pBsrc[i]);
	}
}

/*
 * Función de control de saturación para normalizar los valores de los píxeles al rango (0-255)
 */
int saturationControl(int x, int y) {
	int res = ((256 * (255 - y))/(x+1));

	if(res > 255) {
		res = 255;
	} else if(res < 0) {
		res = 0;
	}

	return res;
}

int main() {
	// Abrir imagen source con CImg
	CImg<data_t> srcImage(SOURCE_IMG);
	CImg<data_t> srcImage2(SOURCE_IMG2);

	filter_args_t filter_args;
	filter_args_t filter_args2;
	data_t *pDstImage; // Puntero a los píxeles de la imagen destino

	// Mostrar imágenes source
	srcImage.display();
	srcImage2.display(); 

	uint width = srcImage.width(); // Ancho de la imagen source
	uint height = srcImage.height(); // Alto de la imagen source
	uint nComp = srcImage.spectrum(); // Número de componentes de cada píxel de la imagen

	// Lo mismo con la segunda imagen source
	uint width2 = srcImage2.width();
	uint height2 = srcImage2.height();
	uint nComp2 = srcImage2.spectrum();

	// Comprobamos que el tamaño de las dos imágenes es el mismo
	if (width != width2 || height != height2 || nComp != nComp2) {
		printf("Error: Not same size images");
		return(-1);
	}

	// Cálculo del número de píxeles de la imagen
	filter_args.pixelCount = width * height;
	filter_args2.pixelCount = width2 * height2;
	
	// Reserva de memoria para las componentes de la imagen destino
	pDstImage = (data_t *) malloc (filter_args.pixelCount * nComp * sizeof(data_t));
	if (pDstImage == NULL) {
		perror("Allocating destination image");
		exit(-2);
	}

	// Punteros a las posiciones de las componentes R, G y B de la primera imagen source
	filter_args.pRsrc = srcImage.data();
	filter_args.pGsrc = filter_args.pRsrc + filter_args.pixelCount;
	filter_args.pBsrc = filter_args.pGsrc + filter_args.pixelCount;
	
	// Punteros a las posiciones de las componentes R, G y B de la segunda imagen source
	filter_args2.pRsrc = srcImage2.data();
	filter_args2.pGsrc = filter_args2.pRsrc + filter_args2.pixelCount;
	filter_args2.pBsrc = filter_args2.pGsrc + filter_args2.pixelCount;

	// Punteros a las posiciones de las componentes R, G y B de la imagen destino
	filter_args.pRdst = pDstImage;
	filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
	filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;

	// Variables para el cálculo del tiempo de ejecución
	struct timespec tStart, tEnd;
    double dElapsedTimeS;

	clock_gettime(CLOCK_REALTIME, &tStart);  // Inicio del contador de tiempo de ejecución

	/************************************************
	 * Algoritmo.
	 */
	filter(filter_args,filter_args2);

	clock_gettime(CLOCK_REALTIME, &tEnd);  // Fin del contador de tiempo de ejecución

	//Cálculo del tiempo transcurrido
	dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;
		
	// Creación de nueva imagen con CImg para almacenar la imagen destino
	CImg<data_t> dstImage(pDstImage, width, height, 1, nComp);

	// Guardar imagen destino en disco
	dstImage.save(DESTINATION_IMG); 

	// Mostrar imagen destino
	dstImage.display();
	
	// Imprimir tiempo total de procesamiento
	printf("Tiempo total de procesamiento: %.6f s\n", dElapsedTimeS);

	// Liberar memoria
	free(pDstImage);

	return 0;
}
