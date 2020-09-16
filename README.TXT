Junchang Ju 

Sep 11, 2020: Update to output for the HLS land tiles only.

1. 
The program L8S2overlap finds all the Landsat path/rows overlapping each MGRS
tile.  The Landsat WRS rows 1-140 and 230-248 are considered. Rows after 122 
are nominally considered "nightime" rows, but over the long summer days some 
sunlit scenes with very large solar zenith angle may be available at these 
locations. Conversely, for both ends of nominal daytime list of rows 1-122, 
there is a chance that no sunlight at all during winter at the time of satellit 
overpass and thus no data.

The derived overlapping relationship is saved in L8S2overlap.txt. The 5 output 
columns are:
  1) WRS path/row  
  2) MGRS tile ID  
  3) Upperleft UTM X coordinate of the MGRS tile
  4) Upperleft UTM Y coordinate of the MGRS tile 
  5) Overlapping area as percentage of MGRS tile area

All the reported overlap is for percentage > 0.1%.

For example:
PathRow S2TileID S2ULX  S2ULY PercentOfS2
001001 29XNK 499980 9000000 1.7
001001 29XNL 499980 9100020 5.0
001001 30XVQ 399960 9000000 4.1 

2. 
In particular, the subset of the global overlap lookup table for the 18501 HLS land tiles 
is extracted and saved in HLS.L8S2overlap.txt.  

The extract script is extract_HLS_overlap.sh 

Note: Tile 24XWU is not in the global overlap table, most likely because
Landsat doesn't go that high.


Notes:

1. The MGRS UTM coordinates used by Sentinel-2 is available in 
	https://sentinel.esa.int/documents/247904/1955685/S2A_OPER_GIP_TILPAR_MPC__20151209T095117_V20150622T000000_21000101T000000_B00.kml

   Relevant information is extracted and saved in file S2tile_info.txt 

2. The Landsat WRS path/row coordinates in lat/lon is available in
	https://www.usgs.gov/media/files/landsat-wrs-2-corner-points

   	Relevant information is extracted and saved in file pathrow_info.txt 

3. The General Cartographic Transformation Package (GCTP) is availble from several
   US government agencies' website, even from GitHub.   

4. The polygon-intersection-aera code is available in:
	https://cadxfem.org/svn/cadxfem/moshade/pia.h
	https://cadxfem.org/svn/cadxfem/moshade/pia.c


