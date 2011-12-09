/***************************************************************************
 *            obj.c
 *
 *  Mon Jun 19 21:29:40 2006
 *  Copyright  2006  Bernard GODARD
 *  
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum dataType
{ T_V, T_VN, T_VT, T_F, T_MTLLIB, T_USEMTL,
	T_NEWMTL, T_NI, T_NS, T_ILLUM, T_D, T_TR,
	T_KA, T_KD, T_KS, T_MAP_KA, T_MAP_KD, T_MAP_KS
};


typedef struct
{
	char val[255];
} nameIndex;

int lookupIndex (nameIndex * indexTab, int *nbElt, int nbMaxElt, char *s,
		 int createNew, int *ind);

int readLineSimple (FILE * fp, char *line);

int readLinePlus (FILE * fp, char *line, enum dataType *t);

int readFace (char *s, objFace * face, int matIndex);

int getMtlInfo (char *fName, int *nbM);

int getObjInfo (char *fName, char *mtlName,
		int *nbV, int *nbVN, int *nbVT, int *nbF);

int initObj (char *objFullFilename, objObj * obj, FILE ** obj_fp,
	     FILE ** mtl_fp, nameIndex ** mtlIndex, nameIndex ** texIndex);

int readMtl (FILE * fp, objObj obj, nameIndex * mtlIndex,
	     nameIndex * texIndex);

int readObj (FILE * fp, objObj obj, nameIndex * mtlIndex);



int cleanBeforeExit (int errorID, objObj * obj, FILE * obj_fp, FILE * mtl_fp,
		     nameIndex * mtlIndex, nameIndex * texIndex);

int
lookupIndex (nameIndex * indexTab, int *nbElt, int nbMaxElt, char *s,
	     int createNew, int *ind)
{

	int i;

	if ((*nbElt) > nbMaxElt)
	{
		return 1;
	}

	for (i = 0; i < (*nbElt); i++)
	{
		if (!strcmp (indexTab[i].val, s))
		{
			*ind = i;
			return 0;
		}
	}
	if (createNew)
	{
		if ((*nbElt) + 1 > nbMaxElt)
		{
			fprintf (stderr,
				 "No more space in temporary index.\n");
			return 1;
		}
		*ind = (*nbElt);
		strcpy (indexTab[*nbElt].val, s);
		(*nbElt)++;
	}
	else
	{
		return 1;
	}

	return 0;

}


int
readLineSimple (FILE * fp, char *line)
{
	if (feof (fp))
		return 1;
	fgets (line, 255, fp);
	return 0;
}

int
readLinePlus (FILE * fp, char *line, enum dataType *t)
{
	int found = 0;
	char tmp1[255], tmp2[255];

	do
	{
		if (readLineSimple (fp, line))
			return 1;
		sscanf (line, "%s %[^\n\r]", tmp1, tmp2);
		strcpy (line, tmp2);
		if (!strcmp (tmp1, "v"))
		{
			found = 1;
			*t = T_V;
		}
		if (!strcmp (tmp1, "vn"))
		{
			found = 1;
			*t = T_VN;
		}
		if (!strcmp (tmp1, "vt"))
		{
			found = 1;
			*t = T_VT;
		}
		if (!strcmp (tmp1, "f"))
		{
			found = 1;
			*t = T_F;
		}
		if (!strcmp (tmp1, "mtllib"))
		{
			found = 1;
			*t = T_MTLLIB;
		}
		if (!strcmp (tmp1, "usemtl"))
		{
			found = 1;
			*t = T_USEMTL;
		}
		if (!strcmp (tmp1, "newmtl"))
		{
			found = 1;
			*t = T_NEWMTL;
		}
		if (!strcmp (tmp1, "illum"))
		{
			found = 1;
			*t = T_ILLUM;
		}
		if (!strcmp (tmp1, "Ni"))
		{
			found = 1;
			*t = T_NI;
		}
		if (!strcmp (tmp1, "Ns"))
		{
			found = 1;
			*t = T_NS;
		}
		if (!strcmp (tmp1, "Tr"))
		{
			found = 1;
			*t = T_TR;
		}
		if (!strcmp (tmp1, "d"))
		{
			found = 1;
			*t = T_D;
		}
		if (!strcmp (tmp1, "Ka"))
		{
			found = 1;
			*t = T_KA;
		}
		if (!strcmp (tmp1, "Kd"))
		{
			found = 1;
			*t = T_KD;
		}
		if (!strcmp (tmp1, "Ks"))
		{
			found = 1;
			*t = T_KS;
		}
		if (!strcmp (tmp1, "map_Ka"))
		{
			found = 1;
			*t = T_MAP_KA;
		}
		if (!strcmp (tmp1, "map_Kd"))
		{
			found = 1;
			*t = T_MAP_KD;
		}
		if (!strcmp (tmp1, "map_Ks"))
		{
			found = 1;
			*t = T_MAP_KS;
		}
	}
	while (!found);

	return 0;
}

int
getMtlInfo (char *fName, int *nbM)
{
	FILE *fp;
	enum dataType t;
	char line[255];
	char errString[255];

	fp = fopen (fName, "r");
	if (fp == NULL)
	{
		sprintf (errString, "Can't open file (%s)", fName);
		perror (errString);
		return 1;
	}

	*nbM = 0;
	while (!readLinePlus (fp, line, &t))
	{
		if (t == T_NEWMTL)
			(*nbM)++;
	}

	fclose (fp);

	return 0;
}

int
getObjInfo (char *fName, char *mtlName,
	    int *nbV, int *nbVN, int *nbVT, int *nbF)
{
	FILE *fp;
	char errString[255];
	char line[255];
	enum dataType t;

	*nbV = 0;
	*nbVN = 0;
	*nbVT = 0;
	*nbF = 0;

	fp = fopen (fName, "r");
	if (fp == NULL)
	{
		sprintf (errString, "Can't open file (%s)", fName);
		perror (errString);
		return 1;
	}

	while (!readLinePlus (fp, line, &t))
	{
		switch (t)
		{
		case T_MTLLIB:
			strcpy (mtlName, line);
			break;
		case T_V:
			(*nbV)++;
			break;
		case T_VN:
			(*nbVN)++;
			break;
		case T_VT:
			(*nbVT)++;
			break;
		case T_F:
			(*nbF)++;
			break;
		default:
			continue;
		}

	}

	fclose (fp);

	return 0;
}


int
readFace (char *s, objFace * face, int matIndex)
{
	char s1[255], s2[255], sv[255];
	int i1, i2, i3;
	int i;
	int hasNormal = 0;
	int hasTexcoord = 0;
	int nbItem = 0;

	face->materialIndex = matIndex;

	if (sscanf (s, "%d/%d/%d %[^\n\r]", &i1, &i2, &i3, s1) == 4)
	{
		hasTexcoord = 1;
		hasNormal = 1;
	}
	else
	{
		if (sscanf (s, "%d//%d %[^\n\r]", &i1, &i3, s1) == 3)
		{
			hasNormal = 1;
		}
		else
		{
			if (sscanf (s, "%d/%d %[^\n\r]", &i1, &i2, s1) == 3)
			{
				hasTexcoord = 1;
			}
		}
	}

	strcpy (s1, s);

	while (s1[0] != '\0')
	{
		s2[0] = '\0';
		sscanf (s1, "%s %[^\n\r]", sv, s2);
		strcpy (s1, s2);
		nbItem++;
	}
	strcpy (s1, s);

	face->nbVertex = nbItem;

	if (nbItem > 0)
	{
		face->vertexIndexList =
			(int *) malloc (nbItem * sizeof (int));
		if (face->vertexIndexList == NULL)
		{
			perror ("Unable to allocate memory for face vertices index array");
			return 1;
		}
		if (hasTexcoord)
		{
			face->texcoordIndexList =
				(int *) malloc (nbItem * sizeof (int));
			if (face->texcoordIndexList == NULL)
			{
				perror ("Unable to allocate memory for face texcoords index array");
				return 1;
			}
		}
		if (hasNormal)
		{
			face->normalIndexList =
				(int *) malloc (nbItem * sizeof (int));
			if (face->normalIndexList == NULL)
			{
				perror ("Unable to allocate memory for face normals index array");
				return 1;
			}
		}

	}


	for (i = 0; i < nbItem; i++)
	{
		sscanf (s1, "%s %[^\n\r]", sv, s2);
		strcpy (s1, s2);

		if ((hasNormal) && (hasTexcoord))
		{
			sscanf (sv, "%d/%d/%d", &i1, &i2, &i3);
		}
		else
		{
			if (hasNormal)
			{
				sscanf (sv, "%d//%d", &i1, &i3);
			}
			else
			{
				if (hasTexcoord)
				{
					sscanf (sv, "%d/%d", &i1, &i2);
				}
				else
				{
					sscanf (sv, "%d", &i1);
				}
			}
		}

		face->vertexIndexList[i] = i1 - 1;
		if (hasTexcoord)
			face->texcoordIndexList[i] = i2 - 1;
		if (hasNormal)
			face->normalIndexList[i] = i3 - 1;


	}

	return 0;

}


int
initObj (char *objFullFilename, objObj * obj, FILE ** obj_fp, FILE ** mtl_fp,
	 nameIndex ** mtlIndex, nameIndex ** texIndex)
{


	char errString[255];
	/* Path to obj and mtl data */
	char pathName[255];
	char *p;
	/* material lib filename */
	char mtlFilename[255];
	/* with path */
	char mtlFullFilename[255];
	/* number of vertices, normals, texture coordinates,  
	 * faces and materials */
	int nbV, nbVN, nbVT, nbF, nbM;
	int i;

	mtlFilename[0] = '\0';

	//printf ("Obj file : %s\n", objFullFilename);

	strcpy (pathName, objFullFilename);
	p = strrchr (pathName, '/');
	if (p == NULL)
		pathName[0] = '\0';
	else
		p[1] = '\0';

	if (getObjInfo
	    (objFullFilename, mtlFilename, &nbV, &nbVN, &nbVT, &nbF))
		return 1;

	//printf ("Obj has %d vertices, %d normals, %d texcoords, %d faces\n",
  //		nbV, nbVN, nbVT, nbF);
	if (mtlFilename[0] == 0)
	{
		fprintf (stderr, "Error: No material library\n");
		return 1;
	}

	strcpy (mtlFullFilename, pathName);
	strcat (mtlFullFilename, mtlFilename);

	//printf ("Material library file is %s\n", mtlFullFilename);

	if (getMtlInfo (mtlFullFilename, &nbM))
		return 1;

	//printf ("Material library has %d materials\n", nbM);

	*obj_fp = fopen (objFullFilename, "r");
	if (*obj_fp == NULL)
	{
		sprintf (errString, "Can't open file (%s)", objFullFilename);
		perror (errString);
		return 1;
	}

	*mtl_fp = fopen (mtlFullFilename, "r");
	if (*mtl_fp == NULL)
	{
		sprintf (errString, "Can't open file (%s)", mtlFullFilename);
		perror (errString);
		return 1;
	}

	obj->nbVertex = nbV;
	obj->nbNormal = nbVN;
	obj->nbTexcoord = nbVT;
	obj->nbFace = nbF;
	obj->nbMaterial = nbM;

	if (nbV > 0)
	{
		obj->vertexList =
			(objVertex *) malloc (nbV * sizeof (objVertex));
		if (obj->vertexList == NULL)
		{
			perror ("Unable to allocate memory for vertices array");
			return 1;
		}
	}
	if (nbVN > 0)
	{
		obj->normalList =
			(objNormal *) malloc (nbVN * sizeof (objNormal));
		if (obj->normalList == NULL)
		{
			perror ("Unable to allocate memory for normals array");
			return 1;
		}
	}
	if (nbVT > 0)
	{
		obj->texcoordList =
			(objTexcoord *) malloc (nbVT * sizeof (objTexcoord));
		if (obj->texcoordList == NULL)
		{
			perror ("Unable to allocate memory for texcoords array");
			return 1;
		}
	}
	if (nbF > 0)
	{
		obj->faceList = (objFace *) malloc (nbF * sizeof (objFace));
		if (obj->faceList == NULL)
		{
			perror ("Unable to allocate memory for faces array");
			return 1;
		}
		for (i = 0; i < nbF; i++)
		{
			obj->faceList[i].vertexIndexList = NULL;
			obj->faceList[i].normalIndexList = NULL;
			obj->faceList[i].texcoordIndexList = NULL;
		}
	}
	if (nbM > 0)
	{
		obj->materialList =
			(objMaterial *) malloc (nbM * sizeof (objMaterial));
		if (obj->materialList == NULL)
		{
			perror ("Unable to allocate memory for materials array");
			return 1;
		}
		/* A temporary array to store material names */
		*mtlIndex = (nameIndex *) malloc (nbM * sizeof (nameIndex));
		if (*mtlIndex == NULL)
		{
			perror ("Unable to allocate memory for material temporary index");
			return 1;
		}
		/* A temporary array to store texture filenames.
		 * Max 3 textures per material (ambient,diffuse,specular)
		 * assuming all nbM materials have got 3 textures each 
		 * we can have at most 3*nbM textures */
		*texIndex =
			(nameIndex *) malloc (3 * nbM * sizeof (nameIndex));
		if (*texIndex == NULL)
		{
			perror ("Unable to allocate memory for texture temporary index");
			return 1;
		}

	}

	return 0;
}

int
readMtl (FILE * fp, objObj obj, nameIndex * mtlIndex, nameIndex * texIndex)
{
	char line[255];
	enum dataType t;
	float v[3];
	int i;
	int texCount;
	int maxTexCount;
	int val;

	int index = -1;

	texCount = 0;
	maxTexCount = 3 * obj.nbMaterial;

	while (!readLinePlus (fp, line, &t))
	{
		if (t == T_NEWMTL)
		{
			index = index + 1;
			strcpy (mtlIndex[index].val, line);
			if (index >= obj.nbMaterial)
			{
				fprintf (stderr,
					 "Problem with materials count\n");
				return 1;
			}
			obj.materialList[index].ambientTexId = -1;
			obj.materialList[index].diffuseTexId = -1;
			obj.materialList[index].specularTexId = -1;
			obj.materialList[index].alpha = 1.0;
			obj.materialList[index].shininess = 0.0;
			obj.materialList[index].refrac = 1.0;
			obj.materialList[index].illumModel = 1;
			for (i = 0; i < 3; i++)
			{
				obj.materialList[index].ambient[i] = 0.2;
				obj.materialList[index].diffuse[i] = 0.8;
				obj.materialList[index].specular[i] = 1.0;
			}
		}
		if (index >= 0)
		{
			switch (t)
			{
			case T_MAP_KA:
				if (lookupIndex
				    (texIndex, &texCount, maxTexCount, line,
				     1,
				     &(obj.materialList[index].ambientTexId)))
				{
					fprintf (stderr,
						 "Problem with textures count\n");
					return 1;
				}
				break;
			case T_MAP_KD:
				if (lookupIndex
				    (texIndex, &texCount, maxTexCount, line,
				     1,
				     &(obj.materialList[index].diffuseTexId)))
				{
					fprintf (stderr,
						 "Problem with textures count\n");
					return 1;
				}
				break;
			case T_MAP_KS:
				if (lookupIndex
				    (texIndex, &texCount, maxTexCount, line,
				     1,
				     &(obj.materialList[index].
				       specularTexId)))
				{
					fprintf (stderr,
						 "Problem with textures count\n");
					return 1;
				}
				break;
			case T_KA:
				sscanf (line, "%f %f %f", &v[0], &v[1],
					&v[2]);
				for (i = 0; i < 3; i++)
					obj.materialList[index].ambient[i] =
						v[i];
				break;
			case T_KD:
				sscanf (line, "%f %f %f", &v[0], &v[1],
					&v[2]);
				for (i = 0; i < 3; i++)
					obj.materialList[index].diffuse[i] =
						v[i];
				break;
			case T_KS:
				sscanf (line, "%f %f %f", &v[0], &v[1],
					&v[2]);
				for (i = 0; i < 3; i++)
					obj.materialList[index].specular[i] =
						v[i];
				break;
			case T_ILLUM:
				sscanf (line, "%d", &val);
				obj.materialList[index].illumModel = val;
			case T_TR:
				sscanf (line, "%f", &v[0]);
				obj.materialList[index].alpha = v[0];
			case T_D:
				sscanf (line, "%f", &v[0]);
				obj.materialList[index].alpha = v[0];
			case T_NS:
				sscanf (line, "%f", &v[0]);
				obj.materialList[index].shininess = v[0];
			case T_NI:
				sscanf (line, "%f", &v[0]);
				obj.materialList[index].refrac = v[0];
			default:
				continue;
			}
		}


	}


	return 0;
}

int
readObj (FILE * fp, objObj obj, nameIndex * mtlIndex)
{
	char line[255];
	enum dataType t;

	float v[3];
	int i;

	int vertexCount = 0;
	int texcoordCount = 0;
	int normalCount = 0;
	int faceCount = 0;
	int currentMaterialIndex = -1;

	while (!readLinePlus (fp, line, &t))
	{
		switch (t)
		{
		case (T_V):
			if (vertexCount >= obj.nbVertex)
			{
				fprintf (stderr,
					 "Problem with vertices Count\n");
				return 1;
			}
			sscanf (line, "%f %f %f", &v[0], &v[1], &v[2]);
			for (i = 0; i < 3; i++)
				obj.vertexList[vertexCount].coords[i] = v[i];
			vertexCount++;
			break;
		case (T_VN):
			if (normalCount >= obj.nbNormal)
			{
				fprintf (stderr,
					 "Problem with normals Count\n");
				return 1;
			}
			sscanf (line, "%f %f %f", &v[0], &v[1], &v[2]);
			for (i = 0; i < 3; i++)
				obj.normalList[normalCount].coords[i] = v[i];
			normalCount++;
			break;
		case (T_VT):
			if (texcoordCount >= obj.nbTexcoord)
			{
				fprintf (stderr,
					 "Problem with texcoords Count\n");
				return 1;
			}
			sscanf (line, "%f %f %f", &v[0], &v[1], &v[2]);
			for (i = 0; i < 3; i++)
				obj.texcoordList[texcoordCount].coords[i] =
					v[i];
			texcoordCount++;
			break;
		case (T_USEMTL):
			if (lookupIndex
			    (mtlIndex, &(obj.nbMaterial), obj.nbMaterial,
			     line, 0, &currentMaterialIndex))
			{
				fprintf (stderr,
					 "Problem with material count\n");
				return 1;
			}
			break;
		case (T_F):
			if (faceCount >= obj.nbFace)
			{
				fprintf (stderr,
					 "Problem with faces Count\n");
				return 1;
			}

			if (readFace
			    (line, &(obj.faceList[faceCount]),
			     currentMaterialIndex))
				return 1;

			faceCount++;
			break;
		default:
			continue;
		}

	}
	return 0;
}

int
freeObj (objObj * obj)
{
	objFace f;
	int i;

	if (obj == NULL)
		return 1;

	if (obj->faceList != NULL)
	{
		for (i = 0; i < obj->nbFace; i++)
		{
			f = obj->faceList[i];
			if (f.vertexIndexList != NULL)
				free (f.vertexIndexList);
			if (f.normalIndexList != NULL)
				free (f.normalIndexList);
			if (f.texcoordIndexList != NULL)
				free (f.texcoordIndexList);

		}
		free (obj->faceList);
	}

	if (obj->materialList != NULL)
		free (obj->materialList);
	if (obj->vertexList != NULL)
		free (obj->vertexList);
	if (obj->normalList != NULL)
		free (obj->normalList);
	if (obj->texcoordList != NULL)
		free (obj->texcoordList);


	return 0;
}


int
cleanBeforeExit (int errorID, objObj * obj, FILE * obj_fp,
		 FILE * mtl_fp, nameIndex * mtlIndex, nameIndex * texIndex)
{
	if (errorID != 0)
	{
		/* clean obj data */
		freeObj (obj);
	}

	if (mtl_fp != NULL)
		fclose (mtl_fp);
	if (obj_fp != NULL)
		fclose (obj_fp);

	if (mtlIndex != NULL)
		free (mtlIndex);
	if (texIndex != NULL)
		free (texIndex);

	return errorID;
}

int
openObj (char *filename, objObj * obj)
{
	nameIndex *mtlIndex;
	nameIndex *texIndex;

	FILE *obj_fp = NULL;
	FILE *mtl_fp = NULL;

	mtlIndex = NULL;
	texIndex = NULL;

	obj->nbVertex = 0;
	obj->nbNormal = 0;
	obj->nbTexcoord = 0;
	obj->nbFace = 0;
	obj->nbMaterial = 0;

	obj->vertexList = NULL;
	obj->normalList = NULL;
	obj->texcoordList = NULL;
	obj->faceList = NULL;
	obj->materialList = NULL;



	if (initObj (filename, obj, &obj_fp, &mtl_fp, &mtlIndex, &texIndex))
		return cleanBeforeExit (1, obj, obj_fp, mtl_fp, mtlIndex,
					texIndex);

	if (readMtl (mtl_fp, *obj, mtlIndex, texIndex))
		return cleanBeforeExit (1, obj, obj_fp, mtl_fp, mtlIndex,
					texIndex);

	if (readObj (obj_fp, *obj, mtlIndex))
		return cleanBeforeExit (1, obj, obj_fp, mtl_fp, mtlIndex,
					texIndex);


	return cleanBeforeExit (0, obj, obj_fp, mtl_fp, mtlIndex, texIndex);
}
