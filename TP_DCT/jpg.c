#include "matrice.h"
#include "dct.h"
#include "jpg.h"
#include "image.h"

/*
 * Calcul de la DCT ou de l'inverse DCT sur un petit carré de l'image.
 * On fait la transformation de l'image ``sur place'' c.a.d.
 * que le paramètre "image" est utilisé pour l'entrée et la sortie.
 *
 * DCT de l'image :  DCT * IMAGE * DCT transposée
 * Inverse        :  DCT transposée * I' * DCT
 */
void dct_image(int inverse, int nbe, float **image)
{
  float ** DCT;
  DCT = allocation_matrice_carree_float(nbe);
  coef_dct(nbe, DCT);

  float ** DCTtrans;
  DCTtrans = allocation_matrice_carree_float(nbe);
  transposition_matrice_carree(nbe, DCT, DCTtrans);

  float ** temp;
  temp = allocation_matrice_carree_float(nbe);

  if(inverse == 0) {
    produit_matrices_carrees_float(nbe, DCT, image, temp);
    produit_matrices_carrees_float(nbe, temp, DCTtrans, image);
  }
  else {
    produit_matrices_carrees_float(nbe, DCTtrans, image, temp);
    produit_matrices_carrees_float(nbe, temp, DCT, image);
    
  }
  liberation_matrice_carree_float(temp, nbe);
  liberation_matrice_carree_float(DCTtrans, nbe);
  liberation_matrice_carree_float(DCT, nbe);
}

/*
 * Quantification/Déquantification des coefficients de la DCT
 * Si inverse est vrai, on déquantifie.
 * Attention, on reste en calculs flottant (en sortie aussi).
 */
void quantification(int nbe, int qualite, float **extrait, int inverse)
{
  int i, j;
  float temp;
  for(i = 0; i < nbe; i++) {
    for(j = 0; j < nbe; j++) {
      temp = 1 + (i + j + 1) * qualite;
      extrait[i][j] = (inverse == 0) ? extrait[i][j] / temp : extrait[i][j] * temp;
    }
  }
}

/*
 * ZIGZAG.
 * On fournit à cette fonction les coordonnées d'un point
 * et elle nous donne le suivant (Toujours YX comme d'habitude)
 *
        x
   * +---+---+---+---+     +---+---+---+
   * |00 |01 |   |   |     |   |   |   |
  y* | ----/ | /---/ |     | ----/ | | |
   * |   |/  |/  |/  |     |   |/  |/| |
   * +---/---/---/---+     +---/---/-|-+
   * |10/|  /|  /|   |     |  /|  /| | |
   * | / | / | / | | |     | / | / | | |
   * | | |/  |/  |/| |     | | |/  |/  |
   * +-|-/---/---/-|-+     +-|-/---/---+
   * | |/|  /|  /| | |     | |/|  /|   |
   * | / | / | / | | |     | / | ----- |
   * |   |/  |/  |/  |     |   |   |   |
   * +---/---/---/---+     +---+---+---+
   * |  /|  /|  /|   |    
   * | /---/ | /---- |    
   * |   |   |   |   |    
   * +---+---+---+---+    
 */
static int montee = 1 ; // 0 = false et 1 = true
void zigzag(int nbe, int *y, int *x)
{
  int ligne = *y; int colonne = *x;

  if(montee == 1) {
    if(ligne != 0 && colonne != nbe - 1) {
      ligne--;
      colonne++;
    }
    else {
      montee = 0;
      if(ligne == 0 && colonne != nbe - 1)
        colonne++;
      else 
        ligne++;
    }
  }
  else {
    if(colonne != 0 && ligne != nbe - 1) {
      colonne--;
      ligne++;
    }
    else {
      montee = 1;
      if(colonne == 0 && ligne != nbe - 1)
        ligne++;
      else 
        colonne++;
    }
  }

  *x = colonne; *y = ligne;
}

/*
 * Extraction d'une matrice de l'image (le résultat est déjà alloué).
 * La sous-image carrée à la position et de la taille indiquée
 * est stockée dans matrice "extrait"
 */
static void extrait_matrice(int y, int x, int nbe
			    , const struct image *entree
			    , float **extrait
			    )
 {
  int i, j ;

  for(j=0;j<nbe;j++)
    for(i=0;i<nbe;i++)
      if ( j+y < entree->hauteur && i+x < entree->largeur )
	extrait[j][i] = entree->pixels[j+y][i+x] ;
      else
	extrait[j][i] = 0 ;
 }

/*
 * Insertion d'une matrice de l'image.
 * C'est l'opération inverse de la précédente.
 */

static void insert_matrice(int y, int x, int nbe
			   , float **extrait
			   , struct image *sortie
			   )
 {
  int i, j ;

  for(j=0;j<nbe;j++)
    for(i=0;i<nbe;i++)
      if ( j+y < sortie->hauteur && i+x < sortie->largeur )
	{
	  if ( extrait[j][i] < 0 )
	    sortie->pixels[j+y][i+x] = 0 ;
	  else
	    {
	      if ( extrait[j][i] > 255 )
		sortie->pixels[j+y][i+x] = 255 ;
	      else
		sortie->pixels[j+y][i+x] = rint(extrait[j][i]) ;
	    }
	}
 }


/*
 * Compression d'une l'image :
 * Pour chaque petit carré on fait la dct et l'on stocke dans un fichier
 */
void compresse_image(int nbe, const struct image *entree, FILE *f)
 {
  static float **tmp = NULL ;
  int i, j, k ;

  if ( tmp == NULL )
    {
      tmp = allocation_matrice_carree_float(nbe) ;
    }

  for(j=0;j<entree->hauteur;j+=nbe)
    for(i=0;i<entree->largeur;i+=nbe)
      {
	extrait_matrice(j, i, nbe, entree, tmp) ;
	dct_image(0, nbe, tmp) ;
	for(k=0; k<nbe; k++)
	  assert(fwrite(tmp[k], sizeof(**tmp), nbe, f) == nbe) ;
      }
 }

/*
 * Décompression image
 * On récupère la DCT de chaque fichier, on fait l'inverse et
 * on insère dans l'image qui est déjà allouée
 */
void decompresse_image(int nbe, struct image *entree, FILE *f)
 {
  static float **tmp = NULL ;
  int i, j, k ;

  if ( tmp == NULL )
    {
      tmp = allocation_matrice_carree_float(nbe) ;
    }

  for(j=0;j<entree->hauteur;j+=nbe)
    for(i=0;i<entree->largeur;i+=nbe)
      {
	for(k=0; k<nbe; k++)
	  assert(fread(tmp[k], sizeof(**tmp), nbe, f) == nbe) ;
	dct_image(1, nbe, tmp) ;
	insert_matrice(j, i, nbe, tmp, entree) ;
      }
 }
