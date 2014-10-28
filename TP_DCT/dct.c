#include "bases.h"
#include "matrice.h"
#include "dct.h"

/*
 * La fonction calculant les coefficients de la DCT (et donc de l'inverse)
 * car la matrice de l'inverse DCT est la transposée de la matrice DCT
 *
 * Cette fonction prend beaucoup de temps.
 * il faut que VOUS l'utilisiez le moins possible (UNE SEULE FOIS)
 *
 * FAITES LES CALCULS EN "double"
 *
 * La valeur de Pi est : M_PI
 *
 * Pour ne pas avoir de problèmes dans la suite du TP, indice vos tableau
 * avec [j][i] et non [i][j].
 */

void coef_dct(int nbe, float **table)
{
	int i, j;
	double sqrt_n = sqrt(nbe);
	double sqrt_2 = sqrt(2);
	
	for(i = 0; i < nbe; i++)
		table[0][i] = 1.f / sqrt_n;
		
	for(i = 0; i < nbe; i++) {
		for(j = 1; j < nbe; j++) {
			table[j][i] = (sqrt_2/sqrt_n) * cos(j*M_PI*(2*i + 1)/(2*nbe));
		}
	}
}

/*
 * La fonction calculant la DCT ou son inverse.
 *
 * Cette fonction va être appelée très souvent pour faire
 * la DCT du son ou de l'image (nombreux paquets).
 */

void dct(int   inverse,		/* ==0: DCT, !=0 DCT inverse */
	 int nbe,		/* Nombre d'échantillons  */
	 const float *entree,	/* Le son avant transformation (DCT/INVDCT) */
	 float *sortie		/* Le son après transformation */
	 )
{
	float ** DCT ;
	DCT = allocation_matrice_carree_float(nbe);
	//transposition_matrice_carree(int nbe, float **a, float **a_t)
	//produit_matrice_carree_vecteur(int nbe, float **m, const float *v, float *resultat)
	
	liberation_matrice_carree_float(DCT, nbe);
}
