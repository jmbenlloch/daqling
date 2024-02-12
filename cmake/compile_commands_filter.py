import sys
import json

filename = sys.argv[1]

# Load the JSON data from the file
with open(filename, 'r') as json_file:
    data = json.load(json_file)

# Filter out entries with "build/_deps" in the "directory" key
filtered_data = [entry for entry in data if "build/_deps" not in entry["directory"]]

# Write the filtered data back to the file
with open(filename, 'w') as json_file:
    json.dump(filtered_data, json_file, indent=2)

