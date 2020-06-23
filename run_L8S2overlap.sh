#!/bin/sh -u

# For each Landsat path/row in the global list, find the overlapping
# Sentinel-2 tiles.
#
# Only need to do this once during the lifetime of this project, since a global 
# list of "daytime" path/row are considered.

pathrowinfo=pathrow_info.txt 
tileinfo=S2tile_info.txt 
outfile=L8S2overlap.txt

./L8S2overlap $pathrowinfo $tileinfo $outfile
