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

// Data type for image components
typedef double data_t; // Cambiado a tipo de dato double
//Imagen original
const char* SOURCE_IMG      = "../Photos/normal/bailarina.bmp";
//Imagen con degradado
const char* SOURCE_IMG2      = "../Photos/backgrounds/background_V.bmp";
//Destino final de la imagen 
const char* DESTINATION_IMG = "../Photos/processed/filter_monothread.bmp";

int saturationControl(int x, int y);

// Filter argument data type
typedef struct {
	data_t *pRsrc; // Pointers to the R, G and B components
	data_t *pGsrc;
	data_t *pBsrc;
	data_t *pRdst;
	data_t *pGdst;
	data_t *pBdst;
	uint pixelCount; // Size of the image in pixels
} filter_args_t;

/***********************************************
 * 
 * Algorithm. Image filter.
 * Problema 12 - Darken
 * Oscurece la imagen original en función de los valores de la segunda imagen
 * *********************************************/
void filter (filter_args_t args,filter_args_t args2) {
	for (uint i = 0; i < args.pixelCount; i++) {
		*(args.pRdst + i) = 255 - (256 * (255 - *(args2.pGsrc + i))/(*(args.pGsrc + i)+1));
		*(args.pGdst + i) = 255 - (256 * (255 - *(args2.pBsrc + i))/(*(args.pBsrc + i)+1));
		*(args.pBdst + i) = 255 - (256 * (255 - *(args2.pRsrc + i))/(*(args.pRsrc + i)+1));
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
	// Open file and object initialization
	CImg<data_t> srcImage(SOURCE_IMG);
	CImg<data_t> srcImage2(SOURCE_IMG2);

	filter_args_t filter_args;
	filter_args_t filter_args2;
	data_t *pDstImage; // Pointer to the new image pixels
	data_t *pDstImage2;

	srcImage.display(); // Displays the source image
	srcImage2.display();
	uint width = srcImage.width();// Getting information from the source image
	uint height = srcImage.height();	
	uint nComp = srcImage.spectrum();// source image number of components
	         // Common values for spectrum (number of image components):
				//  B&W images = 1
				//	Normal color images = 3 (RGB)
				//  Special color images = 4 (RGB and alpha/transparency channel)
	//Segunda imagen
	uint width2 = srcImage2.width();
	uint height2 = srcImage2.height();
	uint nComp2 = srcImage2.spectrum();

	//Comprobación sobre el tamaño de las imágenes
	if (width != width2 || height != height2 || nComp != nComp2) {
		printf("Error: Not same size images");
		return(-1);
	}

	// Calculating image size in pixels
	filter_args.pixelCount = width * height;
	filter_args2.pixelCount = width2 * height2;
	
	// Allocate memory space for destination image components
	pDstImage = (data_t *) malloc (filter_args.pixelCount * nComp * sizeof(data_t));
	if (pDstImage == NULL) {
		perror("Allocating destination image");
		exit(-2);
	}
	//Segunda imagen
	pDstImage2 = (data_t *) malloc (filter_args2.pixelCount * nComp2 * sizeof(data_t));
	if (pDstImage2 == NULL) {
		perror("Allocating destination image");
		exit(-2);
	}

	// Pointers to the componet arrays of the source image
	filter_args.pRsrc = srcImage.data(); // pRcomp points to the R component array
	filter_args.pGsrc = filter_args.pRsrc + filter_args.pixelCount; // pGcomp points to the G component array
	filter_args.pBsrc = filter_args.pGsrc + filter_args.pixelCount; // pBcomp points to B component array
	//Segunda imagen
	filter_args2.pRsrc = srcImage2.data();
	filter_args2.pGsrc = filter_args2.pRsrc + filter_args2.pixelCount;
	filter_args2.pBsrc = filter_args2.pGsrc + filter_args2.pixelCount;
	// Pointers to the RGB arrays of the destination image
	filter_args.pRdst = pDstImage;
	filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
	filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;
	// Variables para el tiempo
	struct timespec tStart, tEnd;
    double dElapsedTimeS;


	clock_gettime(CLOCK_REALTIME, &tStart);  // Inicio del contador de tiempo de ejecución

	/************************************************
	 * Algorithm.
	 */
	filter(filter_args,filter_args2);

	clock_gettime(CLOCK_REALTIME, &tEnd);  // Fin del contador de tiempo de ejecución

	//Cálculo del tiempo transcurrido
	dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;
		
	// Create a new image object with the calculated pixels
	// In case of normal color images use nComp=3,
	// In case of B/W images use nComp=1.
	CImg<data_t> dstImage(pDstImage, width, height, 1, nComp);

	// Store destination image in disk
	dstImage.save(DESTINATION_IMG); 

	// Display destination image
	dstImage.display();
	
	// Free memory
	free(pDstImage);

	return 0;
}
