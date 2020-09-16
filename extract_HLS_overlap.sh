# Sep 11, 2020: Extract the overlap LUT only for the 18501 HLS land tiles 
# saved in HLS.land.tiles.txt.
#
# Tile 24XWU is not in the global overlap table, most likely because
# Landsat doesn't go that high.

awk 'NR == 1' L8S2overlap.txt > tmp.hdr.$$
awk 'NR > 1' L8S2overlap.txt | 
  sort -k2,2 | 
  join -1 2 /dev/stdin HLS.land.tiles.txt |
  awk '{tmp = $1; $1 = $2; $2 = tmp; print $0 }' |
  sort -k1,1 -k2,2 > tmp.land.overlap.$$

cat tmp.hdr.$$ tmp.land.overlap.$$  > HLS.L8S2overlap.txt 

rm tmp.*.$$


