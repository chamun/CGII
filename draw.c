/***************************************************************************
 *            draw.c
 *
 *  Sat Jul  1 21:05:41 2006
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

#include "draw.h"

int
drawObj (objObj obj)
{
	int i, j;
	int ind;


	for (i = 0; i < obj.nbFace; i++)
	{
		ind = obj.faceList[i].materialIndex;
		if (ind >= 0)
		{
			glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT,
				      obj.materialList[ind].ambient);
			glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE,
				      obj.materialList[ind].diffuse);
			glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR,
				      obj.materialList[ind].specular);
			glColor3fv (obj.materialList[ind].diffuse);

		}
		glBegin (GL_POLYGON);
		for (j = 0; j < obj.faceList[i].nbVertex; j++)
		{
			if (obj.faceList[i].normalIndexList != NULL)
			{
				ind = obj.faceList[i].normalIndexList[j];
				glNormal3fv (obj.normalList[ind].coords);
			}
			ind = obj.faceList[i].vertexIndexList[j];
			glVertex3fv (obj.vertexList[ind].coords);


		}
		glEnd ();
	}
	return 0;
}


int
drawCube ()
{

	GLfloat Mspec[] = { 0.5, 0.5, 0.5 };
	GLfloat Mshiny = 50;

	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, Mspec);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, Mshiny);

	glBegin (GL_POLYGON);
	glNormal3d (0, 0, 1);
	glColor3d (1, 0, 0);
	glVertex3f (-1, 1, 1);
	glVertex3f (-1, -1, 1);
	glVertex3f (1, -1, 1);
	glVertex3f (1, 1, 1);
	glEnd ();
	glBegin (GL_POLYGON);
	glNormal3d (1, 0, 0);
	glColor3d (0, 1, 0);
	glVertex3f (1, 1, 1);
	glVertex3f (1, -1, 1);
	glVertex3f (1, -1, -1);
	glVertex3f (1, 1, -1);
	glEnd ();

	glBegin (GL_POLYGON);
	glNormal3d (0, 0, -1);
	glColor3d (0, 0, 1);
	glVertex3f (1, 1, -1);
	glVertex3f (1, -1, -1);
	glVertex3f (-1, -1, -1);
	glVertex3f (-1, 1, -1);
	glEnd ();

	glBegin (GL_POLYGON);
	glNormal3d (-1, 0, 0);
	glColor3d (1, 1, 0);
	glVertex3f (-1, 1, -1);
	glVertex3f (-1, -1, -1);
	glVertex3f (-1, -1, 1);
	glVertex3f (-1, 1, 1);
	glEnd ();

	glBegin (GL_POLYGON);
	glNormal3d (0, 1, 0);
	glColor3d (1, 0, 1);
	glVertex3f (-1, 1, -1);
	glVertex3f (-1, 1, 1);
	glVertex3f (1, 1, 1);
	glVertex3f (1, 1, -1);
	glEnd ();

	glBegin (GL_POLYGON);
	glNormal3d (0, -1, 0);
	glColor3d (0, 1, 1);
	glVertex3f (-1, -1, 1);
	glVertex3f (-1, -1, -1);
	glVertex3f (1, -1, -1);
	glVertex3f (1, -1, 1);
	glEnd ();

	return 0;
}
