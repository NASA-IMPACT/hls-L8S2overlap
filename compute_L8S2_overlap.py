import datetime
import json
import pandas as pd
import requests
import xmltodict

from shapely.geometry import Polygon

class s2_l8pr_intersection:


    def __init__(self):
        with open("s2_l8pr_params.json", "r") as f:
            params = json.loads(f.read())
        self.params = params
        self.L8PR_grid = {"type": "FeatureCollection", "features": []}
        self.S2_HLS_grid = {"type": "FeatureCollection", "features": []}
        self.get_S2_input()
        self.get_S2grid_from_kml()
        self.get_L8PR_input()
        self.get_L8PR_from_kml()
        self.get_intersection()

    def get_S2_input(self):
        self.S2_HLS_grid = {"type": "FeatureCollection", "features": []}
        self.S2_HLS_tiles = requests.get(self.params["s2_tile_url"]).text.split("\n")
        S2_all_tiles = requests.get(self.params["kml_s2_url"])
        self.S2_input = xmltodict.parse(S2_all_tiles.text)

    def get_L8PR_input(self):
        L8PR_tiles = requests.get(self.params["kml_l8pr_url"])
        self.L8PR_input = xmltodict.parse(L8PR_tiles.text)

    def get_S2grid_from_kml(self):
        objects = self.S2_input["kml"]["Document"]["Folder"][0]["Placemark"]
        for obj in objects:
            if obj["name"] in self.S2_HLS_tiles:
                description = pd.read_html(obj["description"])[0]
                coords = description.iloc[3,1].strip("MULTIPOLYGON(((").strip(")))").split(",")
                ul = coords[0].split(" ")
                feature = {"type": "Feature",
                           "properties": {"type": "S2"}, "geometry": {}
                          }
                feature["properties"]["identifier"] = obj["name"]
                feature["properties"]["ulx"] = ul[0]
                feature["properties"]["uly"] = ul[1]
                feature["geometry"]["type"] = "MultiPolygon"
                feature["geometry"]["coordinates"] = []
                polys = obj["MultiGeometry"]["Polygon"]
                polys = [polys] if not isinstance(polys, list) else polys
                for poly in polys:
                    boundary = poly["outerBoundaryIs"]["LinearRing"]["coordinates"]
                    boundary = boundary.split(" ")
                    coordinates = []
                    for coord in boundary:
                        ll = [float(x) for x in coord.split(",")]
                        coordinates.append([ll[0], ll[1], ll[2]])
                    feature["geometry"]["coordinates"].append([coordinates])
                self.S2_HLS_grid["features"].append(feature)
        #sort dictionary
        S2_sorted = sorted(self.S2_HLS_grid["features"], key=lambda x:x["properties"]["identifier"])
        self.S2_HLS_grid["features"] = S2_sorted

    def get_L8PR_from_kml(self):
        objects = self.L8PR_input["kml"]["Document"]["Placemark"]
        for obj in objects:
            path = f'{int(obj["name"].split("_")[0]):03}'
            row = f'{int(obj["name"].split("_")[1]):03}'
            feature = {"type": "Feature",
                       "properties": {"type": "L8PR"}, "geometry": {}
                      }
            feature["properties"]["identifier"] = "".join([path,row])
            feature["geometry"]["type"] = "Polygon"
            feature["geometry"]["coordinates"] = []
            poly = obj["Polygon"]
            boundary = poly["outerBoundaryIs"]["LinearRing"]["coordinates"]
            boundary = boundary.split(" ")
            coordinates = []
            for coord in boundary:
                ll = [float(x) for x in coord.split(",")]
                coordinates.append([ll[0], ll[1], ll[2]])
            feature["geometry"]["coordinates"].append(coordinates)
            self.L8PR_grid["features"].append(feature)

        #sort dictionary
        L8PR_sorted = sorted(self.L8PR_grid["features"], key=lambda x:x["properties"]["identifier"])
        self.L8PR_grid["features"] = L8PR_sorted

    def get_intersection(self):
        with open(self.params["s2_l8_outfile"], "w") as f:
            f.write("PathRow S2TileID S2ULX S2ULY PercentOfS2\n")
            for l8pr_tile in self.L8PR_grid["features"]:
                l8pr = Polygon(l8pr_tile["geometry"]["coordinates"][0])
                for s2_tile in self.S2_HLS_grid["features"]:
                    for s2_poly in s2_tile["geometry"]["coordinates"]:
                        s2 = Polygon(s2_poly[0])
                        x = l8pr.intersection(s2)
                        coverage = (x.area/s2.area)*100.
                        if coverage > 0.1:
                            s2_prop = s2_tile["properties"]
                            print(f"{l8pr_tile['properties']['identifier']} {s2_prop['identifier']} {s2_prop['ulx']} {s2_prop['uly']} {coverage:.1f}")
                            f.write(f"{l8pr_tile['properties']['identifier']} {s2_prop['identifier']} {s2_prop['ulx']} {s2_prop['uly']} {coverage:.1f}\n")


if __name__ == "__main__":
    print(f"Starting L8S2 mapping at: {datetime.datetime.now()}")
    s2_l8pr_intersection()
    print(f"Finished L8S2 mapping at: {datetime.datetime.now()}")
