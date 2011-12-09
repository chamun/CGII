/***************************************************************************
 *            obj.h
 *
 *  Mon Jun 19 21:29:49 2006
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


#ifndef _OBJ_H
#define _OBJ_H

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		float coords[3];
	} objVertex;

	typedef struct
	{
		float coords[3];
	} objNormal;

	typedef struct
	{
		float coords[3];
	} objTexcoord;

	typedef struct
	{
		float ambient[3];
		float diffuse[3];
		float specular[3];
		int illumModel;
		float shininess;
		float alpha;
		float refrac;
		int ambientTexId;
		int diffuseTexId;
		int specularTexId;
	} objMaterial;


	typedef struct
	{
		int nbVertex;
		int *vertexIndexList;
		int *normalIndexList;
		int *texcoordIndexList;
		int materialIndex;
	} objFace;

	typedef struct
	{
		int nbVertex;
		int nbNormal;
		int nbTexcoord;
		int nbFace;
		int nbMaterial;
		objVertex *vertexList;
		objNormal *normalList;
		objTexcoord *texcoordList;
		objFace *faceList;
		objMaterial *materialList;
	} objObj;

	int openObj (char *filename, objObj * obj);
	int freeObj (objObj * obj);

#ifdef __cplusplus
}
#endif

#endif				/* _OBJ_H */
