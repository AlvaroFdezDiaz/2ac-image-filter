/*
 * Main.cpp
 *
 *  Created on: Fall 2019
 */

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <CImg.h>

using namespace cimg_library;

// Data type for image components
// FIXME: Change this type according to your group assignment
typedef double data_t;
//Imagen original
const char* SOURCE_IMG      = "../Photos/normal/bailarina.bmp";
//Imagen con degradado
const char* SOURCE_IMG2      = "../Photos/backgrounds/background_V.bmp";
//Destino final de la imagen 
const char* DESTINATION_IMG = "../Photos/processed/bailarina_con_filtro_monohilo.bmp";
// Hilo de ejecución
pthread_t mi_hilo;

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


int main() {

	/***************************************************
	 * TODO: Variables initialization.
	 *   - Prepare variables for the algorithm
	 *   - This is not included in the benchmark time
	 */
	// Abrir fichero e inicializar objetos
	CImg<data_t> srcImage(SOURCE_IMG);
	CImg<data_t> srcImage2(SOURCE_IMG2);

	filter_args_t filter_args;
	filter_args_t filter_args2;
	data_t *pDstImage; // Puntero a los nuevos píxeles de la imagen
	data_t *pDstImage2;

	srcImage.display(); // Displays de las imagenes
	srcImage2.display();
	uint width = srcImage.width();// Obtener datos de la imagen original
	uint height = srcImage.height();	
	uint nComp = srcImage.spectrum();// número de componentes de la imagen original
	//Segunda imagen
	uint width2 = srcImage2.width();
	uint height2 = srcImage2.height();
	uint nComp2 = srcImage2.spectrum();

	//Comprobación sobre el tamaño de las imágenes
	if (width != width2 || height != height2 || nComp != nComp2) {
		printf("Error: Not same size images");
		return(-1);
	}

	// Calculando el tamaño de la imagen en píxeles
	filter_args.pixelCount = width * height;
	filter_args2.pixelCount = width2 * height2;
	
	// Reservar espacio de memoria para los componentes de la imagen destino
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

	// Punteros a los arrays de componentes de la imagen original
	filter_args.pRsrc = srcImage.data(); // pRcomp apunta al array del componente R
	filter_args.pGsrc = filter_args.pRsrc + filter_args.pixelCount; // pGcomp apunta al array del componente G
	filter_args.pBsrc = filter_args.pGsrc + filter_args.pixelCount; // pBcomp apunta al array del componente B
	//Segunda imagen
	filter_args2.pRsrc = srcImage2.data();
	filter_args2.pGsrc = filter_args2.pRsrc + filter_args2.pixelCount;
	filter_args2.pBsrc = filter_args2.pGsrc + filter_args2.pixelCount;
	// Punteros a los arrays RGB de la imagen destino
	filter_args.pRdst = pDstImage;
	filter_args.pGdst = filter_args.pRdst + filter_args.pixelCount;
	filter_args.pBdst = filter_args.pGdst + filter_args.pixelCount;
	// Variables para el tiempo
	struct timespec tStart, tEnd;
    double dElapsedTimeS;

	/***********************************************
	 * TODO: Algorithm start.
	 *   - Measure initial time
	 */
	clock_gettime(CLOCK_REALTIME, &tStart);  // Inicio del contador de tiempo de ejecución

	/************************************************
	 * TODO: Algorithm.
	 */
		for (uint i = 0; i < args.pixelCount; i++) {
		*(args.pRdst + i) = 255 - (256 * (255 - *(args2.pGsrc + i))/(*(args.pGsrc + i)+1));
		*(args.pGdst + i) = 255 - (256 * (255 - *(args2.pBsrc + i))/(*(args.pBsrc + i)+1));
		*(args.pBdst + i) = 255 - (256 * (255 - *(args2.pRsrc + i))/(*(args.pRsrc + i)+1));
	}


	/***********************************************
	 * TODO: End of the algorithm.
	 *   - Measure the end time
	 *   - Calculate the elapsed time
	 */
	clock_gettime(CLOCK_REALTIME, &tEnd);  // Fin del contador de tiempo de ejecución

	//Cálculo del tiempo transcurrido
	dElapsedTimeS = (tEnd.tv_sec - tStart.tv_sec) + (tEnd.tv_nsec - tStart.tv_nsec) / 1e+9;
	// Crear un nuevo objeto de imagen con los píxeles calculados
	// En caso de imágenes en color normales usar nComp=3,
	CImg<data_t> dstImage(pDstImage, width, height, 1, nComp);

	// Guardar la imagen de destino en disco
	dstImage.save(DESTINATION_IMG); 

	// Mostrar la imagen de destino
	dstImage.display();
	
	// Liberar memoria
	free(pDstImage);


	return 0;
}
void* NombreDeTuFuncion(void* arg) {
    // --- AQUÍ PONES TU LÓGICA ---
    // El código que pongas aquí se ejecutará en paralelo.
    
    return NULL; 
}
