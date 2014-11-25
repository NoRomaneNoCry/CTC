#include "image.h"


/*
 * Lecture d'une ligne du fichier.
 * On saute les lignes commençant par un "#" (commentaire)
 * On simplifie en considérant que les lignes ne dépassent pas MAXLIGNE
 */

void lire_ligne(FILE *f, char *ligne)
{
	while(fgets(ligne, MAXLIGNE, f) != NULL) {
		if(ligne[0] != '#')
			break ;
	}
	ligne = NULL;
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
	struct image * image_lue;
	char * ligne;
	ALLOUER(ligne, MAXLIGNE);
	int largeur, hauteur, i, j;

	/** Initialisation de la structure */
	lire_ligne(f, ligne);
	lire_ligne(f, ligne);
	largeur = atoi(strtok(ligne, " "));
	hauteur = atoi(strtok(NULL, " "));
	image_lue = allocation_image(hauteur, largeur);
	lire_ligne(f, ligne);

	/** Récupération des informations des pixels */
	for(i = 0; i < hauteur; i++) {
		for(j = 0; j < largeur; j++) {
			image_lue->pixels[i][j] = fgetc(f);
		}
	}
	return image_lue ;
}

/*
 * Écriture de l'image (toujours au format PGM)

 */

void ecriture_image(FILE *f, const struct image *image)
{
	int i, j;
	fprintf(f, "P5\n%d %d\n255\n", image->largeur, image->hauteur);

	for(i = 0; i < image->hauteur; i++) {
		for(j = 0; j < image->largeur; j++) {
			fputc(image->pixels[i][j], f);
		}
	}
}
