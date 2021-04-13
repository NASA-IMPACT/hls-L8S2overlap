import datetime
import json
import pandas as pd
import requests
import sys
import xmltodict

from shapely.geometry import Polygon

class KMLtoGeoJson:


    def __init__(self):
        with open("../s2_l8pr_params.json", "r") as f:
            params = json.loads(f.read())
        self.params = params
        self.grid = {"type": "FeatureCollection", "features": []}
        if len(sys.argv) == 1 or (sys.argv[1] != "s2" and sys.argv[1] != "l8pr"):
           print("This function requires an argument variable `s2` or `l8pr`. Exiting now")
           exit()
        self.source = sys.argv[1]
        self.get_input()
        self.write_output()

    def get_input(self):
        tiles = requests.get(self.params[f"kml_{self.source}_url"])
        self.input = xmltodict.parse(tiles.text)
        execDict = {"l8pr": self.get_L8PR_from_kml,
                    "s2": self.get_S2grid_from_kml}
        execDict[self.source]()

    def get_L8PR_from_kml(self):
        objects = self.input["kml"]["Document"]["Placemark"]
        for obj in objects:
            path = f'{int(obj["name"].split("_")[0]):03}'
            row = f'{int(obj["name"].split("_")[1]):03}'
            feature = {"type": "Feature",
                                   "properties": {"type": "L8PR"}, "geometry": {}
                                   }
            feature["properties"]["identifier"] = "".join([path,row])
            feature["geometry"]["type"] = "MultiPolygon"
            feature["geometry"]["coordinates"] = []
            poly = obj["Polygon"]
            boundary = poly["outerBoundaryIs"]["LinearRing"]["coordinates"]
            boundary = boundary.split(" ")
            coordinates = []
            for coord in boundary:
                ll = [float(x) for x in coord.split(",")]
                coordinates.append([ll[0], ll[1], ll[2]])
            feature["geometry"]["coordinates"].append(coordinates)
            self.grid["features"].append(feature)

    def get_S2grid_from_kml(self):
        objects = self.input["kml"]["Document"]["Folder"][0]["Placemark"]
        S2_HLS_tiles = requests.get(self.params["s2_tile_url"]).text.split("\n")
        for obj in objects:
            if obj["name"] in S2_HLS_tiles:
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
                self.grid["features"].append(feature)

    def write_output(self):
        sorted_grid = sorted(self.grid["features"], key=lambda x:x["properties"]["identifier"])
        self.grid["features"] = sorted_grid
        with open(f"{self.source}.geojson","w") as f:
            json.dump(self.grid, f)

if __name__ == "__main__":
    print(f"Starting GeoJson creation at: {datetime.datetime.now()}")
    KMLtoGeoJson()
    print(f"Finished GeoJson creation at: {datetime.datetime.now()}")
