/*
 * Le but du shannon-fano dynamique est de ne pas transmettre la table
 * des occurrences. Pour ce faire, on ajoute à la table un symbole ESCAPE
 * qui permet l'ajout d'éléments à la table.
 * Le décompresseur sait qu'après un événement ESCAPE il trouvera
 * la valeur (et non le code) d'un événement à ajouter à la table.
 */


#include "bits.h"
#include "sf.h"

#define VALEUR_ESCAPE 0x7fffffff /* Plus grand entier positif */

struct evenement
 {
  int valeur ;
  int nb_occurrences ;
 } ;

struct shannon_fano
 {
  int nb_evenements ;
  struct evenement evenements[200000] ;
 } ;

/*
 * Allocation des la structure et remplissage des champs pour initialiser
 * le tableau des événements avec l'événement ESCAPE (avec une occurrence).
 */
struct shannon_fano* open_shannon_fano()
{
	struct shannon_fano * struct_retour ;
	ALLOUER(struct_retour, 1);
	
	struct_retour->nb_evenements = 1;
	struct_retour->evenements[0].valeur = VALEUR_ESCAPE;
	struct_retour->evenements[0].nb_occurrences = 1;

	return struct_retour;
}

/*
 * Fermeture (libération mémoire)
 */
void close_shannon_fano(struct shannon_fano *sf)
{
	free(sf);
}

/*
 * En entrée l'événement (sa valeur, pas son code shannon-fano).
 * En sortie la position de l'événement dans le tableau "evenements"
 * Si l'événement n'est pas trouvé, on retourne la position
 * de l'événement ESCAPE.
 */

static int trouve_position(const struct shannon_fano *sf, int evenement)
{
	int i;
	for(i = 0; i < sf->nb_evenements; i++) {
		if(sf->evenements[i].valeur == evenement)
			return i;
	}
	return trouve_position(sf, VALEUR_ESCAPE);
}

/*
 * Soit le sous-tableau "evenements[position_min..position_max]"
 * Les bornes sont incluses.
 *
 * LES FORTES OCCURRENCES SONT LES PETITS INDICES DU TABLEAU
 *
 * Il faut trouver la séparation.
 * La séparation sera codée comme l'indice le plus fort
 * des fortes occurrences.
 *
 * La séparation minimise la valeur absolue de la différence
 * de la somme des occurrences supérieures et inférieures.
 *
 * L'algorithme (trivial) n'est pas facile à trouver, réfléchissez bien.
 */
static int trouve_separation(const struct shannon_fano *sf
			     , int position_min
			     , int position_max)
{
	int i = position_min;
	int j = position_max;
	int occur_i = sf->evenements[i].nb_occurrences;
	int occur_j = sf->evenements[j].nb_occurrences;
	
	while(i != j) {
		if(occur_j < occur_i) {
			j--;
			occur_j += sf->evenements[j].nb_occurrences;
		}
		else {
			i++;
			occur_i += sf->evenements[i].nb_occurrences;
		}
	}
	
	return (occur_i <= occur_j )? i : i - 1;
}

/*
 * Cette fonction (simplement itérative)
 * utilise "trouve_separation" pour générer les bons bit dans "bs"
 * le code de l'événement "sf->evenements[position]".
 */

static void encode_position(struct bitstream *bs,struct shannon_fano *sf,
		     int position)
{
	int borneMin = 0;
	int borneMax = sf->nb_evenements - 1;
	int separation;
	
	while(borneMin < borneMax) {
		
		separation = trouve_separation(sf, borneMin, borneMax);
		if(position <= separation) {
			borneMax = separation;
			put_bit(bs, 0);
		}
		else {
			borneMin = separation + 1;
			put_bit(bs, 1);
		}
	}
}

/*
 * Cette fonction incrémente le nombre d'occurrence de
 * "sf->evenements[position]"
 * Puis elle modifie le tableau pour qu'il reste trié par nombre
 * d'occurrence (un simple échange d'événement suffit)
 *
 * Les faibles indices correspondent aux grand nombres d'occurrences
 */

static void incremente_et_ordonne(struct shannon_fano *sf, int position)
{
	sf->evenements[position].nb_occurrences++;
	while(position > 0 &&
			sf->evenements[position].nb_occurrences > sf->evenements[position - 1].nb_occurrences) {
		struct evenement tmp ;
		tmp = sf->evenements[position];
		
		sf->evenements[position] = sf->evenements[position - 1];
		sf->evenements[position - 1] = tmp;
		position--;
	}
}

/*
 * Cette fonction trouve la position de l'événement puis l'encode.
 * Si la position envoyée est celle de ESCAPE, elle fait un "put_bits"
 * de "evenement" pour envoyer le code du nouvel l'événement.
 * Elle termine en appelant "incremente_et_ordonne" pour l'événement envoyé.
 */
void put_entier_shannon_fano(struct bitstream *bs
			     ,struct shannon_fano *sf, int evenement)
{
	int position;
	position = trouve_position(sf, evenement);
	encode_position(bs, sf, position);
	
	if(sf->evenements[position].valeur == VALEUR_ESCAPE) {	
		sf->evenements[sf->nb_evenements].valeur = evenement ;
		sf->evenements[sf->nb_evenements].nb_occurrences = 1;
		sf->nb_evenements++;
		put_bits(bs, sizeof(int)*8, evenement);
	}
	
	incremente_et_ordonne(sf, position);
}

/*
 * Fonction inverse de "encode_position"
 */
static int decode_position(struct bitstream *bs,struct shannon_fano *sf)
{
	int borneMin = 0;
	int borneMax = sf->nb_evenements - 1;
	int separation;
	
	while(borneMin < borneMax) {
		
		separation = trouve_separation(sf, borneMin, borneMax);
		if(get_bit(bs) == 0)
			borneMax = separation;
		else
			borneMin = separation + 1;
	}
	return borneMin;
}

/*
 * Fonction inverse de "put_entier_shannon_fano"
 *
 * Attention au piège : "incremente_et_ordonne" change le tableau
 * donc l'événement trouvé peut changer de position.
 */
int get_entier_shannon_fano(struct bitstream *bs, struct shannon_fano *sf)
{
	int position;
	int retour;
	position = decode_position(bs, sf);
	
	if(sf->evenements[position].valeur == VALEUR_ESCAPE) {
		sf->evenements[sf->nb_evenements].valeur = get_bits(bs, sizeof(int)*8);
		retour = sf->evenements[sf->nb_evenements].valeur;
		sf->evenements[sf->nb_evenements].nb_occurrences = 1;
		sf->nb_evenements++;
	}
	else 
		retour = sf->evenements[position].valeur;
	
	incremente_et_ordonne(sf, position);
	return retour ;
}

/*
 * Fonctions pour les tests, NE PAS MODIFIER, NE PAS UTILISER.
 */
int sf_get_nb_evenements(struct shannon_fano *sf)
 {
   return sf->nb_evenements ;
 }
void sf_get_evenement(struct shannon_fano *sf, int i, int *valeur, int *nb_occ)
 {
   *valeur = sf->evenements[i].valeur ;
   *nb_occ = sf->evenements[i].nb_occurrences ;
 }
int sf_table_ok(const struct shannon_fano *sf)
 {
  int i, escape ;

  escape = 0 ;
  for(i=0;i<sf->nb_evenements;i++)
    {
    if ( i != 0
        && sf->evenements[i-1].nb_occurrences<sf->evenements[i].nb_occurrences)
	{
	   fprintf(stderr, "La table des événements n'est pas triée\n") ;
	   return(0) ;
	}
    if ( sf->evenements[i].valeur == VALEUR_ESCAPE )
	escape = 1 ;
    }
 if ( escape == 0 )
	{
	   fprintf(stderr, "Pas de ESCAPE dans la table !\n") ;
	   return(0) ;
	}
 return 1 ;
 }
