#include <immintrin.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "CImg.h"
#include <time.h> //Añadido para medir el tiempo

using namespace cimg_library;

#define ITEMS_PER_PACKET (sizeof(__m256)/sizeof(data_t))

// Data type for image components
typedef float data_t;
//Imagen original
const char* SOURCE_IMG      = "../Photos/normal/bailarina.bmp";
//Imagen con degradado
const char* SOURCE_IMG2      = "../Photos/backgrounds/background_V.bmp";
//Destino final de la imagen 
const char* DESTINATION_IMG = "../Photos/processed/filter_simd.bmp";

typedef struct {
    data_t *pRsrc;
    data_t *pGsrc;
    data_t *pBsrc;
    data_t *pRdst;
    data_t *pGdst;
    data_t *pBdst;
    int pixelCount;
} filter_args_t;

float saturationControl(float x, float y) {
	float res = ((256 * (255 - y))/(x+1));

	if(res > 255) {
		res = 255;
	} else if(res < 0) {
		res = 0;
	}

	return res;
}

/** Algoritmo para el filtro darken (#12) en versión SIMD
*/
void filter(filter_args_t args, filter_args_t args2) {
    int simdIterations = args.pixelCount / ITEMS_PER_PACKET;
    int seqIterations = args.pixelCount % ITEMS_PER_PACKET;

    __m256 num0 = _mm256_set1_ps(0);
    __m256 num1 = _mm256_set1_ps(1.0);
    __m256 num255 = _mm256_set1_ps(255.0);
    __m256 num256 = _mm256_set1_ps(256.0);
    __m256 vRsrc, vGsrc, vBsrc;
    __m256 vRsrc2, vGsrc2, vBsrc2;
    __m256 vRres, vGres, vBres;

    for(int i = 0; i < simdIterations; i++) {
        vRsrc = _mm256_loadu_ps(args.pRsrc + (i * ITEMS_PER_PACKET));
        vGsrc = _mm256_loadu_ps(args.pGsrc + (i * ITEMS_PER_PACKET));
        vBsrc = _mm256_loadu_ps(args.pBsrc + (i * ITEMS_PER_PACKET));
        vRsrc2 = _mm256_loadu_ps(args2.pRsrc + (i * ITEMS_PER_PACKET));
        vGsrc2 = _mm256_loadu_ps(args2.pGsrc + (i * ITEMS_PER_PACKET));
        vBsrc2 = _mm256_loadu_ps(args2.pBsrc + (i * ITEMS_PER_PACKET));
        
        vRres = _mm256_mul_ps(num256, _mm256_sub_ps(num255, vRsrc2));
        vRres = _mm256_div_ps(vRres, _mm256_add_ps(vRsrc, num1));
        vRres = _mm256_sub_ps(num255, vRres);
        vRres = _mm256_max_ps(num0, vRres);
        vRres = _mm256_min_ps(num255, vRres);
        
        vGres = _mm256_mul_ps(num256, _mm256_sub_ps(num255, vGsrc2));
        vGres = _mm256_div_ps(vGres, _mm256_add_ps(vGsrc, num1));
        vGres = _mm256_sub_ps(num255, vGres);
        vGres = _mm256_max_ps(num0, vGres);
        vGres = _mm256_min_ps(num255, vGres);

        vBres = _mm256_mul_ps(num256, _mm256_sub_ps(num255, vBsrc2));
        vBres = _mm256_div_ps(vBres, _mm256_add_ps(vBsrc, num1));
        vBres = _mm256_sub_ps(num255, vBres);
        vBres = _mm256_max_ps(num0, vBres);
        vBres = _mm256_min_ps(num255, vBres);

        _mm256_storeu_ps(args.pRdst + (i * ITEMS_PER_PACKET), vRres);
        _mm256_storeu_ps(args.pGdst + (i * ITEMS_PER_PACKET), vGres);
        _mm256_storeu_ps(args.pBdst + (i * ITEMS_PER_PACKET), vBres);
    }

	int simdOffset =  simdIterations * ITEMS_PER_PACKET;

	for(int i = 0; i < seqIterations; i++) {
		int start = simdOffset + i;
		args.pRdst[start] = 255 - saturationControl(args.pRsrc[start], args2.pRsrc[start]);
		args.pGdst[start] = 255 - saturationControl(args.pGsrc[start], args2.pGsrc[start]);
		args.pBdst[start] = 255 - saturationControl(args.pBsrc[start], args2.pBsrc[start]);
	}
}

int main() {
	// Open file and object initialization
	CImg<data_t> srcImage(SOURCE_IMG);
	CImg<data_t> srcImage2(SOURCE_IMG2);

	filter_args_t filter_args;
	filter_args_t filter_args2;
	data_t *pDstImage; // Pointer to the new image pixels

	// srcImage.display(); // Displays the source image
	// srcImage2.display();
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
	pDstImage = (data_t *)_mm_malloc(filter_args.pixelCount * nComp * sizeof(data_t), sizeof(__m256));
	if (pDstImage == NULL) {
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
	// dstImage.display();
	printf("Total time: %f", dElapsedTimeS);
	// Free memory
	_mm_free(pDstImage);

	return 0;
}