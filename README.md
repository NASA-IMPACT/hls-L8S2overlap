Apr 13, 2021
Junchang Ju & Brian Freitag

This repo maps S2 tile ids to Landsat WRS Path/Rows for HLS L30 data production.

1. Input parameters for the code are provided in the s2_l8pr_params.json file and includes the following mandatory fields:
   1) s2_tile_url: This is the github url to the list of S2 tile ids that define the global land coverage of the HLS data products
      default: "https://raw.githubusercontent.com/NASA-IMPACT/hls-land_tiles/master/HLS.land.tiles.txt"
   2) kml_s2_url: The URL to the Sentinel-2 grid KML file provided by ESA
      default: "https://sentinel.esa.int/documents/247904/1955685/S2A_OPER_GIP_TILPAR_MPC__20151209T095117_V20150622T000000_21000101T000000_B00.kml/ec05e22c-a2bc-4a13-9e84-02d5257b09a8"

   The ascending and descending path/row shapefiles are required for the intersection code. These shapefiles for the WRS-2 grid can be found here (made available by USGS): "https://www.usgs.gov/core-science-systems/nli/landsat/landsat-shapefiles-and-kml-files"
   3) ascending_shapefile: The local file path to the ascending Path/Row shapefile provided by USGS
      Note: Only ascending path/rows intersecting 65N and north are included in the intersection code. These path/rows 65N and above can have valid daytime scenes during northern hemisphere summer. The remaining path/rows are nighttime scenes that are not useful for HLS applications.
   4) descending_shapefile: The local file_path to the descending Path/Row shapefile provided by USGS
      Note: All path/rows in the descending shapefile are included in the intersection code.
   5) s2_l8_outfile: The output file for the Landsat Path/Row and Sentinel-2 tile intersection information

2. The expected format of the s2_l8_outfile should be a space-delimited file with a header row, followed by the intersection data. 

Example:

> PathRow S2TileID S2ULX S2ULY PercentOfS2\
> 001002 28XEQ 499980 9000000 9.2\
> 001002 29XMK 399960 9000000 13.5\
> 001003 27XWH 499980 8800020 3.9\
> 001003 27XWJ 499980 8900040 4.1\
> 001003 28XDN 399960 8800020 15.8

3) Included in the repo is a script to convert the KML files to GeoJson files for viewing in geojson.io. This is in the utils folder in the convertKMLtoGJ.py file. This file leverages the s2_l8pr_params.json file in the parent directory and requires an input argument variable (s2 or l8pr) to determine which geojson to convert. 
