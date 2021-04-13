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
        self.get_L8PR_input()
        self.get_L8PR_from_kml()

    def get_L8PR_input(self):
        L8PR_tiles = requests.get(self.params["kml_l8pr_url"])
        self.L8PR_input = xmltodict.parse(L8PR_tiles.text)

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

        with open("L8_PathRow.geojson","w") as f:
            json.dump(self.L8PR_grid, f)


if __name__ == "__main__":
    print(f"Starting L8S2 mapping at: {datetime.datetime.now()}")
    s2_l8pr_intersection()
    print(f"Finished L8S2 mapping at: {datetime.datetime.now()}")
