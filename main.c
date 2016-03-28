/* inclusion des librairies */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>


#pragma pack(1)

//#####################################
//### K ==> NOMBRE DE NOYEAUX
//#####################################
#define K 7


//#####################################
//### STRUCTURE DE L ENTETE DE L IMAGE
//#####################################
struct headerImg
{
	int size;
	int width;
	int height;
	short plans;
	short depth;
	int compr;
	int sizeTotal;
	int hRes;
	int vRes;
	int nbColor;
	int nbColorImp;
};



//#####################################
//### STRUCTURE DE L ENTETE DU FICHIER
//#####################################
struct headerFile
{
	char sign[2];
	int size;
	int reserved;
	int offset;

	struct headerImg img;
};

typedef struct
{
	unsigned char b;
	unsigned char g;
	unsigned char r;

} color;

//#####################################
//### STRUCTURE CLUSTERS
//#####################################
typedef struct 
{
	unsigned int x;
	unsigned int y;

	unsigned int totalX;
	unsigned int totalY;
	unsigned int nbPixels;

	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char totalR;
	unsigned char totalG;
	unsigned char totalB;

} clusters;



//#####################################
//### INITIALISATION DES FONCTIONS
//#####################################
void kmeans(color **tab, int width, int height, struct headerFile header);
void iterate(clusters cluster[K], color **tab, int width, int height);
int findNearestCluster(clusters cluster[K], color **tab, int x, int y);
int dist(int xa, int xb, int ya, int yb);
void drawCluster(clusters cluster[K], color **tab, int width, int height);
int rgbDiff(clusters cluster, color tab);
void create(struct headerFile header, color **tabColor, int nb);


//#####################################
//### FONCTION MAIN
//#####################################
int main(int argc, char const *argv[])
{
//#####################################
//### DECLARATION DES VARIABLES
//#####################################
	FILE *fichier = NULL;
	int i = 0, j = 0;
	color **tabColor = NULL;

	fichier = fopen("lenaColor.bmp", "rb");

	struct headerFile header;

	fread(&header, sizeof(header), 1, fichier);

	tabColor = ( color ** ) malloc( header.img.width * ( sizeof(color*) ));

	for (i = 0; i < header.img.height; ++i)
	{
		tabColor[i] = ( color * ) malloc( header.img.width * ( sizeof(color) ));
	}

	for (i = header.img.height - 1; i >= 0; --i)
	{
		for (j = 0; j < header.img.width; ++j)
		{
			fread(&tabColor[i][j], sizeof( tabColor[i][j] ), 1, fichier);
		}
	}

	kmeans(tabColor, header.img.width, header.img.height, header);

	create(header, tabColor, 0);
	fclose(fichier);
	
	return 0;

}


void kmeans(color **tab, int width, int height, struct headerFile header)
{
//#####################################
//### DECLARATION DES VARIABLES
//#####################################
	int i = 0, j = 0, z = 0;
	clusters cluster[ K ];

	//----------------------------------
	// - Definition des noyeaux
	//---------------------------------- 
	srand(time(NULL));
	for (i = 0; i < K; ++i)
	{
		cluster[i].x = rand()%width;
		cluster[i].y = rand()%height;
		cluster[i].totalX = cluster[i].x;
		cluster[i].totalY = cluster[i].y;
		cluster[i].b = tab[cluster[i].x][cluster[i].y].b;
		cluster[i].g = tab[cluster[i].x][cluster[i].y].g;
		cluster[i].r = tab[cluster[i].x][cluster[i].y].r;
		cluster[i].nbPixels = 1;
	}

	//----------------------------------
	// - Itération x 20
	//----------------------------------
	for (i = 0; i < 20; ++i)
	{
		iterate(cluster, tab, width, height);
	}

	//----------------------------------
	// - On desinne les clusters
	//----------------------------------
	drawCluster(cluster, tab, width, height);

	//----------------------------------
	// - On affiche les données sur les diff cluster
	//----------------------------------
	for (i = 0; i < K; ++i)
	{
		printf("- Cluster n°%d -\n", i + 1);
		printf("Couleur: rgb(%d, %d, %d)\n", cluster[i].r, cluster[i].g, cluster[i].b);
		printf("Nombre de pixels: %d\n\n", cluster[i].nbPixels);
	}
}


void iterate(clusters cluster[K], color **tab, int width, int height)
{
//#####################################
//### DECLARATION DES VARIABLES
//#####################################
	int i = 0, j = 0, index = 0;

	//---------------------------------- 
	// - Affectation des pixels a un cluster
	//---------------------------------- 
	for (i = 0; i < width; ++i)
	{
		for (j = 0; j < height; ++j)
		{
			index = findNearestCluster(cluster, tab, i, j);
			
			cluster[index].nbPixels++;
			cluster[index].totalX += i;
			cluster[index].totalY += j;
		}
	}

	//---------------------------------- 
	// - Re-evaluation des noyeaux
	//---------------------------------- 
	for (i = 0; i < K; ++i)
	{
		cluster[i].x = cluster[i].totalX / cluster[i].nbPixels;
		cluster[i].y = cluster[i].totalY / cluster[i].nbPixels;
		cluster[i].nbPixels = 1;
		cluster[i].totalY = 0;
		cluster[i].totalX = 0;

		cluster[i].b = tab[cluster[i].x][cluster[i].y].b;
		cluster[i].g = tab[cluster[i].x][cluster[i].y].g;
		cluster[i].r = tab[cluster[i].x][cluster[i].y].r;
	}

}


int findNearestCluster(clusters cluster[K], color **tab, int x, int y)
{
//#####################################
//### DECLARATION DES VARIABLES
//#####################################
	int i = 0, j = 0;

	for (i = 1; i < K; ++i)
	{
		// if (dist(cluster[i].x, cluster[i].y, x, y) < dist(cluster[j].x, cluster[j].y, x, y))
		// {
		// 	j = i;
		// }

		if (rgbDiff(cluster[i], tab[x][y]) < rgbDiff(cluster[j], tab[x][y]) )
		{
			j = i;
		}
	}
	return j;
}


int dist(int xa, int ya, int xb, int yb)
{
//#####################################
//### FORMULE DISTANCE ENTRE 2 POINTS:
//### AB = racine((xB - xA)**2 + (yB - yA)**2)
//#####################################
	int x = xb - xa;
	int y = yb - ya;

	return sqrt(pow(x,2) + pow(y,2));
}


int rgbDiff(clusters cluster, color tab)
{
	int r = cluster.r - tab.r;
	int g = cluster.g - tab.g;
	int b = cluster.b - tab.b;

	return sqrt(pow(r,2) + pow(g,2) + pow(b,2));
}


void drawCluster(clusters cluster[K], color **tab, int width, int height)
{
	int i = 0, j = 0, index = 0;

	for (i = 0; i < width; ++i)
	{
		for (j = 0; j < height; ++j)
		{
			index = findNearestCluster(cluster, tab, i, j);
			
			tab[i][j].r = cluster[index].r;
			tab[i][j].b = cluster[index].b;
			tab[i][j].g = cluster[index].g;
			cluster[index].nbPixels++;
		}
	}
}

void create(struct headerFile header, color **tabColor, int nb)
{
	int i = 0, j = 0;
	FILE *fichierOut = NULL;

	char filename[20];
	//strcpy(filename, nb);
	sprintf(filename, "%s%d%s","Out/", nb, "-lenaOut.bmp");
	fichierOut = fopen(filename, "wb");

	fwrite(&header, sizeof(header), 1, fichierOut);

	for (i = header.img.height - 1; i >= 0; --i)
	{
		for (j = 0; j < header.img.width; ++j)
		{
			fwrite(&tabColor[i][j], sizeof( tabColor[i][j] ), 1, fichierOut);
		}
	}

	fclose(fichierOut);
}