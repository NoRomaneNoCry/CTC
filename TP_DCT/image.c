#include "image.h"


/*
 * Lecture d'une ligne du fichier.
 * On saute les lignes commençant par un "#" (commentaire)
 * On simplifie en considérant que les lignes ne dépassent pas MAXLIGNE
 */

void lire_ligne(FILE *f, char *ligne)
{
	size_t maxligne = MAXLIGNE;
	do {
		int i = getline(&ligne, &maxligne, f);
	} while(ligne[0] == '#');
}

/*
 * Allocation d'une image
 */

struct image* allocation_image(int hauteur, int largeur)
{
	struct image * struct_retour;
	int i;

	ALLOUER(struct_retour, 1);
	struct_retour->hauteur = hauteur;
	struct_retour->largeur = largeur;
	ALLOUER(struct_retour->pixels, hauteur);
	for(i = 0; i < hauteur; i++)
		ALLOUER(struct_retour->pixels[i], largeur);

	return struct_retour;
}

/*
 * Libération image
 */

void liberation_image(struct image* image)
{
	int i;
	for(i = 0; i < image->hauteur; i++)
		free(image->pixels[i]);
	free(image->pixels);
	free(image);
}

/*
 * Allocation et lecture d'un image au format PGM.
 * (L'entête commence par "P5\nLargeur Hauteur\n255\n"
 * Avec des lignes de commentaire possibles avant la dernière.
 */

struct image* lecture_image(FILE *f)
{






































return 0 ; /* pour enlever un warning du compilateur */
}

/*
 * Écriture de l'image (toujours au format PGM)
 */

void ecriture_image(FILE *f, const struct image *image)
{










}
