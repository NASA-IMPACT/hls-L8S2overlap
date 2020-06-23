/* For each Landsat path/row, find all the intersecting S2 tiles, and report the 
 * intersected area as a percentage of an S2 tile area.
 *
 * Each line in the output will be: a Landsat path/row ID, tile ID of one
 * of the overlapping S2 tiles, the tile ULX & ULY, and the percentage of overlap
 * of an S2 tile.  As a Landsat path/row overlaps multiple S2 tiles,  each 
 * overlapping pair is listed on separate lines.
 * 
 * Since the nominal location of the path/row is used in computation and an actual
 * Landsat scene may shift slightly in the east-west direction relative to the nominal 
 * location, there is a chance that some scenes may be neglected in gridding to S2 tiles
 * if it barely overlaps an S2 tile, but this omission has no practical significance. 
 * The geolocation of S2 tiles are fixed.
 *
 * Implementation note:  
 *   The overlapping area percentage is derived in the UTM space. Before applying
 *   the overlapping area derivation function, test whether the UTM zone that a Landsat 
 *   path/row is in is near the UTM zone of the Sentinel-2 tile. This test is important
 *   because if they are far apart, blindly applying GCTP to reproject the corners of the
 *   a path/row into UTM can produce misleading result.
 *
 *   For HLS v1.4, a test for overlap was done in lat/lon space by only using the four corners
 *   of the two polygons. This sparse test can result in omission. Moreover, the inclusion
 *   of some rows from the ascending part of the paths for HLS v1.5 makes the relative
 *   spatial reference (UL,UR,LR,LL) complicated because in the ascending part these relations 
 *   can have opposite meaning compared to the descending part. So for v1.5 use the brute 
 *   force of computers to do the work; it is slower but reliable. 
 *
 *   Do not consider rows 140-230, which can in no way be sunlit with a not too big solar zenith.
 *  
 *  Jun 17, 2020.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hls_projection.h"
#include "pia.h"

/* For the arctic, a WRS path/row and an MGRS tile separated by 4 or 
 * more UTM zone have no chance to overlap.  */
#define ZONEDIFF 4

int main(int argc, char *argv[])
{
	char fname_l8[300];		/* Boundary of all L8 path/row */
	char fname_s2[300];		/* Boundary of all S2 tiles */
	char fname_ol[300];		/* overlap output */
	
	int npt = 4;	/* Four corners for a path/row and a tile */
	int path, row;
	double l8lon[npt], l8lat[npt];
	double l8cenlon, l8cenlat;
	int l8zone;
	char junkstr[300];

	char tileid[100];
	double s2lon[npt], s2lat[npt];
	int ulx, uly;

	char line[300];
	FILE *fl8, *fs2, *fol; 

	/* UTM of four corners for L8 and S2 */
	double l8x, l8y;		/* Landsat utm, temporary variable */
	point_t s2poly[npt], l8poly[npt];
	double s2tilesize = 109800;	/* meters */
	double s2totalarea = s2tilesize * s2tilesize;
	double interarea, pers2; 	/* intersected area as a percent of S2 total */;

	double ulx0, uly0;
	int s2zone;

	double l8vertx[npt], l8verty[npt]; 	/* For debug use only */
	char tmptileid[10];
	int tmpl8zone, tmps2zone;

	if (argc != 4) {
		fprintf(stderr, "%s L8info S2info overlap \n", argv[0]);
		exit(1);
	}

	strcpy(fname_l8, argv[1]);
	strcpy(fname_s2, argv[2]);
	strcpy(fname_ol, argv[3]);

	if ((fl8 = fopen(fname_l8, "r")) == NULL) {
		fprintf(stderr, "Cannot open for read %s\n", fname_l8);
		exit(1);
	}
	if ((fs2 = fopen(fname_s2, "r")) == NULL) {
		fprintf(stderr, "Cannot open for read %s\n", fname_s2);
		exit(1);
	}
	if ((fol = fopen(fname_ol, "w")) == NULL) {
		fprintf(stderr, "Cannot open for write %s\n", fname_ol);
		exit(1);
	}
	fprintf(fol, "PathRow S2TileID S2ULX  S2ULY PercentOfS2\n");

	fgets(line, sizeof(line), fl8);   /* Ignore header */
	/* The columns of Landsat file:
	    Path Row UL_Lat UL_Lat UR_Lat UR_Lat LR_Lat LR_Lat LL_Lat LL_Lat Center_Lat Center_Lat 

	    For the ascending part of the orbit, UL corner may not be what we think; it doesn't 
	    matter now since all we need is a polygon.
	*/
	while (fscanf(fl8, "%d", &path) != EOF) {
		fscanf(fl8, "%d", &row);	
		/* Note that L8 coordinates in input were changed to lon/lat order */
		fscanf(fl8, "%lf%lf", &l8lon[0], &l8lat[0]); /* UL */
		fscanf(fl8, "%lf%lf", &l8lon[1], &l8lat[1]); /* UR */
		fscanf(fl8, "%lf%lf", &l8lon[2], &l8lat[2]); /* LR */
		fscanf(fl8, "%lf%lf", &l8lon[3], &l8lat[3]); /* LL */
		fscanf(fl8, "%lf%lf", &l8cenlon, &l8cenlat);      /* center lon/lat*/

		/* DEBUG 
		if (path != 171 || row != 231)
			continue;
		strcpy(tmptileid, "60WVU");
		*/
			
		/* Jun 4, 2020: In arctic summer, sunlit scenes can come from ascending orbit.
		 * But the solar zenith angle can be very big. */
		if (row >= 140 && row <= 230)	/* These are considered night-time rows */
			continue;
		
		/* Roughly estimate what UTM zone the Landsat path/row is in. The estimated Landsat UTM 
		 * zone should be very close to s2zone in order to apply GCTP for UTM projection 
		 * for further analysis.  Without this careful screening, blindly apply GCTP can 
		 * give meaningless result. 
		 * Jun 16, 2020.
		 * Jim Storey once gave a WRS file which contains the assigned UTM zone for 
		 * each path/row, but the USGS publicly available file does not have it. 
		 */
		l8zone = ceil((l8cenlon + 180)/6.0);   /* Just a rough estimate. */
		/* what if l8zone happens to be 0? */ 

		//fprintf(stderr, "Considering pathrow %03d%03d:\n", path, row);
		/* For each Landsat path/row, determine which S2 tiles it overlaps.
                 * The columns of input S2 file:
			tileid epsg ulx uly ullon ullat urlon urlat lrlon lrlat lllon lllat cenlon cenlat 
		 */
		/* For each L8 pathrow, try all the S2 tiles */
		fseek(fs2, 0, SEEK_SET);
		fgets(line, sizeof(line), fs2);   /* Ignore header */
		while (fscanf(fs2, "%s", tileid) != EOF) {
			fscanf(fs2, "%s", junkstr);  /* epsg */
			fscanf(fs2, "%d%d", &ulx, &uly);	
			fscanf(fs2, "%lf%lf", &s2lon[0], &s2lat[0]);	/* UL */	
			fscanf(fs2, "%lf%lf", &s2lon[1], &s2lat[1]);	/* UR */
			fscanf(fs2, "%lf%lf", &s2lon[2], &s2lat[2]);	/* LR */
			fscanf(fs2, "%lf%lf", &s2lon[3], &s2lat[3]);	/* LL */
			fscanf(fs2, "%s %s",   junkstr, junkstr); 	/* center lon/lat. ignore*/
			
			s2zone = atoi(tileid);

			/* DEBUG. 
			if (strcmp(tileid, tmptileid) != 0)
				continue;
			fprintf(stderr, "l8zone, s2zone = %d, %d\n", l8zone, s2zone);
			*/

			/* If the two UTM zones are too far apart, do not apply GCTP; 
			 * otherwise misleading results will be obtained. 
			 * At high latitude, the zone difference can be 2 but still overlap.
			 * Jun 16, 2020 */
			tmps2zone = s2zone;
			tmpl8zone = l8zone;
			/* Straddling zone 60 */
			if (s2zone < ZONEDIFF && 60 - l8zone < ZONEDIFF) 
				tmps2zone = s2zone + 60;
			else if (l8zone < ZONEDIFF && 60 - s2zone < ZONEDIFF) 
				tmpl8zone = l8zone + 60;

			if (tmpl8zone - tmps2zone  >= ZONEDIFF || tmpl8zone - tmps2zone <= -ZONEDIFF)
				continue;

			ulx0 = ulx;
			if (tileid[2] < 'N' && uly > 0)   	/* In the south */
				uly0 = uly - pow(10,7); 	/* To accommodate GCTP */
			else
				uly0 = uly; 

			/********** Sentinel-2 corners **********/
			/* Sentinel-2 UL */
			s2poly[0].x = ulx0;  
			s2poly[0].y = uly0;

			/* Sentinel-2 UR */
			s2poly[1].x = ulx0 + s2tilesize;
			s2poly[1].y = uly0;

			/* Sentinel-2 LR */
			s2poly[2].x = ulx0 + s2tilesize;
			s2poly[2].y = uly0 - s2tilesize;

			/* Sentinel-2 LL */
			s2poly[3].x = ulx0;
			s2poly[3].y = uly0 - s2tilesize;

			/********** Landsat corners **********/
			/* Landsat UL */
			lonlat2utm(s2zone, l8lon[0], l8lat[0], &l8x, &l8y);
			l8poly[0].x = l8x;
			l8poly[0].y = l8y;
			l8vertx[0] = l8x;
			l8verty[0] = l8y;


			/* Landsat UR */
			lonlat2utm(s2zone, l8lon[1], l8lat[1], &l8x, &l8y);
			l8poly[1].x = l8x;
			l8poly[1].y = l8y;
			l8vertx[1] = l8x;
			l8verty[1] = l8y;

			/* Landsat LR */
			lonlat2utm(s2zone, l8lon[2], l8lat[2], &l8x, &l8y);
			l8poly[2].x = l8x;
			l8poly[2].y = l8y;
			l8vertx[2] = l8x;
			l8verty[2] = l8y;

			/* Landsat LL */
			lonlat2utm(s2zone, l8lon[3], l8lat[3], &l8x, &l8y);
			l8poly[3].x = l8x;
			l8poly[3].y = l8y;
			l8vertx[3] = l8x;
			l8verty[3] = l8y;

			/* DEBUG 
			fprintf(stderr, "lx = %.1lf %.1lf %.1lf %.1lf\n", l8vertx[0], l8vertx[1],l8vertx[2],l8vertx[3]);
			fprintf(stderr, "ly = %.1lf %.1lf %.1lf %.1lf\n", l8verty[0], l8verty[1],l8verty[2],l8verty[3]);
			fprintf(stderr, "sx = %.1lf %.1lf %.1lf %.1lf\n", s2poly[0].x, s2poly[1].x, s2poly[2].x, s2poly[3].x);
			fprintf(stderr, "sy = %.1lf %.1lf %.1lf %.1lf\n", s2poly[0].y, s2poly[1].y, s2poly[2].y, s2poly[3].y);
			*/

			/* Compute the intersected area in UTM, although UTM is not equal-area. The 
			 * intersection is only a qualitative measure. 
			 * Do not test whether two polygons overlap as a first step; use the
			 * function to calculate overlapping area directly.
			 */
			interarea = pia_area(s2poly, npt, l8poly, npt);
			pers2 = (interarea / s2totalarea) * 100;
			if (pers2 > 0.1) 
				fprintf(fol, "%03d%03d %s %d %d %.1lf\n", path, row, tileid, ulx, uly, pers2);
		}
	}

	fclose(fl8);
	fclose(fs2);
	fclose(fol);

	return 0;
}
