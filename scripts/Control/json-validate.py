import json
from jsonschema import validate

with open("demo.json") as f:
  data = json.load(f)
f.close()

with open("config-schema.json") as f:
  schema = json.load(f)
f.close()

try:
  validate(instance=data, schema=schema)
except AssertionError as error:
  print("Exception", error)
